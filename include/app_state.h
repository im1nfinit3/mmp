#ifndef MMP_APP_STATE_H
#define MMP_APP_STATE_H

#include <gtk/gtk.h>
#include <gst/gst.h>

typedef struct {
    char* path;
    char* title;
    char* artist;
    char* album;
} Song;

typedef struct {
    GtkWindow* window;
    GtkLabel* current_track_label;
    GtkLabel* elapsed_time_label;
    GtkLabel* duration_label;
    GtkScale* track_progress_scale;
    GtkButton* play_pause_button;
    GtkListBox* songs_list;
    GtkListBox* recently_added_list;
    GtkListBox* albums_list;
    GtkListBox* artists_list;
    GtkListBox* queue_list;
    GtkSearchEntry* songs_search_entry;
    GstElement* playbin;
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
} MmpApp;

#endif // MMP_APP_STATE_H
