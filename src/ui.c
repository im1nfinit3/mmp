#include "ui.h"
#include "playback.h"

#include <stdbool.h>

typedef struct {
    GtkStack* stack;
    GtkWidget* library_header_row;
    GtkWidget* recently_added_row;
    GtkWidget* albums_row;
    GtkWidget* artists_row;
    GtkWidget* songs_row;
} LibraryNavRows;

static void navigation_row_selected_cb(GtkListBox* list_box, GtkListBoxRow* row, gpointer user_data);

static MmpApp* mmp_app = NULL;

static GList* drag_source_node = NULL;

static void queue_drag_begin_cb(GtkDragSource* source, GdkDrag* drag, gpointer user_data) {
    GtkWidget* row = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(source));
    drag_source_node = g_object_get_data(G_OBJECT(row), "playlist-node");
}

static gboolean queue_drop_cb(GtkDropTarget* target, const GValue* value, double x, double y, gpointer user_data) {
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

static void free_song(Song* song) {
    g_free(song->path);
    g_free(song->title);
    g_free(song->artist);
    g_free(song->album);
    g_free(song);
}

static void add_to_library_ui(MmpApp* app, Song* song) {
    // Add to Artists list if not already there
    bool artist_exists = false;
    GtkWidget* artist_child = gtk_widget_get_first_child(GTK_WIDGET(app->artists_list));
    while (artist_child) {
        GtkWidget* label = gtk_list_box_row_get_child(GTK_LIST_BOX_ROW(artist_child));
        if (GTK_IS_BOX(label)) label = gtk_widget_get_first_child(label);
        if (g_strcmp0(gtk_label_get_label(GTK_LABEL(label)), song->artist) == 0) {
            artist_exists = true;
            break;
        }
        artist_child = gtk_widget_get_next_sibling(artist_child);
    }

    if (!artist_exists) {
        GtkWidget* row = gtk_list_box_row_new();
        GtkWidget* label = gtk_label_new(song->artist);
        gtk_label_set_xalign(GTK_LABEL(label), 0);
        gtk_widget_set_margin_start(label, 12);
        gtk_widget_set_margin_top(label, 8);
        gtk_widget_set_margin_bottom(label, 8);
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), label);
        gtk_list_box_append(app->artists_list, row);
    }

    // Add to Albums list if not already there
    bool album_exists = false;
    GtkWidget* album_child = gtk_widget_get_first_child(GTK_WIDGET(app->albums_list));
    while (album_child) {
        GtkWidget* label = gtk_list_box_row_get_child(GTK_LIST_BOX_ROW(album_child));
        if (GTK_IS_BOX(label)) label = gtk_widget_get_first_child(label);
        if (g_strcmp0(gtk_label_get_label(GTK_LABEL(label)), song->album) == 0) {
            album_exists = true;
            break;
        }
        album_child = gtk_widget_get_next_sibling(album_child);
    }

    if (!album_exists) {
        GtkWidget* row = gtk_list_box_row_new();
        GtkWidget* label = gtk_label_new(song->album);
        gtk_label_set_xalign(GTK_LABEL(label), 0);
        gtk_widget_set_margin_start(label, 12);
        gtk_widget_set_margin_top(label, 8);
        gtk_widget_set_margin_bottom(label, 8);
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), label);
        g_object_set_data_full(G_OBJECT(row), "album-artist", g_strdup(song->artist), g_free);
        gtk_list_box_append(app->albums_list, row);
    }
}

static void song_properties_cb(GtkWidget* widget, gpointer user_data) {
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

static void song_play_now_cb(GtkWidget* widget, gpointer user_data) {
    Song* song = user_data;
    playback_open_file(mmp_app, song->path);
}

static void song_add_to_queue_cb(GtkWidget* widget, gpointer user_data) {
    Song* song = user_data;
    playback_add_to_playlist(mmp_app, song->path, false);
}

static void song_play_next_cb(GtkWidget* widget, gpointer user_data) {
    Song* song = user_data;
    playback_play_next(mmp_app, song->path);
}

static void show_song_context_menu(Song* song, double x, double y, GtkWidget* parent_row) {
    GtkWidget* popover = gtk_popover_new();
    gtk_widget_set_parent(popover, parent_row);
    
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_popover_set_child(GTK_POPOVER(popover), box);
    
    struct { const char* label; GCallback callback; } actions[] = {
        {"Play Now", G_CALLBACK(song_play_now_cb)},
        {"Play Next", G_CALLBACK(song_play_next_cb)},
        {"Add to Queue", G_CALLBACK(song_add_to_queue_cb)},
        {"Properties", G_CALLBACK(song_properties_cb)},
    };
    
    for (int i = 0; i < 4; i++) {
        GtkWidget* button = gtk_button_new_with_label(actions[i].label);
        gtk_widget_add_css_class(button, "flat");
        gtk_widget_set_halign(button, GTK_ALIGN_START);
        g_signal_connect(button, "clicked", actions[i].callback, song);
        g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_popover_popdown), popover);
        gtk_box_append(GTK_BOX(box), button);
    }
    
    GdkRectangle rect = {(int)x, (int)y, 1, 1};
    gtk_popover_set_pointing_to(GTK_POPOVER(popover), &rect);
    gtk_popover_popup(GTK_POPOVER(popover));
}

static void song_row_secondary_click_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer user_data) {
    if (n_press != 1) return;
    GtkWidget* row = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
    Song* song = g_object_get_data(G_OBJECT(row), "song-data");
    if (song) {
        show_song_context_menu(song, x, y, row);
    }
}

static void queue_play_now_cb(GtkWidget* widget, gpointer user_data) {
    GList* node = user_data;
    playback_play_track(mmp_app, node);
}

static void queue_remove_cb(GtkWidget* widget, gpointer user_data) {
    GList* node = user_data;
    playback_remove_from_playlist(mmp_app, node);
}

static void queue_clear_cb(GtkWidget* widget, gpointer user_data) {
    playback_clear_playlist(mmp_app);
}

static void show_queue_context_menu(GList* node, double x, double y, GtkWidget* parent_row) {
    GtkWidget* popover = gtk_popover_new();
    gtk_widget_set_parent(popover, parent_row);
    
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_popover_set_child(GTK_POPOVER(popover), box);
    
    GtkWidget* play_btn = gtk_button_new_with_label("Play Now");
    gtk_widget_add_css_class(play_btn, "flat");
    gtk_widget_set_halign(play_btn, GTK_ALIGN_START);
    g_signal_connect(play_btn, "clicked", G_CALLBACK(queue_play_now_cb), node);
    g_signal_connect_swapped(play_btn, "clicked", G_CALLBACK(gtk_popover_popdown), popover);
    gtk_box_append(GTK_BOX(box), play_btn);
    
    GtkWidget* remove_btn = gtk_button_new_with_label("Remove from Queue");
    gtk_widget_add_css_class(remove_btn, "flat");
    gtk_widget_set_halign(remove_btn, GTK_ALIGN_START);
    g_signal_connect(remove_btn, "clicked", G_CALLBACK(queue_remove_cb), node);
    g_signal_connect_swapped(remove_btn, "clicked", G_CALLBACK(gtk_popover_popdown), popover);
    gtk_box_append(GTK_BOX(box), remove_btn);

    GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(box), separator);

    GtkWidget* clear_btn = gtk_button_new_with_label("Clear Queue");
    gtk_widget_add_css_class(clear_btn, "flat");
    gtk_widget_set_halign(clear_btn, GTK_ALIGN_START);
    g_signal_connect(clear_btn, "clicked", G_CALLBACK(queue_clear_cb), NULL);
    g_signal_connect_swapped(clear_btn, "clicked", G_CALLBACK(gtk_popover_popdown), popover);
    gtk_box_append(GTK_BOX(box), clear_btn);
    
    GdkRectangle rect = {(int)x, (int)y, 1, 1};
    gtk_popover_set_pointing_to(GTK_POPOVER(popover), &rect);
    gtk_popover_popup(GTK_POPOVER(popover));
}

static void queue_row_secondary_click_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer user_data) {
    if (n_press != 1) return;
    GtkWidget* row = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
    GList* node = g_object_get_data(G_OBJECT(row), "playlist-node");
    if (node) {
        show_queue_context_menu(node, x, y, row);
    }
}

static void play_song(MmpApp* app, Song* song) {
    // For now, let's just clear the playlist and play this song
    playback_open_file(app, song->path);
}

static gboolean filter_albums_cb(GtkListBoxRow* row, gpointer user_data) {
    MmpApp* app = user_data;
    if (!app->selected_artist_filter) return TRUE;

    const char* album_artist = g_object_get_data(G_OBJECT(row), "album-artist");
    if (album_artist && g_strcmp0(album_artist, app->selected_artist_filter) == 0) {
        return TRUE;
    }
    return FALSE;
}

static gboolean filter_songs_cb(GtkListBoxRow* row, gpointer user_data) {
    MmpApp* app = user_data;
    Song* song = g_object_get_data(G_OBJECT(row), "song-data");
    if (!song) return TRUE;

    if (app->selected_artist_filter && g_strcmp0(song->artist, app->selected_artist_filter) != 0) {
        return FALSE;
    }
    if (app->selected_album_filter && g_strcmp0(song->album, app->selected_album_filter) != 0) {
        return FALSE;
    }

    const char* search_text = gtk_editable_get_text(GTK_EDITABLE(app->songs_search_entry));
    if (search_text == NULL || strlen(search_text) == 0) return TRUE;

    char* search_lower = g_utf8_strdown(search_text, -1);
    char* title_lower = g_utf8_strdown(song->title, -1);
    
    gboolean visible = (strstr(title_lower, search_lower) != NULL);
    
    g_free(search_lower);
    g_free(title_lower);
    
    return visible;
}

static void search_changed_cb(GtkSearchEntry* entry, gpointer user_data) {
    (void)entry;
    MmpApp* app = user_data;
    gtk_list_box_invalidate_filter(app->songs_list);
}

static void select_nav_row_by_page_name(MmpApp* app, const char* page_name) {
    if (!app->navigation_list) return;
    
    GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(app->navigation_list));
    while (child) {
        if (GTK_IS_LIST_BOX_ROW(child)) {
            const char* row_page = g_object_get_data(G_OBJECT(child), "stack-page");
            if (g_strcmp0(row_page, page_name) == 0) {
                // Select row without emitting signal to avoid clearing filters
                g_signal_handlers_block_by_func(app->navigation_list, navigation_row_selected_cb, NULL);
                gtk_list_box_select_row(app->navigation_list, GTK_LIST_BOX_ROW(child));
                g_signal_handlers_unblock_by_func(app->navigation_list, navigation_row_selected_cb, NULL);
                break;
            }
        }
        child = gtk_widget_get_next_sibling(child);
    }
}

static void artist_row_activated_cb(GtkListBox* list, GtkListBoxRow* row, gpointer user_data) {
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
    if (app->songs_list) gtk_list_box_invalidate_filter(app->songs_list);
    
    if (app->content_stack) {
        gtk_stack_set_visible_child_name(app->content_stack, "albums");
    }
    // We would want to select nav_albums_row in navigation_list, but we might not have a direct pointer.
    // However, the visual indication might be fine without updating the sidebar for now, or we can find it.
    // It's probably better to find and select it without triggering the clear.
    // Actually, `navigation_row_selected_cb` is connected with user_data = `LibraryNavRows*`, which we can't unblock by func easily if it has user_data, but Gtk allows blocking by func.
    // Let's just switch stack for now.
}

static void album_row_activated_cb(GtkListBox* list, GtkListBoxRow* row, gpointer user_data) {
    (void)list;
    MmpApp* app = user_data;
    GtkWidget* label = gtk_list_box_row_get_child(row);
    if (GTK_IS_BOX(label)) label = gtk_widget_get_first_child(label);
    const char* album = gtk_label_get_text(GTK_LABEL(label));
    
    g_free(app->selected_album_filter);
    app->selected_album_filter = g_strdup(album);
    
    if (app->songs_list) gtk_list_box_invalidate_filter(app->songs_list);
    
    if (app->content_stack) {
        gtk_stack_set_visible_child_name(app->content_stack, "songs");
    }
}

static void song_row_activated_cb(GtkListBox* list, GtkListBoxRow* row, gpointer user_data) {
    (void)list;
    MmpApp* app = user_data;
    Song* song = g_object_get_data(G_OBJECT(row), "song-data");
    if (song) {
        play_song(app, song);
    }
}

void ui_update_queue(MmpApp* app) {
    if (!app->queue_list) return;

    // Clear the list
    GtkWidget* child;
    while ((child = gtk_widget_get_first_child(GTK_WIDGET(app->queue_list))) != NULL) {
        gtk_list_box_remove(app->queue_list, child);
    }

    // Populate from playlist
    GList* iter = app->playlist->head;
    while (iter) {
        const char* path = iter->data;
        GtkWidget* row = gtk_list_box_row_new();
        GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_set_margin_start(box, 12);
        gtk_widget_set_margin_end(box, 12);
        gtk_widget_set_margin_top(box, 8);
        gtk_widget_set_margin_bottom(box, 8);

        // Try to find the song in the library to get its title
        Song* found_song = NULL;
        for (GList* l = app->library; l != NULL; l = l->next) {
            Song* s = (Song*)l->data;
            if (g_strcmp0(s->path, path) == 0) {
                found_song = s;
                break;
            }
        }

        char* basename = NULL;
        const char* display_name = NULL;
        if (found_song && found_song->title) {
            display_name = found_song->title;
        } else {
            basename = g_path_get_basename(path);
            display_name = basename;
        }

        GtkWidget* label = gtk_label_new(display_name);
        g_free(basename);
        gtk_label_set_xalign(GTK_LABEL(label), 0);
        gtk_widget_set_hexpand(label, TRUE);
        
        gtk_box_append(GTK_BOX(box), label);
        
        if (iter == app->current_track_node) {
            GtkWidget* active_indicator = gtk_image_new_from_icon_name("audio-volume-medium-symbolic");
            gtk_box_append(GTK_BOX(box), active_indicator);
        }

        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);
        g_object_set_data(G_OBJECT(row), "playlist-node", iter);
        
        GtkGesture* gesture = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture), GDK_BUTTON_SECONDARY);
        g_signal_connect(gesture, "pressed", G_CALLBACK(queue_row_secondary_click_cb), NULL);
        gtk_widget_add_controller(row, GTK_EVENT_CONTROLLER(gesture));

        GtkDragSource* drag_source = gtk_drag_source_new();
        gtk_drag_source_set_actions(drag_source, GDK_ACTION_COPY | GDK_ACTION_MOVE);
        g_signal_connect(drag_source, "drag-begin", G_CALLBACK(queue_drag_begin_cb), NULL);
        // GTK4 needs content to start a drag
        GValue value = G_VALUE_INIT;
        g_value_init(&value, G_TYPE_STRING);
        g_value_set_static_string(&value, "reorder");
        GdkContentProvider* content = gdk_content_provider_new_for_value(&value);
        gtk_drag_source_set_content(drag_source, content);
        g_object_unref(content);
        g_value_unset(&value);
        gtk_widget_add_controller(row, GTK_EVENT_CONTROLLER(drag_source));

        GtkDropTarget* drop_target = gtk_drop_target_new(G_TYPE_STRING, GDK_ACTION_COPY | GDK_ACTION_MOVE);
        g_signal_connect(drop_target, "drop", G_CALLBACK(queue_drop_cb), NULL);
        gtk_widget_add_controller(row, GTK_EVENT_CONTROLLER(drop_target));

        gtk_list_box_append(app->queue_list, row);
        
        iter = iter->next;
    }
}

static void queue_row_activated_cb(GtkListBox* list, GtkListBoxRow* row, gpointer user_data) {
    (void)list;
    MmpApp* app = user_data;
    GList* node = g_object_get_data(G_OBJECT(row), "playlist-node");
    if (node) {
        playback_play_track(app, node);
        ui_update_queue(app);
    }
}

static void add_song_to_ui(MmpApp* app, Song* song) {
    GtkWidget* row = gtk_list_box_row_new();
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_start(box, 12);
    gtk_widget_set_margin_end(box, 12);
    gtk_widget_set_margin_top(box, 8);
    gtk_widget_set_margin_bottom(box, 8);

    GtkWidget* title_label = gtk_label_new(song->title);
    gtk_label_set_xalign(GTK_LABEL(title_label), 0);
    gtk_widget_set_hexpand(title_label, TRUE);
    
    gtk_box_append(GTK_BOX(box), title_label);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);
    
    g_object_set_data(G_OBJECT(row), "song-data", song);
    gtk_list_box_append(app->songs_list, row);

    GtkGesture* gesture = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture), GDK_BUTTON_SECONDARY);
    g_signal_connect(gesture, "pressed", G_CALLBACK(song_row_secondary_click_cb), NULL);
    gtk_widget_add_controller(row, GTK_EVENT_CONTROLLER(gesture));

    // Also add to recently added
    GtkWidget* recent_row = gtk_list_box_row_new();
    GtkWidget* recent_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_start(recent_box, 12);
    gtk_widget_set_margin_end(recent_box, 12);
    gtk_widget_set_margin_top(recent_box, 8);
    gtk_widget_set_margin_bottom(recent_box, 8);
    GtkWidget* recent_label = gtk_label_new(song->title);
    gtk_label_set_xalign(GTK_LABEL(recent_label), 0);
    gtk_box_append(GTK_BOX(recent_box), recent_label);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(recent_row), recent_box);
    g_object_set_data(G_OBJECT(recent_row), "song-data", song);
    gtk_list_box_prepend(app->recently_added_list, recent_row);

    GtkGesture* recent_gesture = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(recent_gesture), GDK_BUTTON_SECONDARY);
    g_signal_connect(recent_gesture, "pressed", G_CALLBACK(song_row_secondary_click_cb), NULL);
    gtk_widget_add_controller(recent_row, GTK_EVENT_CONTROLLER(recent_gesture));

    add_to_library_ui(app, song);
}

static void scan_directory(MmpApp* app, const char* path) {
    GFile* dir = g_file_new_for_path(path);
    GFileEnumerator* enumerator = g_file_enumerate_children(dir, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, NULL);

    if (enumerator) {
        GFileInfo* info;
        while ((info = g_file_enumerator_next_file(enumerator, NULL, NULL)) != NULL) {
            const char* name = g_file_info_get_name(info);
            GFile* child = g_file_get_child(dir, name);
            char* child_path = g_file_get_path(child);

            if (g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY) {
                scan_directory(app, child_path);
            } else if (g_str_has_suffix(name, ".mp3") || g_str_has_suffix(name, ".flac") || 
                       g_str_has_suffix(name, ".ogg") || g_str_has_suffix(name, ".wav") ||
                       g_str_has_suffix(name, ".m4a")) {
                Song* song = g_new0(Song, 1);
                song->path = g_strdup(child_path);
                
                // Set title to filename without extension as a better default track name
                char* title = g_strdup(name);
                char* dot = g_strrstr(title, ".");
                if (dot) *dot = '\0';
                song->title = title;
                
                // Try to guess Artist/Album from directory structure
                GFile* parent = g_file_get_parent(child);
                GFile* grand_parent = parent ? g_file_get_parent(parent) : NULL;
                
                if (parent) {
                    char* parent_name = g_file_get_basename(parent);
                    song->album = g_strdup(parent_name);
                    g_free(parent_name);
                    
                    if (grand_parent) {
                        char* grand_parent_name = g_file_get_basename(grand_parent);
                        song->artist = g_strdup(grand_parent_name);
                        g_free(grand_parent_name);
                    }
                }
                
                if (!song->album) song->album = g_strdup("Unknown Album");
                if (!song->artist) song->artist = g_strdup("Unknown Artist");

                app->library = g_list_append(app->library, song);
                add_song_to_ui(app, song);
                
                if (grand_parent) g_object_unref(grand_parent);
                if (parent) g_object_unref(parent);
            }

            g_free(child_path);
            g_object_unref(child);
            g_object_unref(info);
        }
        g_object_unref(enumerator);
    }
    g_object_unref(dir);
}

static void load_css(GtkWindow* window) {
    GtkCssProvider* provider = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(provider, "/xyz/_1nfinit3/mmp/ui/style.css");
    gtk_style_context_add_provider_for_display(
        gtk_widget_get_display(GTK_WIDGET(window)),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(provider);
}

static void volume_controls_enter_cb(
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

static void volume_controls_leave_cb(GtkEventControllerMotion* controller, gpointer user_data) {
    (void)controller;

    gtk_revealer_set_reveal_child(GTK_REVEALER(user_data), FALSE);
}

static void mute_button_clicked_cb(GtkButton* button, gpointer user_data) {
    (void)user_data;
    if (!mmp_app) return;

    mmp_app->volume_muted = !mmp_app->volume_muted;

    gtk_button_set_icon_name(
        button,
        mmp_app->volume_muted ? "audio-volume-muted-symbolic" : "audio-volume-medium-symbolic"
    );
    gtk_widget_set_tooltip_text(GTK_WIDGET(button), mmp_app->volume_muted ? "Unmute" : "Mute");
    
    playback_set_mute(mmp_app, mmp_app->volume_muted);
    
    // Find the scale to set its sensitivity
    GtkWidget* volume_scale = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "volume-scale"));
    if (volume_scale) {
        gtk_widget_set_sensitive(volume_scale, !mmp_app->volume_muted);
    }
}

static void play_pause_clicked_cb(GtkButton* button, gpointer user_data) {
    (void)button;
    playback_toggle_pause((MmpApp*)user_data);
}

static void volume_scale_changed_cb(GtkRange* range, gpointer user_data) {
    double volume = gtk_range_get_value(range) / 100.0;
    playback_set_volume((MmpApp*)user_data, volume);
}

static void track_progress_scale_value_changed_cb(GtkRange* range, gpointer user_data) {
    MmpApp* app = user_data;
    if (app->is_programmatic_change) return;

    static gint64 last_seek_time = 0;
    gint64 now = g_get_monotonic_time();

    // Throttle seeks to avoid flooding GStreamer
    if (now - last_seek_time < 100000) return; 

    double value = gtk_range_get_value(range);
    playback_seek(app, value);
    
    last_seek_time = now;
}

static void previous_track_clicked_cb(GtkButton* button, gpointer user_data) {
    (void)button;
    MmpApp* app = user_data;
    if (app->current_track_node && app->current_track_node->prev) {
        playback_play_track(app, app->current_track_node->prev);
    }
}

static void next_track_clicked_cb(GtkButton* button, gpointer user_data) {
    (void)button;
    MmpApp* app = user_data;
    if (app->current_track_node && app->current_track_node->next) {
        playback_play_track(app, app->current_track_node->next);
    }
}

static void navigation_row_selected_cb(GtkListBox* list_box, GtkListBoxRow* row, gpointer user_data) {
    (void)list_box;
    LibraryNavRows* rows = user_data;

    if (row == NULL) {
        return;
    }

    if (row == GTK_LIST_BOX_ROW(rows->library_header_row)) {
        gboolean expanded = !gtk_widget_get_visible(rows->recently_added_row);
        gtk_widget_set_visible(rows->recently_added_row, expanded);
        gtk_widget_set_visible(rows->albums_row, expanded);
        gtk_widget_set_visible(rows->artists_row, expanded);
        gtk_widget_set_visible(rows->songs_row, expanded);

        const char* current_page = gtk_stack_get_visible_child_name(rows->stack);
        if (g_strcmp0(current_page, "recently-added") != 0 &&
            g_strcmp0(current_page, "albums") != 0 &&
            g_strcmp0(current_page, "artists") != 0 &&
            g_strcmp0(current_page, "songs") != 0) {
            gtk_list_box_select_row(list_box, GTK_LIST_BOX_ROW(rows->recently_added_row));
        }
        return;
    }

    const char* page_name = g_object_get_data(G_OBJECT(row), "stack-page");
    if (page_name != NULL) {
        gtk_stack_set_visible_child_name(rows->stack, page_name);
        
        // Clear filters on manual navigation to show everything
        if (mmp_app) {
            g_free(mmp_app->selected_artist_filter);
            mmp_app->selected_artist_filter = NULL;
            g_free(mmp_app->selected_album_filter);
            mmp_app->selected_album_filter = NULL;
            
            if (mmp_app->albums_list) gtk_list_box_invalidate_filter(mmp_app->albums_list);
            if (mmp_app->songs_list) gtk_list_box_invalidate_filter(mmp_app->songs_list);
        }
    }
}

static gboolean on_drop_cb(GtkDropTarget* target, const GValue* value, double x, double y, gpointer user_data) {
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

static GtkListBoxRow* navigation_row(GtkBuilder* builder, const char* id, const char* page_name) {
    GtkListBoxRow* row = GTK_LIST_BOX_ROW(gtk_builder_get_object(builder, id));

    g_object_set_data(G_OBJECT(row), "stack-page", (gpointer)page_name);

    return row;
}

void app_activate_cb(GtkApplication* app) {
    GtkWindow* existing_window = gtk_application_get_active_window(app);
    if (existing_window != NULL) {
        gtk_window_present(existing_window);
        return;
    }

    mmp_app = g_new0(MmpApp, 1);
    playback_init(mmp_app);

    GtkBuilder* builder = gtk_builder_new_from_resource("/xyz/_1nfinit3/mmp/ui/window.ui");
    mmp_app->window = GTK_WINDOW(gtk_builder_get_object(builder, "main_window"));
    
    mmp_app->current_track_label = GTK_LABEL(gtk_builder_get_object(builder, "current_track_label"));
    mmp_app->elapsed_time_label = GTK_LABEL(gtk_builder_get_object(builder, "elapsed_time_label"));
    mmp_app->duration_label = GTK_LABEL(gtk_builder_get_object(builder, "duration_label"));
    mmp_app->track_progress_scale = GTK_SCALE(gtk_builder_get_object(builder, "track_progress_scale"));
    mmp_app->play_pause_button = GTK_BUTTON(gtk_builder_get_object(builder, "play_pause_button"));
    mmp_app->songs_list = GTK_LIST_BOX(gtk_builder_get_object(builder, "songs_list"));
    mmp_app->recently_added_list = GTK_LIST_BOX(gtk_builder_get_object(builder, "recently_added_list"));
    mmp_app->albums_list = GTK_LIST_BOX(gtk_builder_get_object(builder, "albums_list"));
    mmp_app->artists_list = GTK_LIST_BOX(gtk_builder_get_object(builder, "artists_list"));
    mmp_app->queue_list = GTK_LIST_BOX(gtk_builder_get_object(builder, "queue_list"));
    mmp_app->songs_search_entry = GTK_SEARCH_ENTRY(gtk_builder_get_object(builder, "songs_search_entry"));

    gtk_list_box_set_filter_func(mmp_app->songs_list, filter_songs_cb, mmp_app, NULL);
    gtk_list_box_set_filter_func(mmp_app->albums_list, filter_albums_cb, mmp_app, NULL);
    g_signal_connect(mmp_app->songs_search_entry, "search-changed", G_CALLBACK(search_changed_cb), mmp_app);
    g_signal_connect(mmp_app->songs_list, "row-activated", G_CALLBACK(song_row_activated_cb), mmp_app);
    g_signal_connect(mmp_app->recently_added_list, "row-activated", G_CALLBACK(song_row_activated_cb), mmp_app);
    g_signal_connect(mmp_app->albums_list, "row-activated", G_CALLBACK(album_row_activated_cb), mmp_app);
    g_signal_connect(mmp_app->artists_list, "row-activated", G_CALLBACK(artist_row_activated_cb), mmp_app);
    g_signal_connect(mmp_app->queue_list, "row-activated", G_CALLBACK(queue_row_activated_cb), mmp_app);

    GtkButton* prev_button = GTK_BUTTON(gtk_builder_get_object(builder, "previous_track_button"));
    GtkButton* next_button = GTK_BUTTON(gtk_builder_get_object(builder, "next_track_button"));

    GtkListBox* navigation_list = GTK_LIST_BOX(gtk_builder_get_object(builder, "navigation_list"));
    GtkStack* content_stack = GTK_STACK(gtk_builder_get_object(builder, "content_stack"));
    mmp_app->navigation_list = navigation_list;
    mmp_app->content_stack = content_stack;
    
    GtkWidget* volume_controls = GTK_WIDGET(gtk_builder_get_object(builder, "volume_controls"));
    GtkRevealer* volume_revealer = GTK_REVEALER(gtk_builder_get_object(builder, "volume_revealer"));
    GtkButton* mute_button = GTK_BUTTON(gtk_builder_get_object(builder, "mute_button"));
    GtkRange* volume_scale = GTK_RANGE(gtk_builder_get_object(builder, "volume_scale"));

    g_object_set_data(G_OBJECT(mute_button), "volume-scale", volume_scale);

    GtkDropTarget* drop_target = gtk_drop_target_new(G_TYPE_FILE, GDK_ACTION_COPY);
    g_signal_connect(drop_target, "drop", G_CALLBACK(on_drop_cb), mmp_app);
    gtk_widget_add_controller(GTK_WIDGET(mmp_app->window), GTK_EVENT_CONTROLLER(drop_target));

    GtkEventController* volume_motion = gtk_event_controller_motion_new();
    GtkListBoxRow* recently_added_row = navigation_row(builder, "nav_recently_added_row", "recently-added");
    LibraryNavRows* library_nav_rows = g_new(LibraryNavRows, 1);

    library_nav_rows->stack = content_stack;
    library_nav_rows->library_header_row = GTK_WIDGET(gtk_builder_get_object(builder, "nav_library_row"));
    library_nav_rows->recently_added_row = GTK_WIDGET(recently_added_row);
    library_nav_rows->albums_row = GTK_WIDGET(navigation_row(builder, "nav_albums_row", "albums"));
    library_nav_rows->artists_row = GTK_WIDGET(navigation_row(builder, "nav_artists_row", "artists"));
    library_nav_rows->songs_row = GTK_WIDGET(navigation_row(builder, "nav_songs_row", "songs"));
    
    navigation_row(builder, "nav_queue_row", "queue");
    navigation_row(builder, "nav_playlists_row", "playlists");
    navigation_row(builder, "nav_settings_row", "settings");

    g_signal_connect_data(navigation_list, "row-selected", G_CALLBACK(navigation_row_selected_cb), library_nav_rows, (GClosureNotify)g_free, 0);

    gtk_list_box_select_row(navigation_list, recently_added_row);
    load_css(mmp_app->window);
    
    g_signal_connect(volume_motion, "enter", G_CALLBACK(volume_controls_enter_cb), volume_revealer);
    g_signal_connect(volume_motion, "leave", G_CALLBACK(volume_controls_leave_cb), volume_revealer);
    gtk_widget_add_controller(volume_controls, volume_motion);
    
    g_signal_connect(mute_button, "clicked", G_CALLBACK(mute_button_clicked_cb), NULL);
    g_signal_connect(mmp_app->play_pause_button, "clicked", G_CALLBACK(play_pause_clicked_cb), mmp_app);
    g_signal_connect(prev_button, "clicked", G_CALLBACK(previous_track_clicked_cb), mmp_app);
    g_signal_connect(next_button, "clicked", G_CALLBACK(next_track_clicked_cb), mmp_app);
    g_signal_connect(volume_scale, "value-changed", G_CALLBACK(volume_scale_changed_cb), mmp_app);
    g_signal_connect(mmp_app->track_progress_scale, "value-changed", G_CALLBACK(track_progress_scale_value_changed_cb), mmp_app);

    const char* music_dir = g_get_user_special_dir(G_USER_DIRECTORY_MUSIC);
    if (music_dir) {
        scan_directory(mmp_app, music_dir);
    }

    g_timeout_add(500, (GSourceFunc)playback_update_ui, mmp_app);

    gtk_window_set_application(mmp_app->window, app);
    gtk_window_present(mmp_app->window);

    g_object_unref(builder);
}

void app_open_cb(GtkApplication* app, GFile** files, int n_files, const char* hint, gpointer user_data) {
    (void)hint;
    (void)user_data;

    app_activate_cb(app);
    if (n_files > 0 && mmp_app != NULL) {
        // Clear current playlist for "Open" action
        g_queue_foreach(mmp_app->playlist, (GFunc)g_free, NULL);
        g_queue_clear(mmp_app->playlist);
        mmp_app->current_track_node = NULL;

        for (int i = 0; i < n_files; i++) {
            char* path = g_file_get_path(files[i]);
            playback_add_to_playlist(mmp_app, path, i == 0);
            g_free(path);
        }
    }
}
