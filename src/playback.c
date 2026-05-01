#include "playback.h"
#include "ui.h"
#include <gst/gst.h>

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
        case GST_MESSAGE_EOS:
            if (app->current_track_node && app->current_track_node->next) {
                playback_play_track(app, app->current_track_node->next);
                ui_update_queue(app);
            } else {
                gst_element_set_state(app->playbin, GST_STATE_READY);
                gtk_button_set_icon_name(app->play_pause_button, "media-playback-start-symbolic");
            }
            break;
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

    GstBus* bus = gst_element_get_bus(app->playbin);
    gst_bus_add_signal_watch(bus);
    g_signal_connect(bus, "message", G_CALLBACK(playbin_bus_message_cb), app);
    gst_object_unref(bus);
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
}

void playback_add_to_playlist(MmpApp* app, const char* path, bool play_now) {
    if (path == NULL) return;
    
    g_queue_push_tail(app->playlist, g_strdup(path));
    
    if (play_now || app->current_track_node == NULL) {
        playback_play_track(app, g_queue_peek_tail_link(app->playlist));
    }
    ui_update_queue(app);
}

void playback_open_file(MmpApp* app, const char* path) {
    // Clear playlist and play this file
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
    if (app->current_track_node) {
        g_queue_insert_after(app->playlist, app->current_track_node, path_copy);
    } else {
        g_queue_push_head(app->playlist, path_copy);
    }
    ui_update_queue(app);
}

void playback_remove_from_playlist(MmpApp* app, GList* node) {
    if (node == NULL) return;
    
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
    GList* l = app->playlist->head;
    while (l) {
        GList* next = l->next;
        if (l != app->current_track_node) {
            g_free(l->data);
            g_queue_delete_link(app->playlist, l);
        }
        l = next;
    }
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
