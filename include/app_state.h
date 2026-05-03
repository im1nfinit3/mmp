#ifndef MMP_APP_STATE_H
#define MMP_APP_STATE_H

#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include "sqlite3.h"

typedef struct {
    char* path;
    char* title;
    char* artist;
    char* album;
    char* duration_str;
} Song;

typedef enum {
    REPEAT_OFF,
    REPEAT_ALL,
    REPEAT_ONE
} RepeatMode;

typedef struct {
    GtkWindow* window;
    GtkLabel* current_track_label;
    GtkLabel* elapsed_time_label;
    GtkLabel* duration_label;
    GtkScale* track_progress_scale;
    GtkButton* play_pause_button;
    GtkButton* shuffle_button;
    GtkButton* repeat_button;
    GtkListBox* songs_list;
    GtkListBox* recently_added_list;
    GtkListBox* albums_list;
    GtkListBox* artists_list;
    GtkListBox* queue_list;
    GtkListBox* playlist_songs_list;
    GtkSearchEntry* songs_search_entry;
    GstElement* playbin;
    GstDiscoverer* discoverer;
    char* current_file_path;
    GQueue* playlist;
    GList* current_track_node;
    GList* library;
    bool volume_muted;
    bool is_programmatic_change;
    char* selected_artist_filter;
    char* selected_album_filter;
    GtkStack* content_stack;
    GtkListBox* navigation_list;
    bool shuffle_mode;
    RepeatMode repeat_mode;
    GList* unplayed_pool;
    sqlite3* db;
    sqlite3* library_db;
    int current_playlist_id;
} MmpApp;

#endif // MMP_APP_STATE_H
