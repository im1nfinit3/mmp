#include "ui.h"
#include "ui_callbacks.h"
#include "playback.h"
#include "database.h"

#include <stdbool.h>

MmpApp* mmp_app = NULL;

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

void ui_update_queue(MmpApp* app) {
    if (!app->queue_list) return;

    // Build a temporary list of Song objects from the queue paths by projecting from library
    GList* queue_songs = NULL;
    GList* iter = app->playlist->head;
    while (iter) {
        const char* path = iter->data;
        
        // Find song in library
        Song* found_song = NULL;
        for (GList* l = app->library; l != NULL; l = l->next) {
            Song* s = (Song*)l->data;
            if (g_strcmp0(s->path, path) == 0) {
                found_song = s;
                break;
            }
        }

        if (found_song) {
            queue_songs = g_list_append(queue_songs, found_song);
        } else {
            // Fallback for files not in library
            Song* s = g_new0(Song, 1);
            s->path = g_strdup(path);
            char* basename = g_path_get_basename(path);
            s->title = g_strdup(basename);
            char* dot = g_strrstr(s->title, ".");
            if (dot) *dot = '\0';
            s->artist = g_strdup("Unknown Artist");
            s->album = g_strdup("Unknown Album");
            g_free(basename);
            queue_songs = g_list_append(queue_songs, s);
            // In a better architecture, we'd add this to a "shadow library" or just library
        }
        iter = iter->next;
    }

    // Populate the queue_list using the same logic as other lists
    // Note: the queue currently doesn't support global filters, but we could add them if needed.
    ui_refresh_view_list(app, app->queue_list, queue_songs, false);

    // After population, we need to add the playlist-node data and decorations
    // This part is slightly different for the queue as it needs specific controllers
    GtkWidget* row = gtk_widget_get_first_child(GTK_WIDGET(app->queue_list));
    iter = app->playlist->head;
    GList* qs_iter = queue_songs;
    while (row && iter && qs_iter) {
        Song* song = qs_iter->data;
        
        g_object_set_data(G_OBJECT(row), "playlist-node", iter);
        
        if (iter == app->current_track_node) {
            GtkWidget* box = gtk_list_box_row_get_child(GTK_LIST_BOX_ROW(row));
            GtkWidget* active_indicator = gtk_image_new_from_icon_name("audio-volume-medium-symbolic");
            gtk_box_append(GTK_BOX(box), active_indicator);
        }

        // Add queue-specific controllers
        GtkGesture* gesture = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture), GDK_BUTTON_SECONDARY);
        g_signal_connect(gesture, "pressed", G_CALLBACK(queue_row_secondary_click_cb), NULL);
        gtk_widget_add_controller(row, GTK_EVENT_CONTROLLER(gesture));

        GtkDragSource* drag_source = gtk_drag_source_new();
        gtk_drag_source_set_actions(drag_source, GDK_ACTION_COPY | GDK_ACTION_MOVE);
        g_signal_connect(drag_source, "drag-begin", G_CALLBACK(queue_drag_begin_cb), NULL);
        
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

        row = gtk_widget_get_next_sibling(row);
        iter = iter->next;
        qs_iter = qs_iter->next;
    }

    // Clean up temporary song pointers (the Songs themselves are either in library or were leaked in fallback)
    g_list_free(queue_songs);
}

GtkWidget* create_song_row_box(Song* song) {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_start(box, 12);
    gtk_widget_set_margin_end(box, 12);
    gtk_widget_set_margin_top(box, 8);
    gtk_widget_set_margin_bottom(box, 8);

    GtkWidget* title_label = gtk_label_new(song->title);
    gtk_label_set_xalign(GTK_LABEL(title_label), 0);
    gtk_widget_set_hexpand(title_label, TRUE);
    gtk_label_set_ellipsize(GTK_LABEL(title_label), PANGO_ELLIPSIZE_END);
    gtk_box_append(GTK_BOX(box), title_label);

    GtkWidget* artist_label = gtk_label_new(song->artist);
    gtk_widget_add_css_class(artist_label, "dim-label");
    gtk_label_set_ellipsize(GTK_LABEL(artist_label), PANGO_ELLIPSIZE_START);
    gtk_label_set_max_width_chars(GTK_LABEL(artist_label), 15);
    gtk_box_append(GTK_BOX(box), artist_label);

    GtkWidget* album_label = gtk_label_new(song->album);
    gtk_widget_add_css_class(album_label, "dim-label");
    gtk_label_set_ellipsize(GTK_LABEL(album_label), PANGO_ELLIPSIZE_START);
    gtk_label_set_max_width_chars(GTK_LABEL(album_label), 15);
    gtk_box_append(GTK_BOX(box), album_label);

    if (song->duration_str) {
        GtkWidget* duration_label = gtk_label_new(song->duration_str);
        gtk_widget_add_css_class(duration_label, "dim-label");
        gtk_widget_set_size_request(duration_label, 40, -1);
        gtk_box_append(GTK_BOX(box), duration_label);
    }

    return box;
}

void free_song(Song* song) {
    if (!song) return;
    g_free(song->path);
    g_free(song->title);
    g_free(song->artist);
    g_free(song->album);
    g_free(song->duration_str);
    g_free(song);
}

static void free_song_filter(gpointer data) {
    SongFilter* filter = data;
    if (filter->notify && filter->user_data) {
        filter->notify(filter->user_data);
    }
    g_free(filter);
}

void ui_clear_filters(MmpApp* app) {
    g_list_free_full(app->current_view_filters, free_song_filter);
    app->current_view_filters = NULL;
}

void ui_set_view(MmpApp* app, GList* base_list, bool owned, bool reverse) {
    if (app->current_view_base_list && app->current_view_base_list_owned) {
        g_list_free(app->current_view_base_list);
    }
    app->current_view_base_list = base_list;
    app->current_view_base_list_owned = owned;
    app->current_view_reverse = reverse;
}

void ui_add_filter(MmpApp* app, SongFilterFunc func, gpointer data, GDestroyNotify notify) {
    SongFilter* filter = g_new0(SongFilter, 1);
    filter->filter = func;
    filter->user_data = data;
    filter->notify = notify;
    app->current_view_filters = g_list_append(app->current_view_filters, filter);
}

void ui_refresh_view_list(MmpApp* app, GtkListBox* list, GList* base_list, bool reverse) {
    if (!list || !base_list) return;

    // Clear the list
    GtkWidget* child;
    while ((child = gtk_widget_get_first_child(GTK_WIDGET(list))) != NULL) {
        gtk_list_box_remove(list, child);
    }

    GList* songs = base_list;
    if (reverse) {
        songs = g_list_copy(songs);
        songs = g_list_reverse(songs);
    }

    for (GList* l = songs; l != NULL; l = l->next) {
        Song* song = l->data;
        bool pass = true;

        for (GList* f = app->current_view_filters; f != NULL; f = f->next) {
            SongFilter* filter = f->data;
            if (!filter->filter(song, filter->user_data)) {
                pass = false;
                break;
            }
        }

        if (pass) {
            ui_add_song_to_list(app, list, song, false, false);
        }
    }

    if (reverse) {
        g_list_free(songs);
    }
}

void ui_refresh_view(MmpApp* app) {
    ui_refresh_view_list(app, app->songs_list, app->current_view_base_list, app->current_view_reverse);
}

bool search_filter_func(Song* song, gpointer user_data) {
    const char* search_text = user_data;
    if (!search_text || strlen(search_text) == 0) return true;

    char* search_lower = g_utf8_strdown(search_text, -1);
    char* title_lower = g_utf8_strdown(song->title, -1);
    char* artist_lower = g_utf8_strdown(song->artist, -1);
    char* album_lower = g_utf8_strdown(song->album, -1);

    bool visible = (strstr(title_lower, search_lower) != NULL) ||
                   (strstr(artist_lower, search_lower) != NULL) ||
                   (strstr(album_lower, search_lower) != NULL);

    g_free(search_lower);
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

void ui_add_song_to_list(MmpApp* app, GtkListBox* list, Song* song, bool prepend, bool own_song) {
    if (!list) return;

    GtkWidget* row = gtk_list_box_row_new();
    GtkWidget* box = create_song_row_box(song);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);
    
    if (own_song) {
        g_object_set_data_full(G_OBJECT(row), "song-data", song, (GDestroyNotify)free_song);
    } else {
        g_object_set_data(G_OBJECT(row), "song-data", song);
    }

    if (prepend) {
        gtk_list_box_prepend(list, row);
    } else {
        gtk_list_box_append(list, row);
    }

    GtkGesture* gesture = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture), GDK_BUTTON_SECONDARY);
    g_signal_connect(gesture, "pressed", G_CALLBACK(song_row_secondary_click_cb), NULL);
    gtk_widget_add_controller(row, GTK_EVENT_CONTROLLER(gesture));
}

void ui_populate_songs(MmpApp* app, GList* songs, bool own_songs) {
    if (!app->songs_list) return;

    // Clear the list
    GtkWidget* child;
    while ((child = gtk_widget_get_first_child(GTK_WIDGET(app->songs_list))) != NULL) {
        gtk_list_box_remove(app->songs_list, child);
    }

    // Populate
    for (GList* l = songs; l != NULL; l = l->next) {
        ui_add_song_to_list(app, app->songs_list, (Song*)l->data, false, own_songs);
    }
}

static void add_song_to_ui(MmpApp* app, Song* song) {
    // If the library is the base list for the current view, refresh it
    if (app->current_view_base_list == app->library) {
        ui_refresh_view(app);
    }
    
    add_to_library_ui(app, song);
}

typedef struct {
    MmpApp* app;
    Song* song;
} SongUpdateData;

static gboolean add_song_idle_cb(gpointer user_data) {
    SongUpdateData* data = user_data;
    data->app->library = g_list_append(data->app->library, data->song);
    add_song_to_ui(data->app, data->song);
    g_free(data);
    return FALSE;
}

static gboolean update_song_ui_idle_cb(gpointer user_data) {
    SongUpdateData* data = user_data;
    GtkListBox* list = data->app->songs_list;
    
    if (list) {
        GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(list));
        while (child) {
            Song* s = g_object_get_data(G_OBJECT(child), "song-data");
            if (s && g_strcmp0(s->path, data->song->path) == 0) {
                if (s != data->song) {
                    g_free(s->title); s->title = g_strdup(data->song->title);
                    g_free(s->artist); s->artist = g_strdup(data->song->artist);
                    g_free(s->album); s->album = g_strdup(data->song->album);
                    g_free(s->duration_str); s->duration_str = g_strdup(data->song->duration_str);
                }
                
                GtkWidget* new_box = create_song_row_box(s);
                gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(child), new_box);
            }
            child = gtk_widget_get_next_sibling(child);
        }
    }
    g_free(data);
    return FALSE;
}

static void scan_directory_recursive(MmpApp* app, const char* path, GHashTable* existing_paths) {
    GFile* dir = g_file_new_for_path(path);
    GFileEnumerator* enumerator = g_file_enumerate_children(dir, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, NULL);

    if (enumerator) {
        GFileInfo* info;
        while ((info = g_file_enumerator_next_file(enumerator, NULL, NULL)) != NULL) {
            const char* name = g_file_info_get_name(info);
            GFile* child = g_file_get_child(dir, name);
            char* child_path = g_file_get_path(child);

            if (g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY) {
                scan_directory_recursive(app, child_path, existing_paths);
            } else if (g_str_has_suffix(name, ".mp3") || g_str_has_suffix(name, ".flac") || 
                       g_str_has_suffix(name, ".ogg") || g_str_has_suffix(name, ".wav") ||
                       g_str_has_suffix(name, ".m4a")) {
                
                Song* existing_song = g_hash_table_lookup(existing_paths, child_path);
                if (!existing_song) {
                    Song* song = g_new0(Song, 1);
                    song->path = g_strdup(child_path);
                    
                    char* title = g_strdup(name);
                    char* dot = g_strrstr(title, ".");
                    if (dot) *dot = '\0';
                    song->title = title;
                    
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

                    playback_get_metadata(app, song);
                    db_save_song(app->library_db, song);

                    SongUpdateData* update_data = g_new0(SongUpdateData, 1);
                    update_data->app = app;
                    update_data->song = song;
                    g_idle_add(add_song_idle_cb, update_data);
                    
                    if (grand_parent) g_object_unref(grand_parent);
                    if (parent) g_object_unref(parent);
                } else if (existing_song->duration_str == NULL) {
                    playback_get_metadata(app, existing_song);
                    if (existing_song->duration_str) {
                        db_save_song(app->library_db, existing_song);
                        
                        SongUpdateData* update_data = g_new0(SongUpdateData, 1);
                        update_data->app = app;
                        update_data->song = existing_song;
                        g_idle_add(update_song_ui_idle_cb, update_data);
                    }
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

static void scan_directory_thread(GTask* task, gpointer source_object, gpointer task_data, GCancellable* cancellable) {
    (void)task; (void)source_object; (void)cancellable;
    MmpApp* app = (MmpApp*)task_data;

    GHashTable* existing_paths = g_hash_table_new(g_str_hash, g_str_equal);
    for (GList* l = app->library; l != NULL; l = l->next) {
        Song* s = l->data;
        g_hash_table_insert(existing_paths, s->path, s);
    }

    const char* music_dir = g_get_user_special_dir(G_USER_DIRECTORY_MUSIC);
    if (music_dir) {
        scan_directory_recursive(app, music_dir, existing_paths);
    }
    g_hash_table_destroy(existing_paths);
}

static void scan_directory_async(MmpApp* app) {
    GTask* task = g_task_new(NULL, NULL, NULL, NULL);
    g_task_set_task_data(task, app, NULL);
    g_task_run_in_thread(task, scan_directory_thread);
    g_object_unref(task);
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

void ui_update_playlists(MmpApp* app) {
    GtkWidget* nav_list = GTK_WIDGET(app->navigation_list);
    if (!nav_list) return;

    GtkWidget* child = gtk_widget_get_first_child(nav_list);
    while (child) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        if (g_object_get_data(G_OBJECT(child), "is-playlist-row")) {
            gtk_list_box_remove(GTK_LIST_BOX(nav_list), child);
        }
        child = next;
    }

    GtkWidget* header_row = GTK_WIDGET(g_object_get_data(G_OBJECT(app->window), "nav-playlists-row"));
    if (!header_row) return;
    int index = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(header_row));

    GList* playlists = db_get_playlists(app->db);
    int i = 1;
    for (GList* l = playlists; l != NULL; l = l->next) {
        Playlist* p = l->data;
        GtkWidget* row = gtk_list_box_row_new();
        g_object_set_data(G_OBJECT(row), "is-playlist-row", GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(row), "stack-page", (gpointer)"songs-view");
        g_object_set_data_full(G_OBJECT(row), "playlist", p, (GDestroyNotify)free_playlist);
        
        GtkWidget* label = gtk_label_new(p->name);
        gtk_label_set_xalign(GTK_LABEL(label), 0);
        gtk_widget_set_margin_start(label, 24);
        gtk_widget_set_margin_top(label, 12);
        gtk_widget_set_margin_bottom(label, 12);
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

    char* config_dir = g_build_filename(g_get_user_config_dir(), "mmp", NULL);
    g_mkdir_with_parents(config_dir, 0755);
    char* db_path = g_build_filename(config_dir, "playlists.db", NULL);
    db_init(db_path, &mmp_app->db);
    g_free(db_path);

    char* library_db_path = g_build_filename(config_dir, "library.db", NULL);
    db_init(library_db_path, &mmp_app->library_db);
    g_free(library_db_path);

    g_free(config_dir);

    GtkBuilder* builder = gtk_builder_new_from_resource("/xyz/_1nfinit3/mmp/ui/window.ui");
    mmp_app->window = GTK_WINDOW(gtk_builder_get_object(builder, "main_window"));
    
    mmp_app->current_track_label = GTK_LABEL(gtk_builder_get_object(builder, "current_track_label"));
    mmp_app->elapsed_time_label = GTK_LABEL(gtk_builder_get_object(builder, "elapsed_time_label"));
    mmp_app->duration_label = GTK_LABEL(gtk_builder_get_object(builder, "duration_label"));
    mmp_app->track_progress_scale = GTK_SCALE(gtk_builder_get_object(builder, "track_progress_scale"));
    mmp_app->play_pause_button = GTK_BUTTON(gtk_builder_get_object(builder, "play_pause_button"));
    mmp_app->shuffle_button = GTK_BUTTON(gtk_builder_get_object(builder, "shuffle_button"));
    mmp_app->repeat_button = GTK_BUTTON(gtk_builder_get_object(builder, "repeat_button"));
    mmp_app->songs_list = GTK_LIST_BOX(gtk_builder_get_object(builder, "songs_list"));
    mmp_app->albums_list = GTK_LIST_BOX(gtk_builder_get_object(builder, "albums_list"));
    mmp_app->artists_list = GTK_LIST_BOX(gtk_builder_get_object(builder, "artists_list"));
    mmp_app->queue_list = GTK_LIST_BOX(gtk_builder_get_object(builder, "queue_list"));
    mmp_app->songs_search_entry = GTK_SEARCH_ENTRY(gtk_builder_get_object(builder, "songs_search_entry"));

    // Load cached library
    GList* cached_songs = db_get_all_songs(mmp_app->library_db);
    for (GList* l = cached_songs; l != NULL; l = l->next) {
        Song* s = l->data;
        mmp_app->library = g_list_append(mmp_app->library, s);
        add_to_library_ui(mmp_app, s);
    }
    g_list_free(cached_songs);

    gtk_list_box_set_filter_func(mmp_app->albums_list, filter_albums_cb, mmp_app, NULL);
    g_signal_connect(mmp_app->songs_search_entry, "search-changed", G_CALLBACK(search_changed_cb), mmp_app);
    g_signal_connect(mmp_app->songs_list, "row-activated", G_CALLBACK(song_row_activated_cb), mmp_app);
    g_signal_connect(mmp_app->albums_list, "row-activated", G_CALLBACK(album_row_activated_cb), mmp_app);
    g_signal_connect(mmp_app->artists_list, "row-activated", G_CALLBACK(artist_row_activated_cb), mmp_app);
    g_signal_connect(mmp_app->queue_list, "row-activated", G_CALLBACK(queue_row_activated_cb), mmp_app);

    GtkButton* prev_button = GTK_BUTTON(gtk_builder_get_object(builder, "previous_track_button"));
    GtkButton* next_button = GTK_BUTTON(gtk_builder_get_object(builder, "next_track_button"));

    GtkListBox* navigation_list = GTK_LIST_BOX(gtk_builder_get_object(builder, "navigation_list"));
    GtkStack* content_stack = GTK_STACK(gtk_builder_get_object(builder, "content_stack"));
    mmp_app->navigation_list = navigation_list;
    mmp_app->content_stack = content_stack;

    GtkListBoxRow* nav_playlists_row = GTK_LIST_BOX_ROW(gtk_builder_get_object(builder, "nav_playlists_row"));
    g_object_set_data(G_OBJECT(mmp_app->window), "nav-playlists-row", nav_playlists_row);

    GtkGesture* playlists_right_click = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(playlists_right_click), GDK_BUTTON_SECONDARY);
    g_signal_connect(playlists_right_click, "released", G_CALLBACK(playlists_header_right_clicked_cb), mmp_app);
    gtk_widget_add_controller(GTK_WIDGET(nav_playlists_row), GTK_EVENT_CONTROLLER(playlists_right_click));

    ui_update_playlists(mmp_app);
    
    GtkWidget* volume_controls = GTK_WIDGET(gtk_builder_get_object(builder, "volume_controls"));
    GtkRevealer* volume_revealer = GTK_REVEALER(gtk_builder_get_object(builder, "volume_revealer"));
    GtkButton* mute_button = GTK_BUTTON(gtk_builder_get_object(builder, "mute_button"));
    GtkRange* volume_scale = GTK_RANGE(gtk_builder_get_object(builder, "volume_scale"));

    g_object_set_data(G_OBJECT(mute_button), "volume-scale", volume_scale);

    GtkDropTarget* drop_target = gtk_drop_target_new(G_TYPE_FILE, GDK_ACTION_COPY);
    g_signal_connect(drop_target, "drop", G_CALLBACK(on_drop_cb), mmp_app);
    gtk_widget_add_controller(GTK_WIDGET(mmp_app->window), GTK_EVENT_CONTROLLER(drop_target));

    GtkEventController* volume_motion = gtk_event_controller_motion_new();
    GtkListBoxRow* recently_added_row = navigation_row(builder, "nav_recently_added_row", "songs-view");
    g_object_set_data(G_OBJECT(recently_added_row), "view-mode", (gpointer)"recently-added");
    
    LibraryNavRows* library_nav_rows = g_new(LibraryNavRows, 1);

    library_nav_rows->stack = content_stack;
    library_nav_rows->recently_added_row = GTK_WIDGET(recently_added_row);
    library_nav_rows->albums_row = GTK_WIDGET(navigation_row(builder, "nav_albums_row", "albums"));
    library_nav_rows->artists_row = GTK_WIDGET(navigation_row(builder, "nav_artists_row", "artists"));
    GtkListBoxRow* songs_row = navigation_row(builder, "nav_songs_row", "songs-view");
    library_nav_rows->songs_row = GTK_WIDGET(songs_row);
    g_object_set_data(G_OBJECT(songs_row), "view-mode", (gpointer)"songs");
    
    navigation_row(builder, "nav_queue_row", "queue");
    navigation_row(builder, "nav_settings_row", "settings");

    g_signal_connect_data(navigation_list, "row-selected", G_CALLBACK(navigation_row_selected_cb), library_nav_rows, (GClosureNotify)g_free, 0);

    gtk_list_box_select_row(navigation_list, recently_added_row);
    load_css(mmp_app->window);
    
    g_signal_connect(volume_motion, "enter", G_CALLBACK(volume_controls_enter_cb), volume_revealer);
    g_signal_connect(volume_motion, "leave", G_CALLBACK(volume_controls_leave_cb), volume_revealer);
    gtk_widget_add_controller(volume_controls, volume_motion);
    
    g_signal_connect(mute_button, "clicked", G_CALLBACK(mute_button_clicked_cb), NULL);
    g_signal_connect(mmp_app->shuffle_button, "clicked", G_CALLBACK(shuffle_clicked_cb), mmp_app);
    g_signal_connect(mmp_app->repeat_button, "clicked", G_CALLBACK(repeat_clicked_cb), mmp_app);
    g_signal_connect(mmp_app->play_pause_button, "clicked", G_CALLBACK(play_pause_clicked_cb), mmp_app);
    g_signal_connect(prev_button, "clicked", G_CALLBACK(previous_track_clicked_cb), mmp_app);
    g_signal_connect(next_button, "clicked", G_CALLBACK(next_track_clicked_cb), mmp_app);
    g_signal_connect(volume_scale, "value-changed", G_CALLBACK(volume_scale_changed_cb), mmp_app);
    g_signal_connect(mmp_app->track_progress_scale, "value-changed", G_CALLBACK(track_progress_scale_value_changed_cb), mmp_app);

    g_timeout_add(500, (GSourceFunc)playback_update_ui, mmp_app);

    gtk_window_set_application(mmp_app->window, app);
    gtk_window_present(mmp_app->window);

    scan_directory_async(mmp_app);

    g_object_unref(builder);
}

void app_open_cb(GtkApplication* app, GFile** files, int n_files, const char* hint, gpointer user_data) {
    (void)hint;
    (void)user_data;

    app_activate_cb(app);
    if (n_files > 0 && mmp_app != NULL) {
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
