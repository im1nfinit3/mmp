#include "mmp_ui_internal.h"
#include <stdbool.h>

G_DEFINE_TYPE(MmpSongItem, mmp_song_item, G_TYPE_OBJECT)

static void mmp_song_item_init(MmpSongItem* self) { (void)self; }
static void mmp_song_item_class_init(MmpSongItemClass* klass) { (void)klass; }

static MmpSongItem* mmp_song_item_new(Song* song) {
    MmpSongItem* item = g_object_new(MMP_TYPE_SONG_ITEM, NULL);
    item->song = song;
    return item;
}

static void add_to_library_ui(MmpUI* ui, Song* song) {
    if (!g_hash_table_contains(ui->artists_set, song->artist)) {
        g_hash_table_insert(ui->artists_set, g_strdup(song->artist), NULL);
        GtkWidget* row = gtk_list_box_row_new();
        GtkWidget* label = gtk_label_new(song->artist);
        gtk_label_set_xalign(GTK_LABEL(label), 0);
        gtk_widget_add_css_class(label, "row-label");
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), label);
        gtk_list_box_append(ui->artists_list, row);
    }

    if (!g_hash_table_contains(ui->albums_set, song->album)) {
        g_hash_table_insert(ui->albums_set, g_strdup(song->album), NULL);
        GtkWidget* row = gtk_list_box_row_new();
        GtkWidget* label = gtk_label_new(song->album);
        gtk_label_set_xalign(GTK_LABEL(label), 0);
        gtk_widget_add_css_class(label, "row-label");
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), label);
        g_object_set_data_full(G_OBJECT(row), "album-artist", g_strdup(song->artist), g_free);
        gtk_list_box_append(ui->albums_list, row);
    }
}

void ui_update_queue(MmpUI* ui) {
    if (!ui->queue_store) return;

    g_list_free_full(ui->queue_fallback_songs, (GDestroyNotify)free_song);
    ui->queue_fallback_songs = NULL;
    g_list_store_remove_all(ui->queue_store);

    GList* paths = mmp_library_get_queue_path_list(ui->library);
    if (!paths) return;

    for (GList* l = paths; l; l = l->next) {
        const char* path = l->data;

        Song* found_song = mmp_library_find_song(ui->library, path);

        MmpSongItem* item;
        if (found_song) {
            item = mmp_song_item_new(found_song);
        } else {
            Song* s = g_new0(Song, 1);
            s->path = g_strdup(path);
            char* basename = g_path_get_basename(path);
            s->title = g_strdup(basename);
            char* dot = g_strrstr(s->title, ".");
            if (dot) *dot = '\0';
            s->artist = g_strdup("Unknown Artist");
            s->album = g_strdup("Unknown Album");
            g_free(basename);
            item = mmp_song_item_new(s);
            ui->queue_fallback_songs = g_list_append(ui->queue_fallback_songs, s);
        }

        g_list_store_append(ui->queue_store, G_OBJECT(item));
        g_object_unref(item);
    }

    g_list_free(paths);
}

gboolean queue_drop_cb(GtkDropTarget* target, const GValue* value, double x, double y, gpointer user_data);
void queue_drag_begin_cb(GtkDragSource* source, GdkDrag* drag, gpointer user_data);
void song_row_secondary_click_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer user_data);
void queue_row_secondary_click_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer user_data);
gboolean filter_albums_cb(GtkListBoxRow* row, gpointer user_data);
void search_changed_cb(GtkSearchEntry* entry, gpointer user_data);
void artist_row_activated_cb(GtkListBox* list, GtkListBoxRow* row, gpointer user_data);
void album_row_activated_cb(GtkListBox* list, GtkListBoxRow* row, gpointer user_data);
void song_view_activate_cb(GtkListView* view, guint position, gpointer user_data);
void queue_view_activate_cb(GtkListView* view, guint position, gpointer user_data);
void volume_controls_enter_cb(GtkEventControllerMotion* controller, double x, double y, gpointer user_data);
void volume_controls_leave_cb(GtkEventControllerMotion* controller, gpointer user_data);
void mute_button_clicked_cb(GtkButton* button, gpointer user_data);
void play_pause_clicked_cb(GtkButton* button, gpointer user_data);
void volume_scale_changed_cb(GtkRange* range, gpointer user_data);
void track_progress_scale_value_changed_cb(GtkRange* range, gpointer user_data);
void shuffle_clicked_cb(GtkButton* button, gpointer user_data);
void repeat_clicked_cb(GtkButton* button, gpointer user_data);
void previous_track_clicked_cb(GtkButton* button, gpointer user_data);
void next_track_clicked_cb(GtkButton* button, gpointer user_data);
void playlist_row_right_clicked_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer user_data);
void playlist_row_double_clicked_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer user_data);
void playlists_header_right_clicked_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer user_data);
void navigation_row_selected_cb(GtkListBox* list_box, GtkListBoxRow* row, gpointer user_data);
void create_playlist_action_cb(GSimpleAction* action, GVariant* parameter, gpointer user_data);
gboolean on_drop_cb(GtkDropTarget* target, const GValue* value, double x, double y, gpointer user_data);

static void song_factory_setup(GtkSignalListItemFactory* factory, GtkListItem* item, gpointer user_data) {
    (void)factory; (void)user_data;

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(box, "song-row");

    GtkWidget* indicator = gtk_image_new();
    gtk_box_append(GTK_BOX(box), indicator);

    GtkWidget* title_label = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(title_label), 0);
    gtk_widget_set_hexpand(title_label, TRUE);
    gtk_label_set_ellipsize(GTK_LABEL(title_label), PANGO_ELLIPSIZE_END);
    gtk_box_append(GTK_BOX(box), title_label);

    GtkWidget* artist_label = gtk_label_new("");
    gtk_widget_add_css_class(artist_label, "dim-label");
    gtk_label_set_ellipsize(GTK_LABEL(artist_label), PANGO_ELLIPSIZE_START);
    gtk_label_set_max_width_chars(GTK_LABEL(artist_label), 15);
    gtk_box_append(GTK_BOX(box), artist_label);

    GtkWidget* album_label = gtk_label_new("");
    gtk_widget_add_css_class(album_label, "dim-label");
    gtk_label_set_ellipsize(GTK_LABEL(album_label), PANGO_ELLIPSIZE_START);
    gtk_label_set_max_width_chars(GTK_LABEL(album_label), 15);
    gtk_box_append(GTK_BOX(box), album_label);

    GtkWidget* duration_label = gtk_label_new("");
    gtk_widget_add_css_class(duration_label, "dim-label");
    gtk_widget_add_css_class(duration_label, "duration-label");
    gtk_box_append(GTK_BOX(box), duration_label);

    gtk_list_item_set_child(item, box);

    GtkGesture* gesture = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture), GDK_BUTTON_SECONDARY);
    g_signal_connect(gesture, "released", G_CALLBACK(song_row_secondary_click_cb), NULL);
    gtk_widget_add_controller(box, GTK_EVENT_CONTROLLER(gesture));
}

static void song_factory_bind(GtkSignalListItemFactory* factory, GtkListItem* item, gpointer user_data) {
    (void)factory;
    MmpUI* ui = user_data;

    MmpSongItem* song_item = MMP_SONG_ITEM(gtk_list_item_get_item(item));
    if (!song_item || !song_item->song) return;

    Song* song = song_item->song;
    GtkWidget* box = gtk_list_item_get_child(item);
    if (!box) return;

    g_object_set_data(G_OBJECT(box), "song-data", song);

    GtkWidget* indicator = gtk_widget_get_first_child(box);
    const char* cur_path = mmp_library_get_current_path(ui->library);
    if (g_strcmp0(song->path, cur_path) == 0) {
        gtk_image_set_from_icon_name(GTK_IMAGE(indicator), "audio-volume-medium-symbolic");
    } else {
        gtk_image_clear(GTK_IMAGE(indicator));
    }

    GtkWidget* title_label = gtk_widget_get_next_sibling(indicator);
    gtk_label_set_label(GTK_LABEL(title_label), song->title);

    GtkWidget* artist_label = gtk_widget_get_next_sibling(title_label);
    gtk_label_set_label(GTK_LABEL(artist_label), song->artist);

    GtkWidget* album_label = gtk_widget_get_next_sibling(artist_label);
    gtk_label_set_label(GTK_LABEL(album_label), song->album);

    GtkWidget* duration_label = gtk_widget_get_next_sibling(album_label);
    if (song->duration_str) {
        gtk_label_set_label(GTK_LABEL(duration_label), song->duration_str);
        gtk_widget_set_visible(duration_label, TRUE);
    } else {
        gtk_widget_set_visible(duration_label, FALSE);
    }
}

static void queue_factory_setup(GtkSignalListItemFactory* factory, GtkListItem* item, gpointer user_data) {
    (void)factory; (void)user_data;

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(box, "song-row");

    GtkWidget* indicator = gtk_image_new();
    gtk_box_append(GTK_BOX(box), indicator);

    GtkWidget* title_label = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(title_label), 0);
    gtk_widget_set_hexpand(title_label, TRUE);
    gtk_label_set_ellipsize(GTK_LABEL(title_label), PANGO_ELLIPSIZE_END);
    gtk_box_append(GTK_BOX(box), title_label);

    GtkWidget* artist_label = gtk_label_new("");
    gtk_widget_add_css_class(artist_label, "dim-label");
    gtk_label_set_ellipsize(GTK_LABEL(artist_label), PANGO_ELLIPSIZE_START);
    gtk_label_set_max_width_chars(GTK_LABEL(artist_label), 15);
    gtk_box_append(GTK_BOX(box), artist_label);

    GtkWidget* album_label = gtk_label_new("");
    gtk_widget_add_css_class(album_label, "dim-label");
    gtk_label_set_ellipsize(GTK_LABEL(album_label), PANGO_ELLIPSIZE_START);
    gtk_label_set_max_width_chars(GTK_LABEL(album_label), 15);
    gtk_box_append(GTK_BOX(box), album_label);

    GtkWidget* duration_label = gtk_label_new("");
    gtk_widget_add_css_class(duration_label, "dim-label");
    gtk_widget_add_css_class(duration_label, "duration-label");
    gtk_box_append(GTK_BOX(box), duration_label);

    gtk_list_item_set_child(item, box);

    GtkGesture* gesture = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture), GDK_BUTTON_SECONDARY);
    g_signal_connect(gesture, "released", G_CALLBACK(queue_row_secondary_click_cb), NULL);
    gtk_widget_add_controller(box, GTK_EVENT_CONTROLLER(gesture));

    GtkDragSource* drag_source = gtk_drag_source_new();
    gtk_drag_source_set_actions(drag_source, GDK_ACTION_COPY | GDK_ACTION_MOVE);
    g_signal_connect(drag_source, "drag-begin", G_CALLBACK(queue_drag_begin_cb), NULL);

    GValue val = G_VALUE_INIT;
    g_value_init(&val, G_TYPE_STRING);
    g_value_set_static_string(&val, "reorder");
    GdkContentProvider* content = gdk_content_provider_new_for_value(&val);
    gtk_drag_source_set_content(drag_source, content);
    g_object_unref(content);
    g_value_unset(&val);
    gtk_widget_add_controller(box, GTK_EVENT_CONTROLLER(drag_source));

    GtkDropTarget* drop_target = gtk_drop_target_new(G_TYPE_STRING, GDK_ACTION_COPY | GDK_ACTION_MOVE);
    g_signal_connect(drop_target, "drop", G_CALLBACK(queue_drop_cb), NULL);
    gtk_widget_add_controller(box, GTK_EVENT_CONTROLLER(drop_target));
}

static void queue_factory_bind(GtkSignalListItemFactory* factory, GtkListItem* item, gpointer user_data) {
    (void)factory;
    MmpUI* ui = user_data;

    MmpSongItem* song_item = MMP_SONG_ITEM(gtk_list_item_get_item(item));
    if (!song_item || !song_item->song) return;

    Song* song = song_item->song;
    GtkWidget* box = gtk_list_item_get_child(item);
    if (!box) return;

    guint pos = gtk_list_item_get_position(item);

    g_object_set_data(G_OBJECT(box), "song-data", song);
    g_object_set_data(G_OBJECT(box), "queue-position", GUINT_TO_POINTER(pos + 1));

    GtkWidget* indicator = gtk_widget_get_first_child(box);
    if (mmp_library_is_playing_queue_position(ui->library, pos)) {
        gtk_image_set_from_icon_name(GTK_IMAGE(indicator), "audio-volume-medium-symbolic");
    } else {
        gtk_image_clear(GTK_IMAGE(indicator));
    }

    GtkWidget* title_label = gtk_widget_get_next_sibling(indicator);
    gtk_label_set_label(GTK_LABEL(title_label), song->title);

    GtkWidget* artist_label = gtk_widget_get_next_sibling(title_label);
    gtk_label_set_label(GTK_LABEL(artist_label), song->artist);

    GtkWidget* album_label = gtk_widget_get_next_sibling(artist_label);
    gtk_label_set_label(GTK_LABEL(album_label), song->album);

    GtkWidget* duration_label = gtk_widget_get_next_sibling(album_label);
    if (song->duration_str) {
        gtk_label_set_label(GTK_LABEL(duration_label), song->duration_str);
        gtk_widget_set_visible(duration_label, TRUE);
    } else {
        gtk_widget_set_visible(duration_label, FALSE);
    }
}

static void free_song_filter(gpointer data) {
    SongFilter* filter = data;
    if (filter->notify && filter->user_data) {
        filter->notify(filter->user_data);
    }
    g_free(filter);
}

void ui_clear_filters(MmpUI* ui) {
    g_list_free_full(ui->current_view_filters, free_song_filter);
    ui->current_view_filters = NULL;
}

void ui_set_view(MmpUI* ui, GList* base_list, bool owned, bool reverse) {
    if (ui->current_view_base_list && ui->current_view_base_list_owned) {
        g_list_free(ui->current_view_base_list);
    }
    ui->current_view_base_list = base_list;
    ui->current_view_base_list_owned = owned;
    ui->current_view_reverse = reverse;
}

void ui_add_filter(MmpUI* ui, SongFilterFunc func, gpointer data, GDestroyNotify notify) {
    SongFilter* filter = g_new0(SongFilter, 1);
    filter->filter = func;
    filter->user_data = data;
    filter->notify = notify;
    ui->current_view_filters = g_list_append(ui->current_view_filters, filter);
}

GList* ui_get_filtered_songs(MmpUI* ui) {
    if (!ui->current_view_base_list) return NULL;

    GList* songs = ui->current_view_base_list;
    bool reverse = ui->current_view_reverse;

    if (reverse) {
        songs = g_list_copy(songs);
        songs = g_list_reverse(songs);
    }

    GList* filtered = NULL;
    for (GList* l = songs; l != NULL; l = l->next) {
        Song* song = l->data;
        bool pass = true;

        for (GList* f = ui->current_view_filters; f != NULL; f = f->next) {
            SongFilter* filter = f->data;
            if (!filter->filter(song, filter->user_data)) {
                pass = false;
                break;
            }
        }

        if (pass) {
            filtered = g_list_prepend(filtered, song);
        }
    }

    if (reverse) {
        g_list_free(songs);
    }

    return g_list_reverse(filtered);
}

void ui_update_now_playing(MmpUI* ui, const char* old_path) {
    const char* new_path = mmp_library_get_current_path(ui->library);

    if (ui->song_store) {
        guint n = g_list_model_get_n_items(G_LIST_MODEL(ui->song_store));
        for (guint i = 0; i < n; i++) {
            GObject* obj = g_list_model_get_item(G_LIST_MODEL(ui->song_store), i);
            MmpSongItem* item = MMP_SONG_ITEM(obj);
            const char* path = item->song->path;
            if (g_strcmp0(path, old_path) == 0 || g_strcmp0(path, new_path) == 0) {
                g_list_store_remove(ui->song_store, i);
                g_list_store_insert(ui->song_store, i, obj);
            }
            g_object_unref(obj);
        }
    }

    if (ui->queue_store) {
        guint n = g_list_model_get_n_items(G_LIST_MODEL(ui->queue_store));
        for (guint i = 0; i < n; i++) {
            GObject* obj = g_list_model_get_item(G_LIST_MODEL(ui->queue_store), i);
            MmpSongItem* item = MMP_SONG_ITEM(obj);
            const char* path = item->song->path;
            if (g_strcmp0(path, old_path) == 0 || g_strcmp0(path, new_path) == 0) {
                g_list_store_remove(ui->queue_store, i);
                g_list_store_insert(ui->queue_store, i, obj);
            }
            g_object_unref(obj);
        }
    }
}

void ui_refresh_view(MmpUI* ui) {
    if (!ui->song_store) return;

    g_list_store_remove_all(ui->song_store);

    GList* songs = ui_get_filtered_songs(ui);
    for (GList* l = songs; l; l = l->next) {
        MmpSongItem* item = mmp_song_item_new(l->data);
        g_list_store_append(ui->song_store, G_OBJECT(item));
        g_object_unref(item);
    }
    g_list_free(songs);
}

void ui_update_search_lowered_text(MmpUI* ui, GtkSearchEntry* entry) {
    g_free(ui->search_lowered_text);
    ui->search_lowered_text = NULL;

    const char* search_text = gtk_editable_get_text(GTK_EDITABLE(entry));
    if (search_text && search_text[0] != '\0') {
        ui->search_lowered_text = g_utf8_strdown(search_text, -1);
    }
}

bool search_filter_func(Song* song, gpointer user_data) {
    MmpUI* ui = user_data;
    const char* search_lower = ui->search_lowered_text;
    if (!search_lower || search_lower[0] == '\0') return true;

    char* title_lower = g_utf8_strdown(song->title, -1);
    char* artist_lower = g_utf8_strdown(song->artist, -1);
    char* album_lower = g_utf8_strdown(song->album, -1);

    bool visible = (strstr(title_lower, search_lower) != NULL) ||
                   (strstr(artist_lower, search_lower) != NULL) ||
                   (strstr(album_lower, search_lower) != NULL);

    g_free(title_lower);
    g_free(artist_lower);
    g_free(album_lower);

    return visible;
}

bool artist_filter_func(Song* song, gpointer user_data) {
    const char* artist = user_data;
    return g_strcmp0(song->artist, artist) == 0;
}

bool album_filter_func(Song* song, gpointer user_data) {
    const char* album = user_data;
    return g_strcmp0(song->album, album) == 0;
}

static void on_lib_queue_changed(MmpLibrary* lib, gpointer user_data) {
    (void)lib;
    MmpUI* ui = (MmpUI*)user_data;
    ui_update_queue(ui);
}

static void on_lib_now_playing_changed(MmpLibrary* lib, const Song* song, gpointer user_data) {
    (void)lib;
    MmpUI* ui = (MmpUI*)user_data;

    const char *old_path = ui->last_playing_path;
    ui_update_now_playing(ui, old_path);
    g_free(ui->last_playing_path);
    ui->last_playing_path = song ? g_strdup(song->path) : NULL;

    if (!song) {
        gtk_label_set_label(ui->current_track_label, "No track playing");
        gtk_button_set_icon_name(ui->play_pause_button, "media-playback-start-symbolic");
        gtk_widget_set_tooltip_text(GTK_WIDGET(ui->play_pause_button), "Play");
    } else {
        char* label = g_strdup_printf("%s - %s", song->artist, song->title);
        gtk_label_set_label(ui->current_track_label, label);
        g_free(label);
        gtk_button_set_icon_name(ui->play_pause_button, "media-playback-pause-symbolic");
        gtk_widget_set_tooltip_text(GTK_WIDGET(ui->play_pause_button), "Pause");
    }
}

static void on_lib_song_added(MmpLibrary* lib, const Song* song, gpointer user_data) {
    (void)lib;
    MmpUI* ui = user_data;
    add_to_library_ui(ui, (Song*)song);
    if (ui->current_view_base_list == mmp_library_get_all_songs(ui->library) && ui->song_store) {
        bool pass = true;
        for (GList* f = ui->current_view_filters; f; f = f->next) {
            SongFilter* filter = f->data;
            if (!filter->filter((Song*)song, filter->user_data)) { pass = false; break; }
        }
        if (pass) {
            MmpSongItem* item = g_object_new(MMP_TYPE_SONG_ITEM, NULL);
            item->song = (Song*)song;
            if (ui->current_view_reverse)
                g_list_store_insert(ui->song_store, 0, G_OBJECT(item));
            else
                g_list_store_append(ui->song_store, G_OBJECT(item));
            g_object_unref(item);
        }
    }
}

static void on_lib_song_updated(MmpLibrary* lib, const Song* song, gpointer user_data) {
    (void)lib;
    MmpUI* ui = user_data;
    if (ui->song_store) {
        guint n = g_list_model_get_n_items(G_LIST_MODEL(ui->song_store));
        for (guint i = 0; i < n; i++) {
            GObject* obj = g_list_model_get_item(G_LIST_MODEL(ui->song_store), i);
            MmpSongItem* item = MMP_SONG_ITEM(obj);
            if (g_strcmp0(item->song->path, song->path) == 0) {
                g_list_store_remove(ui->song_store, i);
                g_list_store_insert(ui->song_store, i, obj);
                g_object_unref(obj);
                break;
            }
            g_object_unref(obj);
        }
    }
}

static void on_lib_playlists_changed(MmpLibrary* lib, gpointer user_data) {
    (void)lib;
    ui_update_playlists((MmpUI*)user_data);
}

static gboolean tick_cb(gpointer user_data) {
    MmpUI* ui = user_data;
    MmpPlayback* pb = ui->playback;
    if (!pb) return TRUE;
    double pos = mmp_playback_get_position(pb);
    double dur = mmp_playback_get_duration(pb);
    if (dur > 0) {
        ui->is_programmatic_change = true;
        gtk_range_set_range(GTK_RANGE(ui->track_progress_scale), 0, dur);
        gtk_range_set_value(GTK_RANGE(ui->track_progress_scale), pos);
        ui->is_programmatic_change = false;
    }
    char* pos_str = g_strdup_printf("%d:%02d", (int)pos / 60, (int)pos % 60);
    char* dur_str = g_strdup_printf("%d:%02d", (int)dur / 60, (int)dur % 60);
    gtk_label_set_label(ui->elapsed_time_label, pos_str);
    gtk_label_set_label(ui->duration_label, dur_str);
    g_free(pos_str);
    g_free(dur_str);
    return TRUE;
}

static void load_css(GtkWindow* window) {
    GtkCssProvider* provider = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(provider, "/mmp/ui/style.css");
    gtk_style_context_add_provider_for_display(
        gtk_widget_get_display(GTK_WIDGET(window)),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(provider);
}

void ui_update_playlists(MmpUI* ui) {
    GtkWidget* nav_list = GTK_WIDGET(ui->navigation_list);
    if (!nav_list) return;

    gtk_widget_set_visible(nav_list, FALSE);

    GtkWidget* child = gtk_widget_get_first_child(nav_list);
    while (child) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        if (g_object_get_data(G_OBJECT(child), "is-playlist-row")) {
            gtk_list_box_remove(GTK_LIST_BOX(nav_list), child);
        }
        child = next;
    }

    GtkWidget* header_row = GTK_WIDGET(g_object_get_data(G_OBJECT(ui->window), "nav-playlists-row"));
    if (!header_row) {
        gtk_widget_set_visible(nav_list, TRUE);
        return;
    }
    int index = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(header_row));

    GList* playlists = mmp_library_get_playlists(ui->library);
    int i = 1;
    for (GList* l = playlists; l != NULL; l = l->next) {
        Playlist* p = l->data;
        GtkWidget* row = gtk_list_box_row_new();
        g_object_set_data(G_OBJECT(row), "is-playlist-row", GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(row), "stack-page", (gpointer)"songs-view");
        g_object_set_data_full(G_OBJECT(row), "playlist", p, (GDestroyNotify)free_playlist);

        GtkWidget* label = gtk_label_new(p->name);
        gtk_label_set_xalign(GTK_LABEL(label), 0);
        gtk_widget_add_css_class(label, "row-label");
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), label);
        gtk_widget_add_css_class(row, "nav-sub-row");

        gtk_list_box_insert(GTK_LIST_BOX(nav_list), row, index + i++);

        GtkGesture* right_click = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(right_click), GDK_BUTTON_SECONDARY);
        g_signal_connect(right_click, "released", G_CALLBACK(playlist_row_right_clicked_cb), row);
        gtk_widget_add_controller(row, GTK_EVENT_CONTROLLER(right_click));

        GtkGesture* double_click = gtk_gesture_click_new();
        g_signal_connect(double_click, "released", G_CALLBACK(playlist_row_double_clicked_cb), row);
        gtk_widget_add_controller(row, GTK_EVENT_CONTROLLER(double_click));
    }
    g_list_free(playlists);

    gtk_widget_set_visible(nav_list, TRUE);
}

static GtkListBoxRow* create_nav_row(const char* label_text, const char* page_name) {
    GtkListBoxRow* row = GTK_LIST_BOX_ROW(gtk_list_box_row_new());
    GtkWidget* label = gtk_label_new(label_text);
    gtk_label_set_xalign(GTK_LABEL(label), 0);
    gtk_widget_add_css_class(label, "row-label");
    gtk_list_box_row_set_child(row, label);
    g_object_set_data(G_OBJECT(row), "stack-page", (gpointer)page_name);
    return row;
}

static GtkWidget* create_library_panel(const char* search_placeholder, GtkListBox** out_list, GtkSearchEntry** out_search) {
    GtkWidget* page_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(page_box, "content-page");

    GtkWidget* panel_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(panel_box, TRUE);
    gtk_widget_add_css_class(panel_box, "library-panel");
    gtk_box_append(GTK_BOX(page_box), panel_box);

    if (search_placeholder) {
        GtkWidget* search_entry = gtk_search_entry_new();
        gtk_search_entry_set_placeholder_text(GTK_SEARCH_ENTRY(search_entry), search_placeholder);
        gtk_box_append(GTK_BOX(panel_box), search_entry);
        if (out_search) *out_search = GTK_SEARCH_ENTRY(search_entry);
    }

    GtkWidget* scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_box_append(GTK_BOX(panel_box), scrolled);

    GtkWidget* list = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_SINGLE);
    gtk_widget_add_css_class(list, "library-list");
    gtk_widget_add_css_class(list, "boxed-list");
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), list);
    if (out_list) *out_list = GTK_LIST_BOX(list);

    return page_box;
}

MmpUI* mmp_ui_new(GtkApplication* app, MmpLibrary* lib, MmpPlayback* pb) {
    MmpUI* ui = g_new0(MmpUI, 1);
    ui->library = lib;
    ui->playback = pb;

    ui->artists_set = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    ui->albums_set  = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    ui->window = GTK_WINDOW(gtk_application_window_new(app));
    g_object_set_data(G_OBJECT(ui->window), "mmp-ui", ui);
    gtk_window_set_title(ui->window, "My Music Player (mmp)");
    gtk_window_set_default_size(ui->window, 900, 600);

    GtkWidget* root_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(root_box, "app-root");
    gtk_window_set_child(ui->window, root_box);

    GtkWidget* playback_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(playback_bar, "playback-bar");
    gtk_box_append(GTK_BOX(root_box), playback_bar);

    GtkWidget* controls_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_valign(controls_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(playback_bar), controls_box);

    GtkButton* prev_button = GTK_BUTTON(gtk_button_new_from_icon_name("media-skip-backward-symbolic"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(prev_button), "Previous track");
    gtk_widget_set_valign(GTK_WIDGET(prev_button), GTK_ALIGN_CENTER);
    gtk_widget_add_css_class(GTK_WIDGET(prev_button), "playback-button");
    gtk_box_append(GTK_BOX(controls_box), GTK_WIDGET(prev_button));

    ui->play_pause_button = GTK_BUTTON(gtk_button_new_from_icon_name("media-playback-start-symbolic"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->play_pause_button), "Play");
    gtk_widget_set_valign(GTK_WIDGET(ui->play_pause_button), GTK_ALIGN_CENTER);
    gtk_widget_add_css_class(GTK_WIDGET(ui->play_pause_button), "playback-button");
    gtk_box_append(GTK_BOX(controls_box), GTK_WIDGET(ui->play_pause_button));

    GtkButton* next_button = GTK_BUTTON(gtk_button_new_from_icon_name("media-skip-forward-symbolic"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(next_button), "Next track");
    gtk_widget_set_valign(GTK_WIDGET(next_button), GTK_ALIGN_CENTER);
    gtk_widget_add_css_class(GTK_WIDGET(next_button), "playback-button");
    gtk_box_append(GTK_BOX(controls_box), GTK_WIDGET(next_button));

    ui->repeat_button = GTK_BUTTON(gtk_button_new_from_icon_name("media-playlist-repeat-symbolic"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->repeat_button), "Repeat");
    gtk_widget_set_valign(GTK_WIDGET(ui->repeat_button), GTK_ALIGN_CENTER);
    gtk_widget_add_css_class(GTK_WIDGET(ui->repeat_button), "playback-button");
    gtk_box_append(GTK_BOX(controls_box), GTK_WIDGET(ui->repeat_button));

    ui->shuffle_button = GTK_BUTTON(gtk_button_new_from_icon_name("media-playlist-shuffle-symbolic"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->shuffle_button), "Shuffle");
    gtk_widget_set_valign(GTK_WIDGET(ui->shuffle_button), GTK_ALIGN_CENTER);
    gtk_widget_add_css_class(GTK_WIDGET(ui->shuffle_button), "playback-button");
    gtk_box_append(GTK_BOX(controls_box), GTK_WIDGET(ui->shuffle_button));

    GtkWidget* info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_hexpand(info_box, TRUE);
    gtk_widget_add_css_class(info_box, "track-info");
    gtk_box_append(GTK_BOX(playback_bar), info_box);

    ui->current_track_label = GTK_LABEL(gtk_label_new("No track selected"));
    gtk_label_set_xalign(ui->current_track_label, 0);
    gtk_label_set_ellipsize(ui->current_track_label, PANGO_ELLIPSIZE_END);
    PangoAttrList* attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    gtk_label_set_attributes(ui->current_track_label, attrs);
    pango_attr_list_unref(attrs);
    gtk_box_append(GTK_BOX(info_box), GTK_WIDGET(ui->current_track_label));

    GtkWidget* progress_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_append(GTK_BOX(info_box), progress_box);

    ui->elapsed_time_label = GTK_LABEL(gtk_label_new("0:00"));
    gtk_widget_add_css_class(GTK_WIDGET(ui->elapsed_time_label), "time-label");
    gtk_box_append(GTK_BOX(progress_box), GTK_WIDGET(ui->elapsed_time_label));

    GtkAdjustment* progress_adj = gtk_adjustment_new(0, 0, 100, 1, 10, 0);
    ui->track_progress_scale = GTK_SCALE(gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, progress_adj));
    gtk_widget_set_hexpand(GTK_WIDGET(ui->track_progress_scale), TRUE);
    gtk_scale_set_draw_value(ui->track_progress_scale, FALSE);
    gtk_box_append(GTK_BOX(progress_box), GTK_WIDGET(ui->track_progress_scale));

    ui->duration_label = GTK_LABEL(gtk_label_new("0:00"));
    gtk_widget_add_css_class(GTK_WIDGET(ui->duration_label), "time-label");
    gtk_box_append(GTK_BOX(progress_box), GTK_WIDGET(ui->duration_label));

    GtkWidget* volume_controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_valign(volume_controls, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(playback_bar), volume_controls);

    GtkRevealer* volume_revealer = GTK_REVEALER(gtk_revealer_new());
    gtk_revealer_set_reveal_child(volume_revealer, FALSE);
    gtk_revealer_set_transition_type(volume_revealer, GTK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT);
    gtk_box_append(GTK_BOX(volume_controls), GTK_WIDGET(volume_revealer));

    GtkAdjustment* volume_adj = gtk_adjustment_new(70, 0, 100, 1, 10, 0);
    GtkWidget* volume_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, volume_adj);
    gtk_widget_add_css_class(volume_scale, "volume-scale");
    gtk_scale_set_draw_value(GTK_SCALE(volume_scale), FALSE);
    gtk_revealer_set_child(volume_revealer, volume_scale);

    GtkButton* mute_button = GTK_BUTTON(gtk_button_new_from_icon_name("audio-volume-medium-symbolic"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(mute_button), "Mute");
    gtk_widget_add_css_class(GTK_WIDGET(mute_button), "volume-button");
    gtk_box_append(GTK_BOX(volume_controls), GTK_WIDGET(mute_button));

    GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(root_box), separator);

    GtkWidget* main_shell = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_vexpand(main_shell, TRUE);
    gtk_widget_add_css_class(main_shell, "main-shell");
    gtk_box_append(GTK_BOX(root_box), main_shell);

    GtkWidget* nav_pane = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(nav_pane, "nav-pane");
    gtk_box_append(GTK_BOX(main_shell), nav_pane);

    ui->navigation_list = GTK_LIST_BOX(gtk_list_box_new());
    gtk_widget_set_vexpand(GTK_WIDGET(ui->navigation_list), TRUE);
    gtk_list_box_set_selection_mode(ui->navigation_list, GTK_SELECTION_SINGLE);
    gtk_widget_add_css_class(GTK_WIDGET(ui->navigation_list), "navigation-list");
    gtk_box_append(GTK_BOX(nav_pane), GTK_WIDGET(ui->navigation_list));

    GtkListBoxRow* nav_recently_added_row = create_nav_row("Recently added", "songs-view");
    g_object_set_data(G_OBJECT(nav_recently_added_row), "view-mode", (gpointer)"recently-added");
    gtk_list_box_append(ui->navigation_list, GTK_WIDGET(nav_recently_added_row));

    GtkListBoxRow* nav_albums_row = create_nav_row("Albums", "albums");
    gtk_list_box_append(ui->navigation_list, GTK_WIDGET(nav_albums_row));

    GtkListBoxRow* nav_artists_row = create_nav_row("Artists", "artists");
    gtk_list_box_append(ui->navigation_list, GTK_WIDGET(nav_artists_row));

    GtkListBoxRow* nav_songs_row = create_nav_row("Songs", "songs-view");
    g_object_set_data(G_OBJECT(nav_songs_row), "view-mode", (gpointer)"songs");
    gtk_list_box_append(ui->navigation_list, GTK_WIDGET(nav_songs_row));

    GtkListBoxRow* nav_queue_row = create_nav_row("Queue", "queue");
    gtk_list_box_append(ui->navigation_list, GTK_WIDGET(nav_queue_row));

    GtkListBoxRow* nav_playlists_row = GTK_LIST_BOX_ROW(gtk_list_box_row_new());
    gtk_list_box_row_set_selectable(nav_playlists_row, FALSE);
    gtk_list_box_row_set_activatable(nav_playlists_row, FALSE);
    gtk_widget_add_css_class(GTK_WIDGET(nav_playlists_row), "nav-header");
    GtkWidget* playlists_label = gtk_label_new("Playlists");
    gtk_label_set_xalign(GTK_LABEL(playlists_label), 0);
    gtk_widget_add_css_class(playlists_label, "row-label");
    gtk_list_box_row_set_child(nav_playlists_row, playlists_label);
    gtk_list_box_append(ui->navigation_list, GTK_WIDGET(nav_playlists_row));
    g_object_set_data(G_OBJECT(ui->window), "nav-playlists-row", nav_playlists_row);

    GtkListBoxRow* nav_settings_row = create_nav_row("Settings", "settings");
    gtk_list_box_append(ui->navigation_list, GTK_WIDGET(nav_settings_row));

    ui->content_stack = GTK_STACK(gtk_stack_new());
    gtk_widget_set_hexpand(GTK_WIDGET(ui->content_stack), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(ui->content_stack), TRUE);
    gtk_stack_set_transition_type(ui->content_stack, GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_widget_add_css_class(GTK_WIDGET(ui->content_stack), "content-stack");
    gtk_box_append(GTK_BOX(main_shell), GTK_WIDGET(ui->content_stack));

    {
        GtkWidget* page_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_add_css_class(page_box, "content-page");

        GtkWidget* panel_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_vexpand(panel_box, TRUE);
        gtk_widget_add_css_class(panel_box, "library-panel");
        gtk_box_append(GTK_BOX(page_box), panel_box);

        ui->songs_search_entry = GTK_SEARCH_ENTRY(gtk_search_entry_new());
        gtk_search_entry_set_placeholder_text(ui->songs_search_entry, "Search songs");
        gtk_box_append(GTK_BOX(panel_box), GTK_WIDGET(ui->songs_search_entry));

        GtkWidget* scrolled = gtk_scrolled_window_new();
        gtk_widget_set_vexpand(scrolled, TRUE);
        gtk_box_append(GTK_BOX(panel_box), scrolled);

        ui->song_store = g_list_store_new(MMP_TYPE_SONG_ITEM);
        GtkSingleSelection* sel = gtk_single_selection_new(G_LIST_MODEL(ui->song_store));
        GtkListItemFactory* factory = gtk_signal_list_item_factory_new();
        g_signal_connect(factory, "setup", G_CALLBACK(song_factory_setup), NULL);
        g_signal_connect(factory, "bind", G_CALLBACK(song_factory_bind), ui);
        ui->song_view = GTK_LIST_VIEW(gtk_list_view_new(GTK_SELECTION_MODEL(sel), factory));
        gtk_widget_add_css_class(GTK_WIDGET(ui->song_view), "library-list");
        gtk_widget_add_css_class(GTK_WIDGET(ui->song_view), "boxed-list");
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), GTK_WIDGET(ui->song_view));

        gtk_stack_add_titled(ui->content_stack, page_box, "songs-view", "Songs");
    }

    GtkWidget* albums_page = create_library_panel(NULL, &ui->albums_list, NULL);
    gtk_list_box_set_selection_mode(ui->albums_list, GTK_SELECTION_NONE);
    gtk_stack_add_titled(ui->content_stack, albums_page, "albums", "Albums");

    GtkWidget* artists_page = create_library_panel(NULL, &ui->artists_list, NULL);
    gtk_list_box_set_selection_mode(ui->artists_list, GTK_SELECTION_NONE);
    gtk_stack_add_titled(ui->content_stack, artists_page, "artists", "Artists");

    {
        GtkWidget* page_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
        gtk_widget_add_css_class(page_box, "content-page");

        GtkWidget* scrolled = gtk_scrolled_window_new();
        gtk_widget_set_vexpand(scrolled, TRUE);
        gtk_box_append(GTK_BOX(page_box), scrolled);

        ui->queue_store = g_list_store_new(MMP_TYPE_SONG_ITEM);
        GtkSingleSelection* sel = gtk_single_selection_new(G_LIST_MODEL(ui->queue_store));
        GtkListItemFactory* factory = gtk_signal_list_item_factory_new();
        g_signal_connect(factory, "setup", G_CALLBACK(queue_factory_setup), NULL);
        g_signal_connect(factory, "bind", G_CALLBACK(queue_factory_bind), ui);
        ui->queue_view = GTK_LIST_VIEW(gtk_list_view_new(GTK_SELECTION_MODEL(sel), factory));
        gtk_widget_add_css_class(GTK_WIDGET(ui->queue_view), "library-list");
        gtk_widget_add_css_class(GTK_WIDGET(ui->queue_view), "boxed-list");
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), GTK_WIDGET(ui->queue_view));

        gtk_stack_add_titled(ui->content_stack, page_box, "queue", "Queue");
    }

    GtkWidget* settings_page_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(settings_page_box, "content-page");

    GtkWidget* scan_checkbox = gtk_check_button_new_with_label("Scan music folder on startup");
    gtk_box_append(GTK_BOX(settings_page_box), scan_checkbox);
    gtk_stack_add_titled(ui->content_stack, settings_page_box, "settings", "Settings");

    const GActionEntry app_actions[] = {
        { "create-playlist", create_playlist_action_cb, NULL, NULL, NULL, {0, 0, 0} }
    };
    g_action_map_add_action_entries(G_ACTION_MAP(app), app_actions, G_N_ELEMENTS(app_actions), ui);

    gtk_list_box_set_filter_func(ui->albums_list, filter_albums_cb, ui, NULL);
    g_signal_connect(ui->songs_search_entry, "search-changed", G_CALLBACK(search_changed_cb), ui);
    g_signal_connect(ui->song_view, "activate", G_CALLBACK(song_view_activate_cb), ui);
    g_signal_connect(ui->albums_list, "row-activated", G_CALLBACK(album_row_activated_cb), ui);
    g_signal_connect(ui->artists_list, "row-activated", G_CALLBACK(artist_row_activated_cb), ui);
    g_signal_connect(ui->queue_view, "activate", G_CALLBACK(queue_view_activate_cb), ui);

    GtkGesture* playlists_right_click = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(playlists_right_click), GDK_BUTTON_SECONDARY);
    g_signal_connect(playlists_right_click, "released", G_CALLBACK(playlists_header_right_clicked_cb), ui);
    gtk_widget_add_controller(GTK_WIDGET(nav_playlists_row), GTK_EVENT_CONTROLLER(playlists_right_click));

    ui_update_playlists(ui);

    g_object_set_data(G_OBJECT(mute_button), "volume-scale", volume_scale);

    GtkDropTarget* drop_target = gtk_drop_target_new(G_TYPE_FILE, GDK_ACTION_COPY);
    g_signal_connect(drop_target, "drop", G_CALLBACK(on_drop_cb), ui);
    gtk_widget_add_controller(GTK_WIDGET(ui->window), GTK_EVENT_CONTROLLER(drop_target));

    GtkEventController* volume_motion = gtk_event_controller_motion_new();

    g_signal_connect(ui->navigation_list, "row-selected", G_CALLBACK(navigation_row_selected_cb), ui);

    gtk_list_box_select_row(ui->navigation_list, nav_recently_added_row);
    load_css(ui->window);

    g_signal_connect(volume_motion, "enter", G_CALLBACK(volume_controls_enter_cb), volume_revealer);
    g_signal_connect(volume_motion, "leave", G_CALLBACK(volume_controls_leave_cb), volume_revealer);
    gtk_widget_add_controller(volume_controls, volume_motion);

    g_signal_connect(mute_button, "clicked", G_CALLBACK(mute_button_clicked_cb), ui);
    g_signal_connect(ui->shuffle_button, "clicked", G_CALLBACK(shuffle_clicked_cb), ui);
    g_signal_connect(ui->repeat_button, "clicked", G_CALLBACK(repeat_clicked_cb), ui);
    g_signal_connect(ui->play_pause_button, "clicked", G_CALLBACK(play_pause_clicked_cb), ui);
    g_signal_connect(prev_button, "clicked", G_CALLBACK(previous_track_clicked_cb), ui);
    g_signal_connect(next_button, "clicked", G_CALLBACK(next_track_clicked_cb), ui);
    g_signal_connect(volume_scale, "value-changed", G_CALLBACK(volume_scale_changed_cb), ui);
    g_signal_connect(ui->track_progress_scale, "value-changed", G_CALLBACK(track_progress_scale_value_changed_cb), ui);

    ui->tick_timer_id = g_timeout_add(500, tick_cb, ui);

    gtk_window_set_application(ui->window, app);
    gtk_window_present(ui->window);

    return ui;
}

void mmp_ui_connect_library(MmpUI *ui, MmpLibrary *lib)
{
    g_signal_connect(lib, "queue-changed", G_CALLBACK(on_lib_queue_changed), ui);
    g_signal_connect(lib, "now-playing-changed", G_CALLBACK(on_lib_now_playing_changed), ui);
    g_signal_connect(lib, "song-added", G_CALLBACK(on_lib_song_added), ui);
    g_signal_connect(lib, "song-updated", G_CALLBACK(on_lib_song_updated), ui);
    g_signal_connect(lib, "playlists-changed", G_CALLBACK(on_lib_playlists_changed), ui);
}

GtkWindow *mmp_ui_get_window(MmpUI *ui)
{
    return ui ? ui->window : NULL;
}

void mmp_ui_present_window(MmpUI *ui)
{
    if (ui && ui->window)
        gtk_window_present(ui->window);
}

MmpLibrary *mmp_ui_get_library(MmpUI *ui)
{
    return ui ? ui->library : NULL;
}

void mmp_ui_free(MmpUI *ui)
{
    if (!ui) return;

    if (ui->tick_timer_id)
        g_source_remove(ui->tick_timer_id);

    g_list_free_full(ui->queue_fallback_songs, (GDestroyNotify)free_song);
    g_free(ui->selected_artist_filter);
    g_free(ui->selected_album_filter);
    g_free(ui->search_lowered_text);
    g_free(ui->last_playing_path);
    g_list_free_full(ui->current_view_filters, free_song_filter);

    if (ui->current_view_base_list && ui->current_view_base_list_owned)
        g_list_free(ui->current_view_base_list);

    g_hash_table_unref(ui->artists_set);
    g_hash_table_unref(ui->albums_set);

    g_free(ui);
}
