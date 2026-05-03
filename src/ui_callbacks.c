#include "ui_callbacks.h"
#include "ui.h"
#include "playback.h"
#include "database.h"

#include <stdbool.h>
#include <string.h>

static GList* drag_source_node = NULL;

void queue_drag_begin_cb(GtkDragSource* source, GdkDrag* drag, gpointer user_data) {
    (void)drag; (void)user_data;
    GtkWidget* row = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(source));
    drag_source_node = g_object_get_data(G_OBJECT(row), "playlist-node");
}

gboolean queue_drop_cb(GtkDropTarget* target, const GValue* value, double x, double y, gpointer user_data) {
    (void)value; (void)x; (void)y; (void)user_data;
    GtkWidget* row = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(target));
    GList* target_node = g_object_get_data(G_OBJECT(row), "playlist-node");
    
    if (drag_source_node && target_node && drag_source_node != target_node) {
        char* path = drag_source_node->data;
        g_queue_delete_link(mmp_app->playlist, drag_source_node);
        g_queue_insert_before(mmp_app->playlist, target_node, path);
        
        drag_source_node = NULL;
        ui_update_queue(mmp_app);
        return TRUE;
    }
    return FALSE;
}

static void ui_show_playlist_contents(MmpApp* app, Playlist* p) {
    app->current_playlist_id = p->id;
    GList* songs = db_get_playlist_songs(app->db, p->id);
    
    app->current_view_base_list = songs;
    app->current_view_reverse = false;
    ui_clear_filters(app);
    
    const char* search_text = gtk_editable_get_text(GTK_EDITABLE(app->songs_search_entry));
    if (search_text && strlen(search_text) > 0) {
        ui_add_filter(app, search_filter_func, (gpointer)search_text, NULL);
    }

    ui_refresh_view(app);
    // Note: in a real app, we'd need to manage the lifecycle of 'songs' GList and its Song objects.
}

static void song_properties_cb(GtkWidget* widget, gpointer user_data) {
    (void)widget;
    Song* song = user_data;
    GtkWindow* parent = mmp_app->window;

    char* message = g_strdup_printf(
        "Artist: %s\nAlbum: %s\nPath: %s",
        song->artist, song->album, song->path
    );

    GtkAlertDialog* dialog = gtk_alert_dialog_new("%s", song->title);
    gtk_alert_dialog_set_detail(dialog, message);
    gtk_alert_dialog_show(dialog, parent);
    g_object_unref(dialog);

    g_free(message);
}

static void song_play_now_action_cb(GSimpleAction* action, GVariant* parameter, gpointer user_data) {    (void)action; (void)parameter;
    Song* song = user_data;
    playback_open_file(mmp_app, song->path);
}

static void song_play_next_action_cb(GSimpleAction* action, GVariant* parameter, gpointer user_data) {
    (void)action; (void)parameter;
    Song* song = user_data;
    playback_play_next(mmp_app, song->path);
}

static void song_add_to_queue_action_cb(GSimpleAction* action, GVariant* parameter, gpointer user_data) {
    (void)action; (void)parameter;
    Song* song = user_data;
    playback_add_to_playlist(mmp_app, song->path, false);
}

static void song_properties_action_cb(GSimpleAction* action, GVariant* parameter, gpointer user_data) {
    (void)action; (void)parameter;
    Song* song = user_data;
    song_properties_cb(NULL, song);
}

static void song_add_to_playlist_action_cb(GSimpleAction* action, GVariant* parameter, gpointer user_data) {
    (void)action;
    Song* song = user_data;
    int playlist_id = g_variant_get_int32(parameter);
    
    if (db_add_song_to_playlist(mmp_app->db, playlist_id, song)) {
        if (mmp_app->current_playlist_id == playlist_id) {
            // Refresh current playlist view
            GList* playlists = db_get_playlists(mmp_app->db);
            for (GList* l = playlists; l != NULL; l = l->next) {
                Playlist* p = l->data;
                if (p->id == playlist_id) {
                    ui_show_playlist_contents(mmp_app, p);
                    break;
                }
            }
            g_list_free_full(playlists, (GDestroyNotify)free_playlist);
        }
    }
}

static void song_remove_from_playlist_action_cb(GSimpleAction* action, GVariant* parameter, gpointer user_data) {
    (void)action; (void)parameter;
    Song* song = user_data;
    if (db_remove_song_from_playlist(mmp_app->db, mmp_app->current_playlist_id, song->path)) {
        // Refresh view
        GList* playlists = db_get_playlists(mmp_app->db);
        for (GList* l = playlists; l != NULL; l = l->next) {
            Playlist* p = l->data;
            if (p->id == mmp_app->current_playlist_id) {
                ui_show_playlist_contents(mmp_app, p);
                break;
            }
        }
        g_list_free_full(playlists, (GDestroyNotify)free_playlist);
    }
}

static void show_song_context_menu(Song* song, double x, double y, GtkWidget* parent_row) {
    GtkWidget* parent_list = gtk_widget_get_parent(parent_row);
    bool in_playlist_view = (mmp_app->current_playlist_id > 0);

    GSimpleActionGroup* action_group = g_simple_action_group_new();

    const GActionEntry actions[] = {
        { "play_now", song_play_now_action_cb, NULL, NULL, NULL, {0, 0, 0} },
        { "play_next", song_play_next_action_cb, NULL, NULL, NULL, {0, 0, 0} },
        { "add_queue", song_add_to_queue_action_cb, NULL, NULL, NULL, {0, 0, 0} },
        { "properties", song_properties_action_cb, NULL, NULL, NULL, {0, 0, 0} },
        { "add_to_playlist", song_add_to_playlist_action_cb, "i", NULL, NULL, {0, 0, 0} },
        { "remove_from_playlist", song_remove_from_playlist_action_cb, NULL, NULL, NULL, {0, 0, 0} }
    };
    g_action_map_add_action_entries(G_ACTION_MAP(action_group), actions, G_N_ELEMENTS(actions), song);
    gtk_widget_insert_action_group(parent_row, "song", G_ACTION_GROUP(action_group));

    GMenu* menu = g_menu_new();
    g_menu_append(menu, "Play Now", "song.play_now");
    g_menu_append(menu, "Play Next", "song.play_next");
    g_menu_append(menu, "Add to Queue", "song.add_queue");
    g_menu_append(menu, "Properties", "song.properties");

    if (in_playlist_view) {
        g_menu_append(menu, "Remove from Playlist", "song.remove_from_playlist");
    } else {
        GList* playlists = db_get_playlists(mmp_app->db);
        if (playlists) {
            GMenu* playlist_menu = g_menu_new();
            for (GList* l = playlists; l != NULL; l = l->next) {
                Playlist* p = l->data;
                GMenuItem* item = g_menu_item_new(p->name, NULL);
                g_menu_item_set_action_and_target(item, "song.add_to_playlist", "i", p->id);
                g_menu_append_item(playlist_menu, item);
                g_object_unref(item);
            }
            g_menu_append_submenu(menu, "Add to Playlist", G_MENU_MODEL(playlist_menu));
            g_object_unref(playlist_menu);
            g_list_free_full(playlists, (GDestroyNotify)free_playlist);
        }
    }

    GtkWidget* popover = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
    gtk_widget_set_parent(popover, parent_row);
    
    GdkRectangle rect = {(int)x, (int)y, 1, 1};
    gtk_popover_set_pointing_to(GTK_POPOVER(popover), &rect);
    gtk_popover_popup(GTK_POPOVER(popover));

    g_object_unref(menu);
    g_object_unref(action_group);
}

void song_row_secondary_click_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer user_data) {
    (void)user_data;
    if (n_press != 1) return;
    GtkWidget* row = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
    Song* song = g_object_get_data(G_OBJECT(row), "song-data");
    if (song) {
        show_song_context_menu(song, x, y, row);
    }
}

static void queue_play_now_action_cb(GSimpleAction* action, GVariant* parameter, gpointer user_data) {
    (void)action; (void)parameter;
    GList* node = user_data;
    playback_play_track(mmp_app, node);
}

static void queue_remove_action_cb(GSimpleAction* action, GVariant* parameter, gpointer user_data) {
    (void)action; (void)parameter;
    GList* node = user_data;
    playback_remove_from_playlist(mmp_app, node);
}

static void queue_clear_action_cb(GSimpleAction* action, GVariant* parameter, gpointer user_data) {
    (void)action; (void)parameter; (void)user_data;
    playback_clear_playlist(mmp_app);
}

static void show_queue_context_menu(GList* node, double x, double y, GtkWidget* parent_row) {
    GSimpleActionGroup* action_group = g_simple_action_group_new();
    const GActionEntry actions[] = {
        { "play_now", queue_play_now_action_cb, NULL, NULL, NULL, {0, 0, 0} },
        { "remove", queue_remove_action_cb, NULL, NULL, NULL, {0, 0, 0} },
        { "clear", queue_clear_action_cb, NULL, NULL, NULL, {0, 0, 0} }
    };
    g_action_map_add_action_entries(G_ACTION_MAP(action_group), actions, G_N_ELEMENTS(actions), node);
    gtk_widget_insert_action_group(parent_row, "queue", G_ACTION_GROUP(action_group));

    GMenu* menu = g_menu_new();
    g_menu_append(menu, "Play Now", "queue.play_now");
    g_menu_append(menu, "Remove from Queue", "queue.remove");
    
    GMenu* section = g_menu_new();
    g_menu_append(section, "Clear Queue", "queue.clear");
    g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
    g_object_unref(section);

    GtkWidget* popover = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
    gtk_widget_set_parent(popover, parent_row);
    
    GdkRectangle rect = {(int)x, (int)y, 1, 1};
    gtk_popover_set_pointing_to(GTK_POPOVER(popover), &rect);
    gtk_popover_popup(GTK_POPOVER(popover));

    g_object_unref(menu);
    g_object_unref(action_group);
}

void queue_row_secondary_click_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer user_data) {
    (void)user_data;
    if (n_press != 1) return;
    GtkWidget* row = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
    GList* node = g_object_get_data(G_OBJECT(row), "playlist-node");
    if (node) {
        show_queue_context_menu(node, x, y, row);
    }
}

static void play_song(MmpApp* app, Song* song) {
    playback_open_file(app, song->path);
}

gboolean filter_albums_cb(GtkListBoxRow* row, gpointer user_data) {
    MmpApp* app = user_data;
    if (!app->selected_artist_filter) return TRUE;

    const char* album_artist = g_object_get_data(G_OBJECT(row), "album-artist");
    if (album_artist && g_strcmp0(album_artist, app->selected_artist_filter) == 0) {
        return TRUE;
    }
    return FALSE;
}

void search_changed_cb(GtkSearchEntry* entry, gpointer user_data) {
    (void)entry;
    MmpApp* app = user_data;
    ui_refresh_view(app);
}

void artist_row_activated_cb(GtkListBox* list, GtkListBoxRow* row, gpointer user_data) {
    (void)list;
    MmpApp* app = user_data;
    GtkWidget* label = gtk_list_box_row_get_child(row);
    if (GTK_IS_BOX(label)) label = gtk_widget_get_first_child(label);
    const char* artist = gtk_label_get_text(GTK_LABEL(label));
    
    g_free(app->selected_artist_filter);
    app->selected_artist_filter = g_strdup(artist);
    
    g_free(app->selected_album_filter);
    app->selected_album_filter = NULL;
    
    if (app->albums_list) gtk_list_box_invalidate_filter(app->albums_list);
    
    if (app->content_stack) {
        gtk_stack_set_visible_child_name(app->content_stack, "albums");
    }
}

void album_row_activated_cb(GtkListBox* list, GtkListBoxRow* row, gpointer user_data) {
    (void)list;
    MmpApp* app = user_data;
    GtkWidget* label = gtk_list_box_row_get_child(row);
    if (GTK_IS_BOX(label)) label = gtk_widget_get_first_child(label);
    const char* album = gtk_label_get_text(GTK_LABEL(label));
    
    g_free(app->selected_album_filter);
    app->selected_album_filter = g_strdup(album);
    
    app->current_view_base_list = app->library;
    app->current_view_reverse = false;
    ui_clear_filters(app);
    
    if (app->selected_artist_filter) {
        ui_add_filter(app, artist_filter_func, g_strdup(app->selected_artist_filter), g_free);
    }
    ui_add_filter(app, album_filter_func, g_strdup(app->selected_album_filter), g_free);
    
    const char* search_text = gtk_editable_get_text(GTK_EDITABLE(app->songs_search_entry));
    if (search_text && strlen(search_text) > 0) {
        ui_add_filter(app, search_filter_func, (gpointer)search_text, NULL);
    }
    
    ui_refresh_view(app);
    
    if (app->content_stack) {
        gtk_stack_set_visible_child_name(app->content_stack, "songs-view");
    }
}

void song_row_activated_cb(GtkListBox* list, GtkListBoxRow* row, gpointer user_data) {
    (void)list;
    MmpApp* app = user_data;
    Song* song = g_object_get_data(G_OBJECT(row), "song-data");
    if (song) {
        play_song(app, song);
    }
}

void queue_row_activated_cb(GtkListBox* list, GtkListBoxRow* row, gpointer user_data) {
    (void)list;
    MmpApp* app = user_data;
    GList* node = g_object_get_data(G_OBJECT(row), "playlist-node");
    if (node) {
        playback_play_track(app, node);
        ui_update_queue(app);
    }
}

void volume_controls_enter_cb(
    GtkEventControllerMotion* controller,
    double x,
    double y,
    gpointer user_data
) {
    (void)controller;
    (void)x;
    (void)y;

    gtk_revealer_set_reveal_child(GTK_REVEALER(user_data), TRUE);
}

void volume_controls_leave_cb(GtkEventControllerMotion* controller, gpointer user_data) {
    (void)controller;

    gtk_revealer_set_reveal_child(GTK_REVEALER(user_data), FALSE);
}

void mute_button_clicked_cb(GtkButton* button, gpointer user_data) {
    (void)user_data;
    if (!mmp_app) return;

    mmp_app->volume_muted = !mmp_app->volume_muted;

    gtk_button_set_icon_name(
        button,
        mmp_app->volume_muted ? "audio-volume-muted-symbolic" : "audio-volume-medium-symbolic"
    );
    gtk_widget_set_tooltip_text(GTK_WIDGET(button), mmp_app->volume_muted ? "Unmute" : "Mute");
    
    playback_set_mute(mmp_app, mmp_app->volume_muted);
    
    GtkWidget* volume_scale = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "volume-scale"));
    if (volume_scale) {
        gtk_widget_set_sensitive(volume_scale, !mmp_app->volume_muted);
    }
}

void play_pause_clicked_cb(GtkButton* button, gpointer user_data) {
    (void)button;
    playback_toggle_pause((MmpApp*)user_data);
}

void volume_scale_changed_cb(GtkRange* range, gpointer user_data) {
    double volume = gtk_range_get_value(range) / 100.0;
    playback_set_volume((MmpApp*)user_data, volume);
}

void track_progress_scale_value_changed_cb(GtkRange* range, gpointer user_data) {
    MmpApp* app = user_data;
    if (app->is_programmatic_change) return;

    static gint64 last_seek_time = 0;
    gint64 now = g_get_monotonic_time();

    if (now - last_seek_time < 100000) return; 

    double value = gtk_range_get_value(range);
    playback_seek(app, value);
    
    last_seek_time = now;
}

void shuffle_clicked_cb(GtkButton* button, gpointer user_data) {
    (void)button;
    MmpApp* app = user_data;
    playback_shuffle_toggle(app);
    
    if (app->shuffle_mode) {
        gtk_widget_add_css_class(GTK_WIDGET(app->shuffle_button), "active-control");
    } else {
        gtk_widget_remove_css_class(GTK_WIDGET(app->shuffle_button), "active-control");
    }
}

void repeat_clicked_cb(GtkButton* button, gpointer user_data) {
    (void)button;
    MmpApp* app = user_data;
    playback_repeat_toggle(app);
    
    switch (app->repeat_mode) {
        case REPEAT_OFF:
            gtk_button_set_icon_name(app->repeat_button, "media-playlist-repeat-symbolic");
            gtk_widget_remove_css_class(GTK_WIDGET(app->repeat_button), "active-control");
            break;
        case REPEAT_ALL:
            gtk_button_set_icon_name(app->repeat_button, "media-playlist-repeat-symbolic");
            gtk_widget_add_css_class(GTK_WIDGET(app->repeat_button), "active-control");
            break;
        case REPEAT_ONE:
            gtk_button_set_icon_name(app->repeat_button, "media-playlist-repeat-song-symbolic");
            gtk_widget_add_css_class(GTK_WIDGET(app->repeat_button), "active-control");
            break;
    }
}

void previous_track_clicked_cb(GtkButton* button, gpointer user_data) {
    (void)button;
    MmpApp* app = user_data;
    
    gint64 position;
    if (gst_element_query_position(app->playbin, GST_FORMAT_TIME, &position) && position > 3LL * GST_SECOND) {
        playback_seek(app, 0);
        return;
    }

    if (app->current_track_node && app->current_track_node->prev) {
        playback_play_track(app, app->current_track_node->prev);
    }
}

void next_track_clicked_cb(GtkButton* button, gpointer user_data) {
    (void)button;
    MmpApp* app = user_data;
    playback_skip_next(app);
}

typedef void (*EntryCallback)(const char* text, gpointer user_data);

typedef struct {
    GtkWidget* dialog;
    GtkWidget* entry;
    EntryCallback callback;
    gpointer user_data;
} DialogData;

static void on_dialog_ok_clicked(GtkButton* btn, gpointer d) {
    (void)btn;
    DialogData* dd = (DialogData*)d;
    dd->callback(gtk_editable_get_text(GTK_EDITABLE(dd->entry)), dd->user_data);
    gtk_window_destroy(GTK_WINDOW(dd->dialog));
    g_free(dd);
}

static void show_entry_dialog(GtkWindow* parent, const char* title, const char* initial_text, EntryCallback callback, gpointer user_data) {
    GtkWidget* dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, -1);

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(box, "content-page");
    gtk_window_set_child(GTK_WINDOW(dialog), box);

    GtkWidget* entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entry), initial_text ? initial_text : "");
    gtk_box_append(GTK_BOX(box), entry);

    GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(box), button_box);

    GtkWidget* cancel_button = gtk_button_new_with_label("Cancel");
    g_signal_connect_swapped(cancel_button, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_box_append(GTK_BOX(button_box), cancel_button);

    GtkWidget* ok_button = gtk_button_new_with_label("OK");
    gtk_widget_add_css_class(ok_button, "suggested-action");
    gtk_box_append(GTK_BOX(button_box), ok_button);

    DialogData* data = g_new0(DialogData, 1);
    data->dialog = dialog;
    data->entry = entry;
    data->callback = callback;
    data->user_data = user_data;

    g_signal_connect(ok_button, "clicked", G_CALLBACK(on_dialog_ok_clicked), data);
    g_signal_connect(entry, "activate", G_CALLBACK(on_dialog_ok_clicked), data);

    gtk_window_present(GTK_WINDOW(dialog));
}

static void on_rename_playlist_done(const char* name, gpointer user_data) {
    if (!name || strlen(name) == 0) return;
    GtkListBoxRow* row = user_data;
    MmpApp* app = mmp_app; 
    Playlist* p = g_object_get_data(G_OBJECT(row), "playlist");
    if (p && db_rename_playlist(app->db, p->id, name)) {
        ui_update_playlists(app);
    }
}

static void on_create_playlist_done(const char* name, gpointer user_data) {
    if (!name || strlen(name) == 0) return;
    MmpApp* app = user_data;
    if (db_create_playlist(app->db, name, NULL)) {
        ui_update_playlists(app);
    }
}

static void rename_action_activated_cb(GSimpleAction* action, GVariant* parameter, gpointer user_data) {
    (void)action; (void)parameter;
    GtkListBoxRow* row = user_data;
    Playlist* p = g_object_get_data(G_OBJECT(row), "playlist");
    if (p) {
        show_entry_dialog(GTK_WINDOW(mmp_app->window), "Rename Playlist", p->name, on_rename_playlist_done, row);
    }
}

static void remove_action_activated_cb(GSimpleAction* action, GVariant* parameter, gpointer user_data) {
    (void)action; (void)parameter;
    GtkListBoxRow* row = user_data;
    MmpApp* app = mmp_app;
    Playlist* p = g_object_get_data(G_OBJECT(row), "playlist");
    if (p && db_delete_playlist(app->db, p->id)) {
        ui_update_playlists(app);
    }
}

static void show_playlist_context_menu(GtkListBoxRow* row, double x, double y) {
    GSimpleActionGroup* action_group = g_simple_action_group_new();
    const GActionEntry actions[] = {
        { "rename", rename_action_activated_cb, NULL, NULL, NULL, {0, 0, 0} },
        { "remove", remove_action_activated_cb, NULL, NULL, NULL, {0, 0, 0} }
    };
    g_action_map_add_action_entries(G_ACTION_MAP(action_group), actions, G_N_ELEMENTS(actions), row);
    gtk_widget_insert_action_group(GTK_WIDGET(row), "playlist", G_ACTION_GROUP(action_group));

    GMenu* menu = g_menu_new();
    g_menu_append(menu, "Rename", "playlist.rename");
    g_menu_append(menu, "Remove", "playlist.remove");

    GtkWidget* popover = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
    gtk_widget_set_parent(popover, GTK_WIDGET(row));
    
    GdkRectangle rect = {(int)x, (int)y, 1, 1};
    gtk_popover_set_pointing_to(GTK_POPOVER(popover), &rect);
    gtk_popover_popup(GTK_POPOVER(popover));

    g_object_unref(menu);
    g_object_unref(action_group);
}

void playlist_row_right_clicked_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer user_data) {
    (void)n_press;
    GtkListBoxRow* row = user_data;
    show_playlist_context_menu(row, x, y);
}

void playlist_row_double_clicked_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer user_data) {
    (void)x; (void)y;
    if (n_press != 2) return;
    
    GtkListBoxRow* row = user_data;
    Playlist* p = g_object_get_data(G_OBJECT(row), "playlist");
    if (!p) return;

    GList* songs = db_get_playlist_songs(mmp_app->db, p->id);
    if (!songs) return;

    playback_clear_playlist(mmp_app);
    
    GList* start_node_ptr = NULL;
    GList* start_song_data = songs;

    if (mmp_app->shuffle_mode) {
        int len = g_list_length(songs);
        int start_idx = g_random_int_range(0, len);
        start_song_data = g_list_nth(songs, start_idx);
    }

    for (GList* l = songs; l != NULL; l = l->next) {
        Song* s = l->data;
        GList* added_node = playback_add_to_playlist(mmp_app, s->path, false);
        if (l == start_song_data) {
            start_node_ptr = added_node;
        }
    }
    
    if (start_node_ptr) {
        playback_play_track(mmp_app, start_node_ptr);
    }
    
    g_list_free_full(songs, (GDestroyNotify)free_song);
}

static void create_playlist_clicked_cb(GtkButton* button, gpointer user_data) {
    (void)button;
    MmpApp* app = user_data;
    show_entry_dialog(GTK_WINDOW(app->window), "New Playlist", "New Playlist", on_create_playlist_done, app);
}

void create_playlist_action_cb(GSimpleAction* action, GVariant* parameter, gpointer user_data) {
    (void)action; (void)parameter;
    create_playlist_clicked_cb(NULL, user_data);
}

void playlists_header_right_clicked_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer user_data) {
    (void)n_press;
    MmpApp* app = user_data;
    GtkWidget* header_row = GTK_WIDGET(g_object_get_data(G_OBJECT(app->window), "nav-playlists-row"));

    GMenu* menu = g_menu_new();
    g_menu_append(menu, "Create Playlist", "app.create-playlist");

    GtkWidget* popover = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
    gtk_widget_set_parent(popover, header_row);
    
    GdkRectangle rect = {(int)x, (int)y, 1, 1};
    gtk_popover_set_pointing_to(GTK_POPOVER(popover), &rect);
    gtk_popover_popup(GTK_POPOVER(popover));

    g_object_unref(menu);
}

static Song* find_song_in_library(MmpApp* app, const char* path) {
    for (GList* l = app->library; l != NULL; l = l->next) {
        Song* s = l->data;
        if (g_strcmp0(s->path, path) == 0) return s;
    }
    return NULL;
}

void navigation_row_selected_cb(GtkListBox* list_box, GtkListBoxRow* row, gpointer user_data) {
    (void)list_box;
    LibraryNavRows* rows = user_data;

    if (row == NULL) {
        return;
    }

    const char* page_name = g_object_get_data(G_OBJECT(row), "stack-page");
    if (page_name != NULL) {
        GList* base_list = mmp_app->library;
        bool owned = false;
        bool reverse = false;

        ui_clear_filters(mmp_app);

        if (g_object_get_data(G_OBJECT(row), "is-playlist-row")) {
            Playlist* p = g_object_get_data(G_OBJECT(row), "playlist");
            if (p) {
                mmp_app->current_playlist_id = p->id;
                GList* playlist_songs = db_get_playlist_songs(mmp_app->db, p->id);
                GList* projected = NULL;
                for (GList* l = playlist_songs; l != NULL; l = l->next) {
                    Song* ps = l->data;
                    Song* ls = find_song_in_library(mmp_app, ps->path);
                    if (ls) projected = g_list_append(projected, ls);
                }
                g_list_free_full(playlist_songs, (GDestroyNotify)free_song);
                base_list = projected;
                owned = true;
            }
        } else if (g_strcmp0(page_name, "songs-view") == 0) {
            const char* view_mode = g_object_get_data(G_OBJECT(row), "view-mode");
            mmp_app->current_playlist_id = 0;
            if (g_strcmp0(view_mode, "recently-added") == 0) {
                reverse = true;
            }
        }

        ui_set_view(mmp_app, base_list, owned, reverse);

        const char* search_text = gtk_editable_get_text(GTK_EDITABLE(mmp_app->songs_search_entry));
        if (search_text && strlen(search_text) > 0) {
            ui_add_filter(mmp_app, search_filter_func, (gpointer)search_text, NULL);
        }

        ui_refresh_view(mmp_app);
        gtk_stack_set_visible_child_name(rows->stack, page_name);
        
        if (mmp_app) {
            g_free(mmp_app->selected_artist_filter);
            mmp_app->selected_artist_filter = NULL;
            g_free(mmp_app->selected_album_filter);
            mmp_app->selected_album_filter = NULL;
            
            if (mmp_app->albums_list) gtk_list_box_invalidate_filter(mmp_app->albums_list);
        }
    }
}

gboolean on_drop_cb(GtkDropTarget* target, const GValue* value, double x, double y, gpointer user_data) {
    (void)target;
    (void)x;
    (void)y;
    MmpApp* app = user_data;

    if (G_VALUE_HOLDS(value, G_TYPE_FILE)) {
        GFile* file = g_value_get_object(value);
        char* path = g_file_get_path(file);
        playback_add_to_playlist(app, path, app->current_track_node == NULL);
        g_free(path);
        return TRUE;
    }
    return FALSE;
}
