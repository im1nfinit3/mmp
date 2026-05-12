#include "mmp_library.h"
#include "mmp_playback.h"
#include "sqlite3.h"

#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include <stdio.h>

struct _MmpLibrary {
    GObject parent_instance;

    sqlite3       *db;
    sqlite3       *library_db;

    GstDiscoverer *discoverer;

    GList         *songs;
    GHashTable    *songs_by_path;

    GQueue        *queue;
    GList         *current_track_node;
    guint          current_track_position;
    char          *current_file_path;

    bool           shuffle_mode;
    RepeatMode     repeat_mode;
    GPtrArray     *unplayed_pool;

    MmpPlayback   *playback;
};

G_DEFINE_TYPE(MmpLibrary, mmp_library, G_TYPE_OBJECT)

enum {
    SIGNAL_QUEUE_CHANGED,
    SIGNAL_NOW_PLAYING_CHANGED,
    SIGNAL_SONG_ADDED,
    SIGNAL_SONG_UPDATED,
    SIGNAL_PLAYLISTS_CHANGED,
    N_SIGNALS
};
static guint lib_signals[N_SIGNALS] = {0};

static GList *get_next_track_node(MmpLibrary *lib);
bool db_save_song(sqlite3 *db, const Song *song);

/* ---- database prototypes (from database.c) ---- */
extern bool   db_init(const char *db_path, sqlite3 **db_out);
extern void   db_close(sqlite3 *db);
extern GList *db_get_all_songs(sqlite3 *db);
extern GList *db_get_playlists(sqlite3 *db);
extern GList *db_get_playlist_songs(sqlite3 *db, int playlist_id);
extern bool   db_create_playlist(sqlite3 *db, const char *name, int *playlist_id_out);
extern bool   db_delete_playlist(sqlite3 *db, int playlist_id);
extern bool   db_rename_playlist(sqlite3 *db, int playlist_id, const char *new_name);
extern bool   db_add_song_to_playlist(sqlite3 *db, int playlist_id, const Song *song);
extern bool   db_remove_song_from_playlist(sqlite3 *db, int playlist_id, const char *song_path);

/* ---- forward decls for signal handlers ---- */
static void on_pb_eos          (MmpPlayback *pb, gpointer user_data);
static void on_pb_tag_received (MmpPlayback *pb, const char *artist, const char *title, gpointer user_data);
static void on_pb_error        (MmpPlayback *pb, const char *message, gpointer user_data);
static void on_pb_state_changed(MmpPlayback *pb, int state, gpointer user_data);

/* ---- directory scan ---- */
static void scan_directory_recursive(MmpLibrary *lib, const char *path, GHashTable *existing_paths);
static void scan_directory_thread(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable);

/* ======================================================================
 * Internal helpers
 * ====================================================================== */

static void rebuild_unplayed_pool(MmpLibrary *lib)
{
    g_clear_pointer(&lib->unplayed_pool, g_ptr_array_unref);

    if (!lib->shuffle_mode) return;

    lib->unplayed_pool = g_ptr_array_new();
    for (GList *l = lib->queue->head; l != NULL; l = l->next) {
        if (l != lib->current_track_node)
            g_ptr_array_add(lib->unplayed_pool, l);
    }
}

static void set_current_track_node(MmpLibrary *lib, GList *node)
{
    lib->current_track_node = node;
    if (node) {
        gint idx = g_queue_link_index(lib->queue, node);
        lib->current_track_position = (idx >= 0) ? (guint)idx : G_MAXUINT;
    } else {
        lib->current_track_position = G_MAXUINT;
    }
}

static GList *find_queue_node(MmpLibrary *lib, const char *path)
{
    for (GList *l = lib->queue->head; l != NULL; l = l->next) {
        if (g_strcmp0((const char *)l->data, path) == 0)
            return l;
    }
    return NULL;
}

static GList *get_next_track_node(MmpLibrary *lib)
{
    if (lib->repeat_mode == REPEAT_ONE && lib->current_track_node)
        return lib->current_track_node;

    if (lib->shuffle_mode) {
        if (lib->unplayed_pool == NULL) {
            if (lib->repeat_mode == REPEAT_ALL || lib->current_track_node == NULL)
                rebuild_unplayed_pool(lib);
            else
                return NULL;
        }

        if (lib->unplayed_pool == NULL || lib->unplayed_pool->len == 0)
            return NULL;

        guint index = g_random_int_range(0, (gint32)lib->unplayed_pool->len);
        GList *node = g_ptr_array_index(lib->unplayed_pool, index);
        g_ptr_array_remove_index_fast(lib->unplayed_pool, index);
        return node;
    }

    if (lib->current_track_node && lib->current_track_node->next)
        return lib->current_track_node->next;
    else if (lib->repeat_mode == REPEAT_ALL)
        return lib->queue->head;

    return NULL;
}

static void play_track_node(MmpLibrary *lib, GList *node)
{
    if (!node) return;

    set_current_track_node(lib, node);
    g_free(lib->current_file_path);
    lib->current_file_path = g_strdup((const char *)node->data);

    char *uri = g_filename_to_uri(lib->current_file_path, NULL, NULL);
    if (uri) {
        mmp_playback_play_uri(lib->playback, uri);
        g_free(uri);
    }
}

static void add_to_queue_internal(MmpLibrary *lib, const char *path, bool play_now, bool emit_signal)
{
    if (!path) return;

    g_queue_push_tail(lib->queue, g_strdup(path));
    GList *new_node = g_queue_peek_tail_link(lib->queue);

    if (lib->shuffle_mode && lib->unplayed_pool)
        g_ptr_array_add(lib->unplayed_pool, new_node);

    if (play_now)
        play_track_node(lib, new_node);
    else if (emit_signal)
        g_signal_emit(lib, lib_signals[SIGNAL_QUEUE_CHANGED], 0);
}

/* ======================================================================
 * Playback signal handlers
 * ====================================================================== */

static void on_pb_eos(MmpPlayback *pb, gpointer user_data)
{
    (void)pb;
    MmpLibrary *lib = MMP_LIBRARY(user_data);

    GList *next = get_next_track_node(lib);
    if (next)
        play_track_node(lib, next);
    else
        mmp_playback_stop(lib->playback);
}

static void on_pb_tag_received(MmpPlayback *pb, const char *artist, const char *title, gpointer user_data)
{
    (void)pb;
    MmpLibrary *lib = MMP_LIBRARY(user_data);

    Song *s = g_hash_table_lookup(lib->songs_by_path, lib->current_file_path);
    if (!s) return;

    /* Only fill in from stream tags if metadata is missing.
     * GstDiscoverer already parsed file tags during the scan;
     * stream tags are often incomplete and shouldn't overwrite. */
    bool updated = false;
    if ((!s->artist || !s->artist[0]) && artist && artist[0]) {
        g_free(s->artist);
        s->artist = g_strdup(artist);
        updated = true;
    }
    if ((!s->title || !s->title[0]) && title && title[0]) {
        g_free(s->title);
        s->title = g_strdup(title);
        updated = true;
    }

    if (updated) {
        db_save_song(lib->library_db, s);
        g_signal_emit(lib, lib_signals[SIGNAL_NOW_PLAYING_CHANGED], 0, s);
    }
}

static void on_pb_error(MmpPlayback *pb, const char *message, gpointer user_data)
{
    (void)pb;
    g_printerr("GStreamer error: %s\n", message);
    MmpLibrary *lib = MMP_LIBRARY(user_data);
    g_signal_emit(lib, lib_signals[SIGNAL_NOW_PLAYING_CHANGED], 0, NULL);
}

static void on_pb_state_changed(MmpPlayback *pb, int state, gpointer user_data)
{
    (void)pb;
    MmpLibrary *lib = MMP_LIBRARY(user_data);

    if (state == MMP_PLAYBACK_PLAYING && lib->current_file_path) {
        Song *s = g_hash_table_lookup(lib->songs_by_path, lib->current_file_path);
        g_signal_emit(lib, lib_signals[SIGNAL_NOW_PLAYING_CHANGED], 0, s);
    } else if (state == MMP_PLAYBACK_STOPPED) {
        g_signal_emit(lib, lib_signals[SIGNAL_NOW_PLAYING_CHANGED], 0, NULL);
    }
}

/* ======================================================================
 * GObject lifecycle
 * ====================================================================== */

static void mmp_library_init(MmpLibrary *lib)
{
    lib->queue = g_queue_new();
    lib->songs_by_path = g_hash_table_new(g_str_hash, g_str_equal);
    lib->shuffle_mode = false;
    lib->repeat_mode = REPEAT_OFF;
    lib->unplayed_pool = NULL;
    lib->current_track_position = G_MAXUINT;

    GError *err = NULL;
    lib->discoverer = gst_discoverer_new(2 * GST_SECOND, &err);
    if (err) {
        g_warning("Could not create GstDiscoverer: %s", err->message);
        g_clear_error(&err);
    }

    char *config_dir = g_build_filename(g_get_user_config_dir(), "mmp", NULL);
    g_mkdir_with_parents(config_dir, 0755);

    char *db_path = g_build_filename(config_dir, "playlists.db", NULL);
    db_init(db_path, &lib->db);
    g_free(db_path);

    char *library_db_path = g_build_filename(config_dir, "library.db", NULL);
    db_init(library_db_path, &lib->library_db);
    g_free(library_db_path);

    g_free(config_dir);
}

static void mmp_library_finalize(GObject *obj)
{
    MmpLibrary *lib = MMP_LIBRARY(obj);

    if (lib->discoverer)
        gst_object_unref(lib->discoverer);

    db_close(lib->db);
    db_close(lib->library_db);

    g_hash_table_destroy(lib->songs_by_path);
    g_list_free_full(lib->songs, (GDestroyNotify)free_song);

    g_queue_foreach(lib->queue, (GFunc)g_free, NULL);
    g_queue_free(lib->queue);

    g_clear_pointer(&lib->unplayed_pool, g_ptr_array_unref);
    g_free(lib->current_file_path);

    G_OBJECT_CLASS(mmp_library_parent_class)->finalize(obj);
}

static void mmp_library_class_init(MmpLibraryClass *klass)
{
    GObjectClass *gobj = G_OBJECT_CLASS(klass);
    gobj->finalize = mmp_library_finalize;

    lib_signals[SIGNAL_QUEUE_CHANGED] = g_signal_new(
        "queue-changed", MMP_TYPE_LIBRARY, G_SIGNAL_RUN_LAST,
        0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    lib_signals[SIGNAL_NOW_PLAYING_CHANGED] = g_signal_new(
        "now-playing-changed", MMP_TYPE_LIBRARY, G_SIGNAL_RUN_LAST,
        0, NULL, NULL, g_cclosure_marshal_VOID__BOXED, G_TYPE_NONE, 1,
        MMP_TYPE_SONG);

    lib_signals[SIGNAL_SONG_ADDED] = g_signal_new(
        "song-added", MMP_TYPE_LIBRARY, G_SIGNAL_RUN_LAST,
        0, NULL, NULL, g_cclosure_marshal_VOID__BOXED, G_TYPE_NONE, 1,
        MMP_TYPE_SONG);

    lib_signals[SIGNAL_SONG_UPDATED] = g_signal_new(
        "song-updated", MMP_TYPE_LIBRARY, G_SIGNAL_RUN_LAST,
        0, NULL, NULL, g_cclosure_marshal_VOID__BOXED, G_TYPE_NONE, 1,
        MMP_TYPE_SONG);

    lib_signals[SIGNAL_PLAYLISTS_CHANGED] = g_signal_new(
        "playlists-changed", MMP_TYPE_LIBRARY, G_SIGNAL_RUN_LAST,
        0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

/* ======================================================================
 * Public API — lifecycle
 * ====================================================================== */

MmpLibrary *mmp_library_new(MmpPlayback *pb)
{
    MmpLibrary *lib = g_object_new(MMP_TYPE_LIBRARY, NULL);
    mmp_library_attach_playback(lib, pb);
    return lib;
}

void mmp_library_attach_playback(MmpLibrary *lib, MmpPlayback *pb)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));
    lib->playback = pb;

    g_signal_connect(pb, "eos",           G_CALLBACK(on_pb_eos),           lib);
    g_signal_connect(pb, "tag-received",  G_CALLBACK(on_pb_tag_received),  lib);
    g_signal_connect(pb, "error",         G_CALLBACK(on_pb_error),         lib);
    g_signal_connect(pb, "state-changed", G_CALLBACK(on_pb_state_changed), lib);
}

/* ======================================================================
 * Public API — library
 * ====================================================================== */

void mmp_library_load_cached(MmpLibrary *lib)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));

    GList *cached = db_get_all_songs(lib->library_db);
    for (GList *l = cached; l != NULL; l = l->next) {
        Song *s = l->data;
        g_hash_table_insert(lib->songs_by_path, s->path, s);
    }
    lib->songs = g_list_concat(lib->songs, cached);

    for (GList *l = lib->songs; l != NULL; l = l->next)
        g_signal_emit(lib, lib_signals[SIGNAL_SONG_ADDED], 0, (const Song *)l->data);
}

Song *mmp_library_find_song(MmpLibrary *lib, const char *path)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), NULL);
    return g_hash_table_lookup(lib->songs_by_path, path);
}

GList *mmp_library_get_all_songs(MmpLibrary *lib)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), NULL);
    return lib->songs;
}

static void mmp_library_extract_metadata(MmpLibrary *lib, Song *song)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));
    if (!lib->discoverer || !song) return;

    char *uri = g_filename_to_uri(song->path, NULL, NULL);
    if (!uri) return;

    GError *err = NULL;
    GstDiscovererInfo *info = gst_discoverer_discover_uri(lib->discoverer, uri, &err);

    if (info) {
        GstClockTime duration = gst_discoverer_info_get_duration(info);
        if (GST_CLOCK_TIME_IS_VALID(duration)) {
            int seconds = (int)(duration / GST_SECOND);
            g_free(song->duration_str);
            song->duration_str = g_strdup_printf("%d:%02d", seconds / 60, seconds % 60);
        }

        const GstTagList *tags = gst_discoverer_info_get_tags(info);
        if (tags) {
            char *title = NULL, *artist = NULL, *album = NULL;
            if (gst_tag_list_get_string(tags, GST_TAG_TITLE, &title) && title && title[0])
                { g_free(song->title); song->title = title; }
            else g_free(title);
            if (gst_tag_list_get_string(tags, GST_TAG_ARTIST, &artist) && artist && artist[0])
                { g_free(song->artist); song->artist = artist; }
            else g_free(artist);
            if (gst_tag_list_get_string(tags, GST_TAG_ALBUM, &album) && album && album[0])
                { g_free(song->album); song->album = album; }
            else g_free(album);
        }
        gst_discoverer_info_unref(info);
    } else {
        g_clear_error(&err);
    }
    g_free(uri);
}

/* ======================================================================
 * Public API — queue
 * ====================================================================== */

void mmp_library_add_to_queue(MmpLibrary *lib, const char *path, bool play_now)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));
    add_to_queue_internal(lib, path, play_now, true);
}

void mmp_library_add_songs_to_queue(MmpLibrary *lib, GList *songs)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));
    if (!songs) return;

    for (GList *l = songs; l != NULL; l = l->next) {
        Song *song = l->data;
        add_to_queue_internal(lib, song->path, false, false);
    }
    g_signal_emit(lib, lib_signals[SIGNAL_QUEUE_CHANGED], 0);
}

void mmp_library_open_files(MmpLibrary *lib, GFile **files, int n)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));

    mmp_library_clear_queue(lib);

    for (int i = 0; i < n; i++) {
        char *path = g_file_get_path(files[i]);
        mmp_library_add_to_queue(lib, path, i == 0);
        g_free(path);
    }
}

void mmp_library_play_from_library(MmpLibrary *lib, const char *path)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));

    GList *node = find_queue_node(lib, path);
    if (node) {
        play_track_node(lib, node);
    } else {
        g_queue_push_head(lib->queue, g_strdup(path));
        play_track_node(lib, lib->queue->head);
        g_signal_emit(lib, lib_signals[SIGNAL_QUEUE_CHANGED], 0);
    }
    rebuild_unplayed_pool(lib);
}

static void remove_queue_node_internal(MmpLibrary *lib, GList *node)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));
    if (!node) return;

    if (lib->unplayed_pool) {
        for (guint i = 0; i < lib->unplayed_pool->len; i++) {
            if (g_ptr_array_index(lib->unplayed_pool, i) == node) {
                g_ptr_array_remove_index_fast(lib->unplayed_pool, i);
                break;
            }
        }
    }

    bool is_current = (node == lib->current_track_node);
    GList *next_node = is_current ? node->next : NULL;

    if (is_current && next_node)
        play_track_node(lib, next_node);
    else if (is_current) {
        mmp_playback_stop(lib->playback);
        set_current_track_node(lib, NULL);
    }

    g_free(node->data);
    g_queue_delete_link(lib->queue, node);

    if (is_current && next_node)
        rebuild_unplayed_pool(lib);

    g_signal_emit(lib, lib_signals[SIGNAL_QUEUE_CHANGED], 0);
}

void mmp_library_clear_queue(MmpLibrary *lib)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));

    g_clear_pointer(&lib->unplayed_pool, g_ptr_array_unref);
    mmp_playback_stop(lib->playback);

    set_current_track_node(lib, NULL);
    g_free(lib->current_file_path);
    lib->current_file_path = NULL;

    g_queue_foreach(lib->queue, (GFunc)g_free, NULL);
    g_queue_clear(lib->queue);

    g_signal_emit(lib, lib_signals[SIGNAL_QUEUE_CHANGED], 0);
}

void mmp_library_play_next(MmpLibrary *lib, const char *path)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));
    if (!path) return;

    char *path_copy = g_strdup(path);
    GList *new_node;
    if (lib->current_track_node) {
        g_queue_insert_after(lib->queue, lib->current_track_node, path_copy);
        new_node = lib->current_track_node->next;
    } else {
        g_queue_push_head(lib->queue, path_copy);
        new_node = lib->queue->head;
    }

    if (lib->shuffle_mode && lib->unplayed_pool)
        g_ptr_array_add(lib->unplayed_pool, new_node);

    g_signal_emit(lib, lib_signals[SIGNAL_QUEUE_CHANGED], 0);
}

void mmp_library_skip_next(MmpLibrary *lib)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));

    GList *next = get_next_track_node(lib);
    if (next)
        play_track_node(lib, next);
    else
        mmp_playback_stop(lib->playback);
}

void mmp_library_skip_prev(MmpLibrary *lib)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));

    double pos = mmp_playback_get_position(lib->playback);
    if (pos > 3.0) {
        mmp_playback_seek(lib->playback, 0);
        return;
    }

    if (lib->current_track_node && lib->current_track_node->prev)
        play_track_node(lib, lib->current_track_node->prev);
}

guint mmp_library_get_queue_length(MmpLibrary *lib)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), 0);
    return lib->queue ? lib->queue->length : 0;
}

const char *mmp_library_get_queue_path_at(MmpLibrary *lib, guint index)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), NULL);
    GList *node = g_queue_peek_nth_link(lib->queue, index);
    return node ? (const char *)node->data : NULL;
}

bool mmp_library_is_playing_queue_position(MmpLibrary *lib, guint index)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), false);
    return lib->current_track_position == index;
}

void mmp_library_reorder_queue(MmpLibrary *lib, guint from_index, guint to_index)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));
    if (from_index == to_index) return;

    GList *node = g_queue_peek_nth_link(lib->queue, from_index);
    if (!node) return;

    char *path = node->data;
    bool was_current = (node == lib->current_track_node);

    g_queue_delete_link(lib->queue, node);

    guint adjusted_to = (to_index > from_index) ? to_index - 1 : to_index;
    GList *target = g_queue_peek_nth_link(lib->queue, adjusted_to);

    if (target)
        g_queue_insert_before(lib->queue, target, path);
    else
        g_queue_push_tail(lib->queue, path);

    if (was_current)
        set_current_track_node(lib, target ? target->prev : g_queue_peek_tail_link(lib->queue));

    rebuild_unplayed_pool(lib);
    g_signal_emit(lib, lib_signals[SIGNAL_QUEUE_CHANGED], 0);
}

GList *mmp_library_get_queue_path_list(MmpLibrary *lib)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), NULL);
    return g_list_copy(lib->queue->head);
}

void mmp_library_remove_from_queue(MmpLibrary *lib, guint index)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));
    GList *node = g_queue_peek_nth_link(lib->queue, index);
    if (node)
        remove_queue_node_internal(lib, node);
}

const char *mmp_library_get_current_path(MmpLibrary *lib)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), NULL);
    return lib->current_file_path;
}

/* ======================================================================
 * Public API — shuffle / repeat
 * ====================================================================== */

void mmp_library_toggle_shuffle(MmpLibrary *lib)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));

    lib->shuffle_mode = !lib->shuffle_mode;
    if (lib->shuffle_mode)
        rebuild_unplayed_pool(lib);
    else
        g_clear_pointer(&lib->unplayed_pool, g_ptr_array_unref);
}

void mmp_library_toggle_repeat(MmpLibrary *lib)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));
    lib->repeat_mode = (lib->repeat_mode + 1) % 3;
}

bool mmp_library_get_shuffle(MmpLibrary *lib)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), false);
    return lib->shuffle_mode;
}

RepeatMode mmp_library_get_repeat(MmpLibrary *lib)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), REPEAT_OFF);
    return lib->repeat_mode;
}

/* ======================================================================
 * Public API — playlists
 * ====================================================================== */

GList *mmp_library_get_playlists(MmpLibrary *lib)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), NULL);
    return db_get_playlists(lib->db);
}

GList *mmp_library_get_playlist_songs(MmpLibrary *lib, int playlist_id)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), NULL);
    return db_get_playlist_songs(lib->db, playlist_id);
}

bool mmp_library_create_playlist(MmpLibrary *lib, const char *name, int *id_out)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), false);
    bool ok = db_create_playlist(lib->db, name, id_out);
    if (ok)
        g_signal_emit(lib, lib_signals[SIGNAL_PLAYLISTS_CHANGED], 0);
    return ok;
}

bool mmp_library_delete_playlist(MmpLibrary *lib, int playlist_id)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), false);
    bool ok = db_delete_playlist(lib->db, playlist_id);
    if (ok)
        g_signal_emit(lib, lib_signals[SIGNAL_PLAYLISTS_CHANGED], 0);
    return ok;
}

bool mmp_library_rename_playlist(MmpLibrary *lib, int playlist_id, const char *new_name)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), false);
    bool ok = db_rename_playlist(lib->db, playlist_id, new_name);
    if (ok)
        g_signal_emit(lib, lib_signals[SIGNAL_PLAYLISTS_CHANGED], 0);
    return ok;
}

bool mmp_library_add_song_to_playlist(MmpLibrary *lib, int playlist_id, Song *song)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), false);
    return db_add_song_to_playlist(lib->db, playlist_id, song);
}

bool mmp_library_remove_song_from_playlist(MmpLibrary *lib, int playlist_id, const char *path)
{
    g_return_val_if_fail(MMP_IS_LIBRARY(lib), false);
    return db_remove_song_from_playlist(lib->db, playlist_id, path);
}

void mmp_library_load_playlist(MmpLibrary *lib, int playlist_id, const char *start_song_path)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));

    GList *songs = db_get_playlist_songs(lib->db, playlist_id);
    if (!songs) return;

    mmp_library_clear_queue(lib);

    for (GList *l = songs; l != NULL; l = l->next) {
        Song *s = l->data;
        add_to_queue_internal(lib, s->path, false, false);
    }

    GList *start_node = NULL;
    if (start_song_path) {
        for (GList *l = lib->queue->head; l != NULL; l = l->next) {
            if (g_strcmp0((const char *)l->data, start_song_path) == 0) {
                start_node = l;
                break;
            }
        }
    }

    if (!start_node) {
        if (lib->shuffle_mode) {
            int len = g_queue_get_length(lib->queue);
            int idx = g_random_int_range(0, len);
            start_node = g_queue_peek_nth_link(lib->queue, idx);
        } else {
            start_node = lib->queue->head;
        }
    }

    if (start_node)
        play_track_node(lib, start_node);

    rebuild_unplayed_pool(lib);
    g_signal_emit(lib, lib_signals[SIGNAL_QUEUE_CHANGED], 0);

    g_list_free_full(songs, (GDestroyNotify)free_song);
}

/* ======================================================================
 * Public API — directory scan
 * ====================================================================== */

static void scan_directory_recursive(MmpLibrary *lib, const char *path, GHashTable *existing_paths)
{
    GFile *dir = g_file_new_for_path(path);
    GFileEnumerator *enumerator = g_file_enumerate_children(dir,
        "standard::*", G_FILE_QUERY_INFO_NONE, NULL, NULL);

    if (enumerator) {
        GFileInfo *info;
        while ((info = g_file_enumerator_next_file(enumerator, NULL, NULL)) != NULL) {
            const char *name = g_file_info_get_name(info);
            GFile *child = g_file_get_child(dir, name);
            char *child_path = g_file_get_path(child);

            if (g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY) {
                scan_directory_recursive(lib, child_path, existing_paths);
            } else if (g_str_has_suffix(name, ".mp3") ||
                       g_str_has_suffix(name, ".flac") ||
                       g_str_has_suffix(name, ".ogg") ||
                       g_str_has_suffix(name, ".wav") ||
                       g_str_has_suffix(name, ".m4a")) {

                Song *existing_song = g_hash_table_lookup(existing_paths, child_path);
                if (!existing_song) {
                    Song *song = g_new0(Song, 1);
                    song->path = g_strdup(child_path);

                    char *title = g_strdup(name);
                    char *dot = g_strrstr(title, ".");
                    if (dot) *dot = '\0';
                    song->title = title;

                    GFile *parent = g_file_get_parent(child);
                    GFile *grand_parent = parent ? g_file_get_parent(parent) : NULL;

                    if (parent) {
                        char *parent_name = g_file_get_basename(parent);
                        song->album = g_strdup(parent_name);
                        g_free(parent_name);

                        if (grand_parent) {
                            char *grand_parent_name = g_file_get_basename(grand_parent);
                            song->artist = g_strdup(grand_parent_name);
                            g_free(grand_parent_name);
                        }
                    }

                    if (!song->album) song->album = g_strdup("Unknown Album");
                    if (!song->artist) song->artist = g_strdup("Unknown Artist");

                    mmp_library_extract_metadata(lib, song);
                    db_save_song(lib->library_db, song);

                    g_hash_table_insert(existing_paths, song->path, song);

                    if (grand_parent) g_object_unref(grand_parent);
                    if (parent) g_object_unref(parent);
                } else if (!existing_song->duration_str) {
                    mmp_library_extract_metadata(lib, existing_song);
                    if (existing_song->duration_str)
                        db_save_song(lib->library_db, existing_song);
                }
            }

            g_free(child_path);
            g_object_unref(child);
            g_object_unref(info);
        }
        g_object_unref(enumerator);
    }
    g_object_unref(dir);
}

typedef struct {
    MmpLibrary *lib;
    GHashTable *songs_to_add;
} ScanResult;

typedef struct {
    MmpLibrary *lib;
    char       *music_dir;
    GHashTable *existing_paths;
} ScanTaskData;

static void scan_task_data_free(gpointer data)
{
    ScanTaskData *std = data;
    if (std->existing_paths)
        g_hash_table_destroy(std->existing_paths);
    g_free(data);
}

static void scan_directory_thread(GTask *task, gpointer source_object,
                                   gpointer task_data, GCancellable *cancellable)
{
    (void)task; (void)source_object; (void)cancellable;
    ScanTaskData *std = (ScanTaskData *)task_data;
    MmpLibrary *lib = std->lib;

    ScanResult *result = g_new0(ScanResult, 1);
    result->lib = lib;
    result->songs_to_add = g_hash_table_new(g_str_hash, g_str_equal);

    GHashTable *existing_paths = std->existing_paths;

    const char *music_dir = std->music_dir;
    if (music_dir)
        scan_directory_recursive(lib, music_dir, existing_paths);
    g_free(std->music_dir);

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, existing_paths);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        Song *s = value;
        if (!g_hash_table_contains(result->songs_to_add, s->path))
            g_hash_table_insert(result->songs_to_add, s->path, s);
    }

    g_hash_table_destroy(existing_paths);
    std->existing_paths = NULL;

    g_task_return_pointer(task, result, NULL);
}

static void on_scan_complete(GObject *source, GAsyncResult *res, gpointer user_data)
{
    (void)source; (void)user_data;

    GTask *task = G_TASK(res);
    ScanResult *result = g_task_propagate_pointer(task, NULL);
    if (!result) return;

    MmpLibrary *lib = result->lib;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, result->songs_to_add);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        Song *song = value;
        Song *existing = g_hash_table_lookup(lib->songs_by_path, song->path);
        if (!existing) {
            g_hash_table_insert(lib->songs_by_path, song->path, song);
            lib->songs = g_list_append(lib->songs, song);
            g_signal_emit(lib, lib_signals[SIGNAL_SONG_ADDED], 0, song);
        } else if (song != existing) {
            free_song(song);
        }
    }

    g_hash_table_unref(result->songs_to_add);
    g_free(result);
}

void mmp_library_scan_async(MmpLibrary *lib, const char *music_dir)
{
    g_return_if_fail(MMP_IS_LIBRARY(lib));

    ScanTaskData *std = g_new(ScanTaskData, 1);
    std->lib = lib;
    std->music_dir = g_strdup(music_dir);

    std->existing_paths = g_hash_table_new(g_str_hash, g_str_equal);
    for (GList *l = lib->songs; l != NULL; l = l->next) {
        Song *s = l->data;
        g_hash_table_insert(std->existing_paths, s->path, s);
    }

    GTask *task = g_task_new(NULL, NULL, on_scan_complete, NULL);
    g_task_set_task_data(task, std, scan_task_data_free);
    g_task_run_in_thread(task, scan_directory_thread);
    g_object_unref(task);
}
