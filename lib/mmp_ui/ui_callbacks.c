#include "mmp_ui_internal.h"
#include <stdbool.h>
#include <string.h>

static guint drag_source_pos = G_MAXUINT;

typedef struct {
    Song          *song;
    MmpUI         *ui;
} SongActionData;

typedef struct {
    guint          position;
    MmpUI         *ui;
} QueueActionData;

typedef struct {
    GtkStack *stack;
} LibraryNavRows;

static MmpUI* get_ui_from_widget(GtkWidget *widget) {
    GtkRoot *root = gtk_widget_get_root(widget);
    if (!root) return NULL;
    return g_object_get_data(G_OBJECT(root), "mmp-ui");
}

void queue_drag_begin_cb(GtkDragSource *source, GdkDrag *drag, gpointer user_data) {
    (void)drag; (void)user_data;
    GtkWidget *row = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(source));
    drag_source_pos = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(row), "queue-position")) - 1;
}

gboolean queue_drop_cb(GtkDropTarget *target, const GValue *value, double x, double y, gpointer user_data) {
    (void)value; (void)x; (void)y; (void)user_data;
    GtkWidget *row = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(target));
    MmpUI *ui = get_ui_from_widget(row);
    guint target_pos = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(row), "queue-position")) - 1;

    if (drag_source_pos != G_MAXUINT && drag_source_pos != target_pos) {
        mmp_library_reorder_queue(ui->library, drag_source_pos, target_pos);
        drag_source_pos = G_MAXUINT;
        return TRUE;
    }
    return FALSE;
}

static void ui_show_playlist_contents(MmpUI *ui, Playlist *p) {
    ui->current_playlist_id = p->id;
    GList *songs = mmp_library_get_playlist_songs(ui->library, p->id);

    ui_set_view(ui, songs, true, false);
    ui_clear_filters(ui);

    ui_update_search_lowered_text(ui, ui->songs_search_entry);
    ui_add_filter(ui, search_filter_func, ui, NULL);

    ui_refresh_view(ui);
}

static void song_properties_cb(GtkWidget *widget, gpointer user_data) {
    (void)widget;
    SongActionData *data = user_data;
    Song *song = data->song;
    GtkWindow *parent = data->ui->window;

    char *message = g_strdup_printf(
        "Artist: %s\nAlbum: %s\nPath: %s",
        song->artist, song->album, song->path
    );

    GtkAlertDialog *dialog = gtk_alert_dialog_new("%s", song->title);
    gtk_alert_dialog_set_detail(dialog, message);
    gtk_alert_dialog_show(dialog, parent);
    g_object_unref(dialog);

    g_free(message);
}

static void song_play_now_action_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action; (void)parameter;
    SongActionData *data = user_data;
    Song *song = data->song;
    MmpUI *ui = data->ui;
    if (ui->current_playlist_id > 0) {
        mmp_library_load_playlist(ui->library, ui->current_playlist_id, song->path);
    } else {
        mmp_library_play_from_library(ui->library, song->path);
    }
}

static void song_play_next_action_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action; (void)parameter;
    SongActionData *data = user_data;
    Song *song = data->song;
    mmp_library_play_next(data->ui->library, song->path);
}

static void song_add_to_queue_action_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action; (void)parameter;
    SongActionData *data = user_data;
    Song *song = data->song;
    mmp_library_add_to_queue(data->ui->library, song->path, false);
}

static void song_add_all_to_queue_action_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action; (void)parameter;
    SongActionData *data = user_data;
    GList *songs = ui_get_filtered_songs(data->ui);
    if (songs) {
        mmp_library_add_songs_to_queue(data->ui->library, songs);
        g_list_free(songs);
    }
}

static void song_properties_action_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action; (void)parameter;
    SongActionData *data = user_data;
    song_properties_cb(NULL, data);
}

static void song_add_to_playlist_action_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action;
    SongActionData *data = user_data;
    Song *song = data->song;
    int playlist_id = g_variant_get_int32(parameter);

    if (mmp_library_add_song_to_playlist(data->ui->library, playlist_id, song)) {
        if (data->ui->current_playlist_id == playlist_id) {
            GList *playlists = mmp_library_get_playlists(data->ui->library);
            for (GList *l = playlists; l != NULL; l = l->next) {
                Playlist *p = l->data;
                if (p->id == playlist_id) {
                    ui_show_playlist_contents(data->ui, p);
                    break;
                }
            }
            g_list_free_full(playlists, (GDestroyNotify)free_playlist);
        }
    }
}

static void song_remove_from_playlist_action_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action; (void)parameter;
    SongActionData *data = user_data;
    Song *song = data->song;
    if (mmp_library_remove_song_from_playlist(data->ui->library, data->ui->current_playlist_id, song->path)) {
        GList *playlists = mmp_library_get_playlists(data->ui->library);
        for (GList *l = playlists; l != NULL; l = l->next) {
            Playlist *p = l->data;
            if (p->id == data->ui->current_playlist_id) {
                ui_show_playlist_contents(data->ui, p);
                break;
            }
        }
        g_list_free_full(playlists, (GDestroyNotify)free_playlist);
    }
}

static void show_song_context_menu(MmpUI *ui, Song *song, double x, double y, GtkWidget *parent_row) {
    bool in_playlist_view = (ui->current_playlist_id > 0);

    SongActionData *action_data = g_new0(SongActionData, 1);
    action_data->song = song;
    action_data->ui = ui;

    GSimpleActionGroup *action_group = g_simple_action_group_new();

    const GActionEntry actions[] = {
        { "play_now",             song_play_now_action_cb,             NULL, NULL, NULL, {0, 0, 0} },
        { "play_next",            song_play_next_action_cb,            NULL, NULL, NULL, {0, 0, 0} },
        { "add_queue",            song_add_to_queue_action_cb,         NULL, NULL, NULL, {0, 0, 0} },
        { "add_all_queue",        song_add_all_to_queue_action_cb,     NULL, NULL, NULL, {0, 0, 0} },
        { "properties",           song_properties_action_cb,           NULL, NULL, NULL, {0, 0, 0} },
        { "add_to_playlist",      song_add_to_playlist_action_cb,      "i", NULL, NULL, {0, 0, 0} },
        { "remove_from_playlist", song_remove_from_playlist_action_cb, NULL, NULL, NULL, {0, 0, 0} }
    };
    g_action_map_add_action_entries(G_ACTION_MAP(action_group), actions, G_N_ELEMENTS(actions), action_data);
    gtk_widget_insert_action_group(parent_row, "song", G_ACTION_GROUP(action_group));

    GMenu *menu = g_menu_new();
    g_menu_append(menu, "Play Now", "song.play_now");
    g_menu_append(menu, "Play Next", "song.play_next");
    g_menu_append(menu, "Add to Queue", "song.add_queue");
    g_menu_append(menu, "Add all to Queue", "song.add_all_queue");
    g_menu_append(menu, "Properties", "song.properties");

    if (in_playlist_view) {
        g_menu_append(menu, "Remove from Playlist", "song.remove_from_playlist");
    } else {
        GList *playlists = mmp_library_get_playlists(ui->library);
        if (playlists) {
            GMenu *playlist_menu = g_menu_new();
            for (GList *l = playlists; l != NULL; l = l->next) {
                Playlist *p = l->data;
                GMenuItem *item = g_menu_item_new(p->name, NULL);
                g_menu_item_set_action_and_target(item, "song.add_to_playlist", "i", p->id);
                g_menu_append_item(playlist_menu, item);
                g_object_unref(item);
            }
            g_menu_append_submenu(menu, "Add to Playlist", G_MENU_MODEL(playlist_menu));
            g_object_unref(playlist_menu);
            g_list_free_full(playlists, (GDestroyNotify)free_playlist);
        }
    }

    GtkWidget *popover = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
    gtk_widget_set_parent(popover, parent_row);

    GdkRectangle rect = {(int)x, (int)y, 1, 1};
    gtk_popover_set_pointing_to(GTK_POPOVER(popover), &rect);
    gtk_popover_popup(GTK_POPOVER(popover));

    g_object_set_data_full(G_OBJECT(popover), "song-action-data", action_data, g_free);

    g_object_unref(menu);
    g_object_unref(action_group);
}

void song_row_secondary_click_cb(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {
    (void)user_data;
    if (n_press != 1) return;
    GtkWidget *widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
    Song *song = g_object_get_data(G_OBJECT(widget), "song-data");
    if (song) {
        MmpUI *ui = get_ui_from_widget(widget);
        if (ui) show_song_context_menu(ui, song, x, y, widget);
    }
}

static void queue_play_now_action_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action; (void)parameter;
    QueueActionData *data = user_data;
    const char *path = mmp_library_get_queue_path_at(data->ui->library, data->position);
    if (path)
        mmp_library_play_from_library(data->ui->library, path);
}

static void queue_remove_action_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action; (void)parameter;
    QueueActionData *data = user_data;
    mmp_library_remove_from_queue(data->ui->library, data->position);
}

static void queue_clear_action_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action; (void)parameter;
    QueueActionData *data = user_data;
    mmp_library_clear_queue(data->ui->library);
}

static void queue_save_as_playlist_action_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);

static void show_queue_context_menu(MmpUI *ui, guint position, double x, double y, GtkWidget *parent_row) {
    QueueActionData *action_data = g_new0(QueueActionData, 1);
    action_data->position = position;
    action_data->ui = ui;

    GSimpleActionGroup *action_group = g_simple_action_group_new();
    const GActionEntry actions[] = {
        { "play_now",      queue_play_now_action_cb,      NULL, NULL, NULL, {0, 0, 0} },
        { "remove",        queue_remove_action_cb,        NULL, NULL, NULL, {0, 0, 0} },
        { "save_playlist", queue_save_as_playlist_action_cb, NULL, NULL, NULL, {0, 0, 0} },
        { "clear",         queue_clear_action_cb,         NULL, NULL, NULL, {0, 0, 0} }
    };
    g_action_map_add_action_entries(G_ACTION_MAP(action_group), actions, G_N_ELEMENTS(actions), action_data);
    gtk_widget_insert_action_group(parent_row, "queue", G_ACTION_GROUP(action_group));

    GMenu *menu = g_menu_new();
    g_menu_append(menu, "Play Now", "queue.play_now");
    g_menu_append(menu, "Remove from Queue", "queue.remove");

    GMenu *section = g_menu_new();
    g_menu_append(section, "Save Queue as Playlist", "queue.save_playlist");
    g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
    g_object_unref(section);

    section = g_menu_new();
    g_menu_append(section, "Clear Queue", "queue.clear");
    g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
    g_object_unref(section);

    GtkWidget *popover = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
    gtk_widget_set_parent(popover, parent_row);

    GdkRectangle rect = {(int)x, (int)y, 1, 1};
    gtk_popover_set_pointing_to(GTK_POPOVER(popover), &rect);
    gtk_popover_popup(GTK_POPOVER(popover));

    g_object_set_data_full(G_OBJECT(popover), "queue-action-data", action_data, g_free);

    g_object_unref(menu);
    g_object_unref(action_group);
}

void queue_row_secondary_click_cb(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {
    (void)user_data;
    if (n_press != 1) return;
    GtkWidget *row = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
    gpointer pos_data = g_object_get_data(G_OBJECT(row), "queue-position");
    if (pos_data != NULL) {
        MmpUI *ui = get_ui_from_widget(row);
        if (ui) show_queue_context_menu(ui, GPOINTER_TO_UINT(pos_data) - 1, x, y, row);
    }
}

static void play_song(MmpUI *ui, Song *song) {
    if (ui->current_playlist_id > 0) {
        mmp_library_load_playlist(ui->library, ui->current_playlist_id, song->path);
    } else {
        mmp_library_play_from_library(ui->library, song->path);
    }
}

gboolean filter_albums_cb(GtkListBoxRow *row, gpointer user_data) {
    MmpUI *ui = user_data;
    if (!ui->selected_artist_filter) return TRUE;

    const char *album_artist = g_object_get_data(G_OBJECT(row), "album-artist");
    if (album_artist && g_strcmp0(album_artist, ui->selected_artist_filter) == 0) {
        return TRUE;
    }
    return FALSE;
}

void search_changed_cb(GtkSearchEntry *entry, gpointer user_data) {
    MmpUI *ui = user_data;
    ui_update_search_lowered_text(ui, entry);
    ui_refresh_view(ui);
}

void artist_row_activated_cb(GtkListBox *list, GtkListBoxRow *row, gpointer user_data) {
    (void)list;
    MmpUI *ui = user_data;
    GtkWidget *label = gtk_list_box_row_get_child(row);
    if (GTK_IS_BOX(label)) label = gtk_widget_get_first_child(label);
    const char *artist = gtk_label_get_text(GTK_LABEL(label));

    g_free(ui->selected_artist_filter);
    ui->selected_artist_filter = g_strdup(artist);

    g_free(ui->selected_album_filter);
    ui->selected_album_filter = NULL;

    if (ui->albums_list) gtk_list_box_invalidate_filter(ui->albums_list);

    if (ui->content_stack) {
        gtk_stack_set_visible_child_name(ui->content_stack, "albums");
    }
}

void album_row_activated_cb(GtkListBox *list, GtkListBoxRow *row, gpointer user_data) {
    (void)list;
    MmpUI *ui = user_data;
    GtkWidget *label = gtk_list_box_row_get_child(row);
    if (GTK_IS_BOX(label)) label = gtk_widget_get_first_child(label);
    const char *album = gtk_label_get_text(GTK_LABEL(label));

    g_free(ui->selected_album_filter);
    ui->selected_album_filter = g_strdup(album);

    ui_set_view(ui, mmp_library_get_all_songs(ui->library), false, false);
    ui_clear_filters(ui);

    if (ui->selected_artist_filter) {
        ui_add_filter(ui, artist_filter_func, g_strdup(ui->selected_artist_filter), g_free);
    }
    ui_add_filter(ui, album_filter_func, g_strdup(ui->selected_album_filter), g_free);

    ui_update_search_lowered_text(ui, ui->songs_search_entry);
    ui_add_filter(ui, search_filter_func, ui, NULL);

    ui_refresh_view(ui);

    if (ui->content_stack) {
        gtk_stack_set_visible_child_name(ui->content_stack, "songs-view");
    }
}

void song_view_activate_cb(GtkListView *view, guint position, gpointer user_data) {
    MmpUI *ui = user_data;
    GtkSelectionModel *sel = gtk_list_view_get_model(view);
    GObject *obj = g_list_model_get_item(G_LIST_MODEL(sel), position);
    if (obj) {
        MmpSongItem *item = MMP_SONG_ITEM(obj);
        if (item->song) {
            play_song(ui, item->song);
        }
        g_object_unref(obj);
    }
}

void queue_view_activate_cb(GtkListView *view, guint position, gpointer user_data) {
    MmpUI *ui = user_data;
    const char *path = mmp_library_get_queue_path_at(ui->library, position);
    if (path)
        mmp_library_play_from_library(ui->library, path);
}

void volume_controls_enter_cb(
    GtkEventControllerMotion *controller,
    double x,
    double y,
    gpointer user_data
) {
    (void)controller;
    (void)x;
    (void)y;

    gtk_revealer_set_reveal_child(GTK_REVEALER(user_data), TRUE);
}

void volume_controls_leave_cb(GtkEventControllerMotion *controller, gpointer user_data) {
    (void)controller;

    gtk_revealer_set_reveal_child(GTK_REVEALER(user_data), FALSE);
}

void mute_button_clicked_cb(GtkButton *button, gpointer user_data) {
    MmpUI *ui = (MmpUI *)user_data;
    ui->volume_muted = !ui->volume_muted;

    gtk_button_set_icon_name(button,
        ui->volume_muted ? "audio-volume-muted-symbolic" : "audio-volume-medium-symbolic");
    gtk_widget_set_tooltip_text(GTK_WIDGET(button), ui->volume_muted ? "Unmute" : "Mute");

    mmp_playback_set_mute(ui->playback, ui->volume_muted);

    GtkWidget *volume_scale = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "volume-scale"));
    if (volume_scale)
        gtk_widget_set_sensitive(volume_scale, !ui->volume_muted);
}

void play_pause_clicked_cb(GtkButton *button, gpointer user_data) {
    (void)button;
    MmpUI *ui = user_data;
    mmp_playback_toggle_pause(ui->playback);
}

void volume_scale_changed_cb(GtkRange *range, gpointer user_data) {
    MmpUI *ui = user_data;
    double volume = gtk_range_get_value(range) / 100.0;
    mmp_playback_set_volume(ui->playback, volume);
}

void track_progress_scale_value_changed_cb(GtkRange *range, gpointer user_data) {
    MmpUI *ui = user_data;
    if (ui->is_programmatic_change) return;

    static gint64 last_seek_time = 0;
    gint64 now = g_get_monotonic_time();

    if (now - last_seek_time < 100000) return;

    double value = gtk_range_get_value(range);
    mmp_playback_seek(ui->playback, value);

    last_seek_time = now;
}

void shuffle_clicked_cb(GtkButton *button, gpointer user_data) {
    (void)button;
    MmpUI *ui = user_data;
    mmp_library_toggle_shuffle(ui->library);

    if (mmp_library_get_shuffle(ui->library)) {
        gtk_widget_add_css_class(GTK_WIDGET(ui->shuffle_button), "active-control");
    } else {
        gtk_widget_remove_css_class(GTK_WIDGET(ui->shuffle_button), "active-control");
    }
}

void repeat_clicked_cb(GtkButton *button, gpointer user_data) {
    (void)button;
    MmpUI *ui = user_data;
    mmp_library_toggle_repeat(ui->library);

    switch (mmp_library_get_repeat(ui->library)) {
        case REPEAT_OFF:
            gtk_button_set_icon_name(ui->repeat_button, "media-playlist-repeat-symbolic");
            gtk_widget_remove_css_class(GTK_WIDGET(ui->repeat_button), "active-control");
            break;
        case REPEAT_ALL:
            gtk_button_set_icon_name(ui->repeat_button, "media-playlist-repeat-symbolic");
            gtk_widget_add_css_class(GTK_WIDGET(ui->repeat_button), "active-control");
            break;
        case REPEAT_ONE:
            gtk_button_set_icon_name(ui->repeat_button, "media-playlist-repeat-song-symbolic");
            gtk_widget_add_css_class(GTK_WIDGET(ui->repeat_button), "active-control");
            break;
    }
}

void previous_track_clicked_cb(GtkButton *button, gpointer user_data) {
    (void)button;
    MmpUI *ui = user_data;

    double pos = mmp_playback_get_position(ui->playback);
    if (pos > 3.0) {
        mmp_playback_seek(ui->playback, 0.0);
        return;
    }

    mmp_library_skip_prev(ui->library);
}

void next_track_clicked_cb(GtkButton *button, gpointer user_data) {
    (void)button;
    MmpUI *ui = user_data;
    mmp_library_skip_next(ui->library);
}

typedef void (*EntryCallback)(const char *text, gpointer user_data);

typedef struct {
    GtkWidget     *dialog;
    GtkWidget     *entry;
    EntryCallback  callback;
    gpointer       user_data;
} DialogData;

static void on_dialog_ok_clicked(GtkButton *btn, gpointer d) {
    (void)btn;
    DialogData *dd = (DialogData *)d;
    dd->callback(gtk_editable_get_text(GTK_EDITABLE(dd->entry)), dd->user_data);
    gtk_window_destroy(GTK_WINDOW(dd->dialog));
    g_free(dd);
}

static void show_entry_dialog(GtkWindow *parent, const char *title, const char *initial_text, EntryCallback callback, gpointer user_data) {
    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, -1);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(box, "content-page");
    gtk_window_set_child(GTK_WINDOW(dialog), box);

    GtkWidget *entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entry), initial_text ? initial_text : "");
    gtk_box_append(GTK_BOX(box), entry);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(box), button_box);

    GtkWidget *cancel_button = gtk_button_new_with_label("Cancel");
    g_signal_connect_swapped(cancel_button, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_box_append(GTK_BOX(button_box), cancel_button);

    GtkWidget *ok_button = gtk_button_new_with_label("OK");
    gtk_widget_add_css_class(ok_button, "suggested-action");
    gtk_box_append(GTK_BOX(button_box), ok_button);

    DialogData *data = g_new0(DialogData, 1);
    data->dialog = dialog;
    data->entry = entry;
    data->callback = callback;
    data->user_data = user_data;

    g_signal_connect(ok_button, "clicked", G_CALLBACK(on_dialog_ok_clicked), data);
    g_signal_connect(entry, "activate", G_CALLBACK(on_dialog_ok_clicked), data);

    gtk_window_present(GTK_WINDOW(dialog));
}

static void on_rename_playlist_done(const char *name, gpointer user_data) {
    if (!name || strlen(name) == 0) return;
    GtkListBoxRow *row = user_data;
    MmpUI *ui = get_ui_from_widget(GTK_WIDGET(row));
    if (!ui) return;
    Playlist *p = g_object_get_data(G_OBJECT(row), "playlist");
    if (p && mmp_library_rename_playlist(ui->library, p->id, name)) {
        ui_update_playlists(ui);
    }
}

static void on_create_playlist_done(const char *name, gpointer user_data) {
    if (!name || strlen(name) == 0) return;
    MmpUI *ui = user_data;
    if (mmp_library_create_playlist(ui->library, name, NULL)) {
        ui_update_playlists(ui);
    }
}

static void rename_action_activated_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action; (void)parameter;
    GtkListBoxRow *row = user_data;
    Playlist *p = g_object_get_data(G_OBJECT(row), "playlist");
    if (p) {
        MmpUI *ui = get_ui_from_widget(GTK_WIDGET(row));
        if (ui) {
            show_entry_dialog(ui->window, "Rename Playlist", p->name, on_rename_playlist_done, row);
        }
    }
}

static void remove_action_activated_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action; (void)parameter;
    GtkListBoxRow *row = user_data;
    MmpUI *ui = get_ui_from_widget(GTK_WIDGET(row));
    if (!ui) return;
    Playlist *p = g_object_get_data(G_OBJECT(row), "playlist");
    if (p && mmp_library_delete_playlist(ui->library, p->id)) {
        ui_update_playlists(ui);
    }
}

static void show_playlist_context_menu(GtkListBoxRow *row, double x, double y) {
    MmpUI *ui = get_ui_from_widget(GTK_WIDGET(row));
    if (!ui) return;

    GSimpleActionGroup *action_group = g_simple_action_group_new();
    const GActionEntry actions[] = {
        { "rename", rename_action_activated_cb, NULL, NULL, NULL, {0, 0, 0} },
        { "remove", remove_action_activated_cb, NULL, NULL, NULL, {0, 0, 0} }
    };
    g_action_map_add_action_entries(G_ACTION_MAP(action_group), actions, G_N_ELEMENTS(actions), row);
    gtk_widget_insert_action_group(GTK_WIDGET(row), "playlist", G_ACTION_GROUP(action_group));

    GMenu *menu = g_menu_new();
    g_menu_append(menu, "Rename", "playlist.rename");
    g_menu_append(menu, "Remove", "playlist.remove");

    GtkWidget *popover = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
    gtk_widget_set_parent(popover, GTK_WIDGET(row));

    GdkRectangle rect = {(int)x, (int)y, 1, 1};
    gtk_popover_set_pointing_to(GTK_POPOVER(popover), &rect);
    gtk_popover_popup(GTK_POPOVER(popover));

    g_object_unref(menu);
    g_object_unref(action_group);
}

void playlist_row_right_clicked_cb(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {
    (void)n_press;
    GtkListBoxRow *row = user_data;
    show_playlist_context_menu(row, x, y);
}

void playlist_row_double_clicked_cb(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {
    (void)x; (void)y;
    if (n_press != 2) return;

    GtkListBoxRow *row = user_data;
    Playlist *p = g_object_get_data(G_OBJECT(row), "playlist");
    if (!p) return;

    MmpUI *ui = get_ui_from_widget(GTK_WIDGET(row));
    if (!ui) return;

    mmp_library_load_playlist(ui->library, p->id, NULL);
}

static void create_playlist_clicked_cb(GtkButton *button, gpointer user_data) {
    (void)button;
    MmpUI *ui = user_data;
    show_entry_dialog(ui->window, "New Playlist", "New Playlist", on_create_playlist_done, ui);
}

void create_playlist_action_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action; (void)parameter;
    create_playlist_clicked_cb(NULL, user_data);
}

static void on_save_queue_as_playlist_done(const char *name, gpointer user_data) {
    if (!name || strlen(name) == 0) return;
    MmpUI *ui = user_data;

    int new_id;
    if (!mmp_library_create_playlist(ui->library, name, &new_id)) return;

    GList *paths = mmp_library_get_queue_path_list(ui->library);
    for (GList *l = paths; l != NULL; l = l->next) {
        Song *song = mmp_library_find_song(ui->library, (const char *)l->data);
        if (song)
            mmp_library_add_song_to_playlist(ui->library, new_id, song);
    }
    g_list_free(paths);

    ui_update_playlists(ui);
}

static void queue_save_as_playlist_action_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action; (void)parameter;
    QueueActionData *data = user_data;
    show_entry_dialog(data->ui->window, "Save Queue as Playlist", "New Playlist", on_save_queue_as_playlist_done, data->ui);
}

void playlists_header_right_clicked_cb(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {
    (void)n_press;
    MmpUI *ui = user_data;
    GtkWidget *header_row = GTK_WIDGET(g_object_get_data(G_OBJECT(ui->window), "nav-playlists-row"));

    GMenu *menu = g_menu_new();
    g_menu_append(menu, "Create Playlist", "app.create-playlist");

    GtkWidget *popover = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
    gtk_widget_set_parent(popover, header_row);

    GdkRectangle rect = {(int)x, (int)y, 1, 1};
    gtk_popover_set_pointing_to(GTK_POPOVER(popover), &rect);
    gtk_popover_popup(GTK_POPOVER(popover));

    g_object_unref(menu);
}

void navigation_row_selected_cb(GtkListBox *list_box, GtkListBoxRow *row, gpointer user_data) {
    (void)list_box;
    LibraryNavRows *rows = user_data;

    if (row == NULL) {
        return;
    }

    MmpUI *ui = get_ui_from_widget(GTK_WIDGET(rows->stack));
    if (!ui) return;

    const char *page_name = g_object_get_data(G_OBJECT(row), "stack-page");
    if (page_name != NULL) {
        GList *base_list = mmp_library_get_all_songs(ui->library);
        bool owned = false;
        bool reverse = false;

        ui_clear_filters(ui);

        if (g_object_get_data(G_OBJECT(row), "is-playlist-row")) {
            Playlist *p = g_object_get_data(G_OBJECT(row), "playlist");
            if (p) {
                ui->current_playlist_id = p->id;
                GList *playlist_songs = mmp_library_get_playlist_songs(ui->library, p->id);
                GList *projected = NULL;
                for (GList *l = playlist_songs; l != NULL; l = l->next) {
                    Song *ps = l->data;
                    Song *ls = mmp_library_find_song(ui->library, ps->path);
                    if (ls) projected = g_list_prepend(projected, ls);
                }
                projected = g_list_reverse(projected);
                g_list_free_full(playlist_songs, (GDestroyNotify)free_song);
                base_list = projected;
                owned = true;
            }
        } else if (g_strcmp0(page_name, "songs-view") == 0) {
            const char *view_mode = g_object_get_data(G_OBJECT(row), "view-mode");
            ui->current_playlist_id = 0;
            if (g_strcmp0(view_mode, "recently-added") == 0) {
                reverse = true;
            }
        }

        ui_set_view(ui, base_list, owned, reverse);

        ui_update_search_lowered_text(ui, ui->songs_search_entry);
        ui_add_filter(ui, search_filter_func, ui, NULL);

        ui_refresh_view(ui);
        gtk_stack_set_visible_child_name(rows->stack, page_name);

        g_free(ui->selected_artist_filter);
        ui->selected_artist_filter = NULL;
        g_free(ui->selected_album_filter);
        ui->selected_album_filter = NULL;

        if (ui->albums_list) gtk_list_box_invalidate_filter(ui->albums_list);
    }
}

gboolean on_drop_cb(GtkDropTarget *target, const GValue *value, double x, double y, gpointer user_data) {
    (void)target;
    (void)x;
    (void)y;
    MmpUI *ui = user_data;

    if (G_VALUE_HOLDS(value, G_TYPE_FILE)) {
        GFile *file = g_value_get_object(value);
        char *path = g_file_get_path(file);
        bool play_now = (mmp_library_get_queue_length(ui->library) == 0);
        mmp_library_add_to_queue(ui->library, path, play_now);
        g_free(path);
        return TRUE;
    }
    return FALSE;
}
