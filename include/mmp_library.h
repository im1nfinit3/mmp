#ifndef MMP_LIBRARY_H
#define MMP_LIBRARY_H

#include <gio/gio.h>
#include <glib-object.h>
#include "mmp_types.h"

typedef struct _MmpPlayback MmpPlayback;

Song  *mmp_song_copy(const Song *song);
void   free_song(Song *song);
#define MMP_TYPE_SONG (mmp_song_get_type())
GType  mmp_song_get_type(void);

typedef struct {
    int   id;
    char *name;
} Playlist;
void   free_playlist(Playlist *p);

#define MMP_TYPE_LIBRARY (mmp_library_get_type())
G_DECLARE_FINAL_TYPE(MmpLibrary, mmp_library, MMP, LIBRARY, GObject)

MmpLibrary   *mmp_library_new(MmpPlayback *pb);
void          mmp_library_attach_playback(MmpLibrary *lib, MmpPlayback *pb);

void          mmp_library_load_cached(MmpLibrary *lib);
void          mmp_library_scan_async(MmpLibrary *lib, const char *music_dir);
Song         *mmp_library_find_song(MmpLibrary *lib, const char *path);
GList        *mmp_library_get_all_songs(MmpLibrary *lib);

void          mmp_library_add_to_queue      (MmpLibrary *lib, const char *path, bool play_now);
void          mmp_library_add_songs_to_queue(MmpLibrary *lib, GList *songs);
void          mmp_library_open_files        (MmpLibrary *lib, GFile **files, int n);
void          mmp_library_play_from_library (MmpLibrary *lib, const char *path);
void          mmp_library_remove_from_queue (MmpLibrary *lib, guint index);
void          mmp_library_clear_queue       (MmpLibrary *lib);
void          mmp_library_play_next         (MmpLibrary *lib, const char *path);
void          mmp_library_skip_next         (MmpLibrary *lib);
void          mmp_library_skip_prev         (MmpLibrary *lib);
guint         mmp_library_get_queue_length          (MmpLibrary *lib);
const char   *mmp_library_get_queue_path_at         (MmpLibrary *lib, guint index);
bool          mmp_library_is_playing_queue_position (MmpLibrary *lib, guint index);
void          mmp_library_reorder_queue             (MmpLibrary *lib, guint from_index, guint to_index);
GList        *mmp_library_get_queue_path_list       (MmpLibrary *lib);
const char   *mmp_library_get_current_path          (MmpLibrary *lib);

void          mmp_library_toggle_shuffle(MmpLibrary *lib);
void          mmp_library_toggle_repeat (MmpLibrary *lib);
bool          mmp_library_get_shuffle   (MmpLibrary *lib);
RepeatMode    mmp_library_get_repeat    (MmpLibrary *lib);

GList        *mmp_library_get_playlists            (MmpLibrary *lib);
GList        *mmp_library_get_playlist_songs        (MmpLibrary *lib, int playlist_id);
bool          mmp_library_create_playlist           (MmpLibrary *lib, const char *name, int *id_out);
bool          mmp_library_delete_playlist           (MmpLibrary *lib, int playlist_id);
bool          mmp_library_rename_playlist           (MmpLibrary *lib, int playlist_id, const char *name);
bool          mmp_library_add_song_to_playlist      (MmpLibrary *lib, int playlist_id, Song *song);
bool          mmp_library_remove_song_from_playlist (MmpLibrary *lib, int playlist_id, const char *path);
void          mmp_library_load_playlist             (MmpLibrary *lib, int playlist_id, const char *start_path);

#endif
