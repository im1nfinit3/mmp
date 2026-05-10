#include "playback.h"
#include "ui.h"
#include "database.h"
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>

void playback_rebuild_unplayed_pool(MmpApp* app) {
    if (app->unplayed_pool) {
        g_list_free(app->unplayed_pool);
        app->unplayed_pool = NULL;
    }
    
    if (!app->shuffle_mode) return;
    
    for (GList* l = app->playlist->head; l != NULL; l = l->next) {
        // Don't add current track to pool if we just started shuffling
        if (l != app->current_track_node) {
            app->unplayed_pool = g_list_append(app->unplayed_pool, l);
        }
    }
}

static GList* playback_get_next_node(MmpApp* app) {
    if (app->repeat_mode == REPEAT_ONE && app->current_track_node) {
        return app->current_track_node;
    }
    
    if (app->shuffle_mode) {
        if (app->unplayed_pool == NULL) {
            if (app->repeat_mode == REPEAT_ALL || app->current_track_node == NULL) {
                playback_rebuild_unplayed_pool(app);
            } else {
                return NULL;
            }
        }
        
        if (app->unplayed_pool == NULL) return NULL;
        
        int length = g_list_length(app->unplayed_pool);
        int index = g_random_int_range(0, length);
        GList* pool_link = g_list_nth(app->unplayed_pool, index);
        GList* playlist_node = pool_link->data;
        
        app->unplayed_pool = g_list_delete_link(app->unplayed_pool, pool_link);
        return playlist_node;
    } else {
        if (app->current_track_node && app->current_track_node->next) {
            return app->current_track_node->next;
        } else if (app->repeat_mode == REPEAT_ALL) {
            return app->playlist->head;
        }
    }
    
    return NULL;
}

static void playbin_bus_message_cb(GstBus* bus, GstMessage* msg, gpointer user_data) {
    (void)bus;
    MmpApp* app = user_data;

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_TAG: {
            GstTagList* tags = NULL;
            gst_message_parse_tag(msg, &tags);
            if (tags) {
                gchar* artist = NULL;
                gchar* title = NULL;

                gst_tag_list_get_string(tags, GST_TAG_ARTIST, &artist);
                gst_tag_list_get_string(tags, GST_TAG_TITLE, &title);

                if (artist || title) {
                    char* label = g_strdup_printf("%s - %s", 
                        artist ? artist : "Unknown Artist", 
                        title ? title : "Unknown Track");
                    gtk_label_set_label(app->current_track_label, label);
                    g_free(label);

                    // Update library metadata with real tags discovered during playback
                    for (GList* l = app->library; l != NULL; l = l->next) {
                        Song* s = (Song*)l->data;
                        if (g_strcmp0(s->path, app->current_file_path) == 0) {
                            if (title) {
                                g_free(s->title);
                                s->title = g_strdup(title);
                            }
                            if (artist) {
                                g_free(s->artist);
                                s->artist = g_strdup(artist);
                            }
                            db_save_song(app->library_db, s);
                            break;
                        }
                    }
                    ui_update_queue(app);
                }
                
                g_free(artist);
                g_free(title);
                gst_tag_list_unref(tags);
            }
            break;
        }
        case GST_MESSAGE_EOS: {
            GList* next = playback_get_next_node(app);
            if (next) {
                playback_play_track(app, next);
                ui_update_queue(app);
            } else {
                gst_element_set_state(app->playbin, GST_STATE_READY);
                gtk_button_set_icon_name(app->play_pause_button, "media-playback-start-symbolic");
            }
            break;
        }
        case GST_MESSAGE_ERROR: {
            GError* err;
            gchar* debug;
            gst_message_parse_error(msg, &err, &debug);
            g_printerr("GStreamer error: %s\n", err->message);
            g_error_free(err);
            g_free(debug);
            break;
        }
        default:
            break;
    }
}

void playback_init(MmpApp* app) {
    app->playbin = gst_element_factory_make("playbin", "player");
    app->playlist = g_queue_new();
    app->shuffle_mode = false;
    app->repeat_mode = REPEAT_OFF;
    app->unplayed_pool = NULL;

    GError* err = NULL;
    app->discoverer = gst_discoverer_new(2 * GST_SECOND, &err);
    if (err) {
        g_warning("Could not create GstDiscoverer: %s", err->message);
        g_error_free(err);
    }

    GstBus* bus = gst_element_get_bus(app->playbin);
    gst_bus_add_signal_watch(bus);
    g_signal_connect(bus, "message", G_CALLBACK(playbin_bus_message_cb), app);
    gst_object_unref(bus);
}

void playback_get_metadata(MmpApp* app, Song* song) {
    if (!app->discoverer) return;

    char* uri = g_filename_to_uri(song->path, NULL, NULL);
    if (!uri) return;

    GError* err = NULL;
    GstDiscovererInfo* info = gst_discoverer_discover_uri(app->discoverer, uri, &err);

    if (info) {
        GstClockTime duration = gst_discoverer_info_get_duration(info);
        if (GST_CLOCK_TIME_IS_VALID(duration)) {
            int seconds = (int)(duration / GST_SECOND);
            int minutes = seconds / 60;
            seconds %= 60;
            g_free(song->duration_str);
            song->duration_str = g_strdup_printf("%d:%02d", minutes, seconds);
            g_print("Extracted duration for %s: %s\n", song->path, song->duration_str);
        } else {
            g_print("Duration not valid for %s\n", song->path);
        }

        const GstTagList* tags = gst_discoverer_info_get_tags(info);
        if (tags) {
            char* title = NULL;
            char* artist = NULL;
            char* album = NULL;

            if (gst_tag_list_get_string(tags, GST_TAG_TITLE, &title)) {
                if (title && title[0] != '\0') {
                    g_free(song->title);
                    song->title = title;
                } else {
                    g_free(title);
                }
            }
            if (gst_tag_list_get_string(tags, GST_TAG_ARTIST, &artist)) {
                if (artist && artist[0] != '\0') {
                    g_free(song->artist);
                    song->artist = artist;
                } else {
                    g_free(artist);
                }
            }
            if (gst_tag_list_get_string(tags, GST_TAG_ALBUM, &album)) {
                if (album && album[0] != '\0') {
                    g_free(song->album);
                    song->album = album;
                } else {
                    g_free(album);
                }
            }
        }
        gst_discoverer_info_unref(info);
    } else {
        if (err) {
            g_clear_error(&err);
        }
    }

    g_free(uri);
}

void playback_shuffle_toggle(MmpApp* app) {
    app->shuffle_mode = !app->shuffle_mode;
    if (app->shuffle_mode) {
        playback_rebuild_unplayed_pool(app);
    } else {
        if (app->unplayed_pool) {
            g_list_free(app->unplayed_pool);
            app->unplayed_pool = NULL;
        }
    }
}

void playback_repeat_toggle(MmpApp* app) {
    app->repeat_mode = (app->repeat_mode + 1) % 3;
}

void playback_play_track(MmpApp* app, GList* node) {
    if (node == NULL) return;
    
    app->current_track_node = node;
    char* path = node->data;
    
    g_free(app->current_file_path);
    app->current_file_path = g_strdup(path);
    
    char* uri = g_filename_to_uri(path, NULL, NULL);
    if (uri) {
        gst_element_set_state(app->playbin, GST_STATE_NULL);
        g_object_set(app->playbin, "uri", uri, NULL);
        gst_element_set_state(app->playbin, GST_STATE_PLAYING);
        
        // Try to find the song in the library to get immediate metadata
        Song* found_song = NULL;
        for (GList* l = app->library; l != NULL; l = l->next) {
            Song* s = (Song*)l->data;
            if (g_strcmp0(s->path, path) == 0) {
                found_song = s;
                break;
            }
        }

        if (found_song) {
            char* label = g_strdup_printf("%s - %s", found_song->artist, found_song->title);
            gtk_label_set_label(app->current_track_label, label);
            g_free(label);
        } else {
            char* basename = g_path_get_basename(path);
            gtk_label_set_label(app->current_track_label, basename);
            g_free(basename);
        }
        
        gtk_button_set_icon_name(app->play_pause_button, "media-playback-pause-symbolic");
        g_free(uri);
    }
    ui_update_queue(app);
    ui_refresh_view(app);
}

static GList* playback_add_to_playlist_internal(MmpApp* app, const char* path, bool play_now, bool update_ui) {
    if (path == NULL) return NULL;
    
    g_queue_push_tail(app->playlist, g_strdup(path));
    GList* new_node = g_queue_peek_tail_link(app->playlist);

    if (app->shuffle_mode && app->unplayed_pool) {
        app->unplayed_pool = g_list_append(app->unplayed_pool, new_node);
    }
    
    if (play_now || app->current_track_node == NULL) {
        playback_play_track(app, new_node);
    } else if (update_ui) {
        ui_update_queue(app);
    }
    return new_node;
}

GList* playback_add_to_playlist(MmpApp* app, const char* path, bool play_now) {
    return playback_add_to_playlist_internal(app, path, play_now, true);
}

void playback_add_songs_to_playlist(MmpApp* app, GList* songs) {
    if (!songs) return;
    
    for (GList* l = songs; l != NULL; l = l->next) {
        Song* song = l->data;
        playback_add_to_playlist_internal(app, song->path, false, false);
    }
    ui_update_queue(app);
}

void playback_open_file(MmpApp* app, const char* path) {
    // Clear playlist and play this file
    if (app->unplayed_pool) {
        g_list_free(app->unplayed_pool);
        app->unplayed_pool = NULL;
    }

    g_queue_foreach(app->playlist, (GFunc)g_free, NULL);
    g_queue_clear(app->playlist);
    app->current_track_node = NULL;
    
    playback_add_to_playlist(app, path, true);
}

void playback_toggle_pause(MmpApp* app) {
    GstState state;
    gst_element_get_state(app->playbin, &state, NULL, 0);

    if (state == GST_STATE_PLAYING) {
        gst_element_set_state(app->playbin, GST_STATE_PAUSED);
        gtk_button_set_icon_name(app->play_pause_button, "media-playback-start-symbolic");
        gtk_widget_set_tooltip_text(GTK_WIDGET(app->play_pause_button), "Play");
    } else if (state == GST_STATE_PAUSED || state == GST_STATE_READY) {
        gst_element_set_state(app->playbin, GST_STATE_PLAYING);
        gtk_button_set_icon_name(app->play_pause_button, "media-playback-pause-symbolic");
        gtk_widget_set_tooltip_text(GTK_WIDGET(app->play_pause_button), "Pause");
    }
}

void playback_seek(MmpApp* app, double seconds) {
    gst_element_seek_simple(app->playbin, GST_FORMAT_TIME, 
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, 
        (gint64)(seconds * GST_SECOND));
}

void playback_set_volume(MmpApp* app, double volume) {
    g_object_set(app->playbin, "volume", volume, NULL);
}

void playback_set_mute(MmpApp* app, bool mute) {
    g_object_set(app->playbin, "mute", mute, NULL);
}

void playback_play_next(MmpApp* app, const char* path) {
    if (path == NULL) return;
    
    char* path_copy = g_strdup(path);
    GList* new_node;
    if (app->current_track_node) {
        g_queue_insert_after(app->playlist, app->current_track_node, path_copy);
        new_node = app->current_track_node->next;
    } else {
        g_queue_push_head(app->playlist, path_copy);
        new_node = app->playlist->head;
    }

    if (app->shuffle_mode && app->unplayed_pool) {
        app->unplayed_pool = g_list_append(app->unplayed_pool, new_node);
    }

    ui_update_queue(app);
}

void playback_skip_next(MmpApp* app) {
    GList* next = playback_get_next_node(app);
    if (next) {
        playback_play_track(app, next);
    } else {
        // If we can't skip next (e.g. end of playlist and repeat off), just stop
        gst_element_set_state(app->playbin, GST_STATE_READY);
        gtk_button_set_icon_name(app->play_pause_button, "media-playback-start-symbolic");
    }
}

void playback_remove_from_playlist(MmpApp* app, GList* node) {
    if (node == NULL) return;
    
    // Remove from unplayed pool if it's there
    if (app->unplayed_pool) {
        GList* pool_link = g_list_find(app->unplayed_pool, node);
        if (pool_link) {
            app->unplayed_pool = g_list_delete_link(app->unplayed_pool, pool_link);
        }
    }
    
    bool is_current = (node == app->current_track_node);
    
    // If it's the current track and there's a next one, play the next one
    if (is_current && node->next) {
        playback_play_track(app, node->next);
    } else if (is_current) {
        // No next track, stop playback
        gst_element_set_state(app->playbin, GST_STATE_READY);
        gtk_button_set_icon_name(app->play_pause_button, "media-playback-start-symbolic");
        app->current_track_node = NULL;
    }
    
    g_free(node->data);
    g_queue_delete_link(app->playlist, node);
    ui_update_queue(app);
}

void playback_clear_playlist(MmpApp* app) {
    if (app->unplayed_pool) {
        g_list_free(app->unplayed_pool);
        app->unplayed_pool = NULL;
    }

    gst_element_set_state(app->playbin, GST_STATE_READY);
    app->current_track_node = NULL;
    g_free(app->current_file_path);
    app->current_file_path = NULL;
    gtk_label_set_label(app->current_track_label, "No track playing");
    gtk_button_set_icon_name(app->play_pause_button, "media-playback-start-symbolic");

    g_queue_foreach(app->playlist, (GFunc)g_free, NULL);
    g_queue_clear(app->playlist);
    
    ui_update_queue(app);
}

gboolean playback_update_ui(MmpApp* app) {
    if (!app->playbin) return TRUE;

    GstState state;
    gst_element_get_state(app->playbin, &state, NULL, 0);
    if (state != GST_STATE_PLAYING && state != GST_STATE_PAUSED) return TRUE;

    gint64 duration, position;
    if (gst_element_query_duration(app->playbin, GST_FORMAT_TIME, &duration) &&
        gst_element_query_position(app->playbin, GST_FORMAT_TIME, &position)) {
        
        double pos_sec = (double)position / GST_SECOND;
        double dur_sec = (double)duration / GST_SECOND;

        app->is_programmatic_change = true;
        gtk_range_set_range(GTK_RANGE(app->track_progress_scale), 0, dur_sec);
        gtk_range_set_value(GTK_RANGE(app->track_progress_scale), pos_sec);
        app->is_programmatic_change = false;

        char* pos_str = g_strdup_printf("%d:%02d", (int)pos_sec / 60, (int)pos_sec % 60);
        char* dur_str = g_strdup_printf("%d:%02d", (int)dur_sec / 60, (int)dur_sec % 60);
        
        gtk_label_set_label(app->elapsed_time_label, pos_str);
        gtk_label_set_label(app->duration_label, dur_str);

        g_free(pos_str);
        g_free(dur_str);
    }

    return TRUE;
}
