#ifndef MMP_UI_CALLBACKS_H
#define MMP_UI_CALLBACKS_H

#include <gtk/gtk.h>
#include "app_state.h"

typedef struct {
    GtkStack* stack;
    GtkWidget* recently_added_row;
    GtkWidget* albums_row;
    GtkWidget* artists_row;
    GtkWidget* songs_row;
} LibraryNavRows;

extern MmpApp* mmp_app;

// Callbacks
void queue_drag_begin_cb(GtkDragSource* source, GdkDrag* drag, gpointer user_data);
gboolean queue_drop_cb(GtkDropTarget* target, const GValue* value, double x, double y, gpointer user_data);
void song_row_secondary_click_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer user_data);
void queue_row_secondary_click_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer user_data);
gboolean filter_albums_cb(GtkListBoxRow* row, gpointer user_data);
void search_changed_cb(GtkSearchEntry* entry, gpointer user_data);
void artist_row_activated_cb(GtkListBox* list, GtkListBoxRow* row, gpointer user_data);
void album_row_activated_cb(GtkListBox* list, GtkListBoxRow* row, gpointer user_data);
void song_row_activated_cb(GtkListBox* list, GtkListBoxRow* row, gpointer user_data);
void queue_row_activated_cb(GtkListBox* list, GtkListBoxRow* row, gpointer user_data);
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
gboolean on_drop_cb(GtkDropTarget* target, const GValue* value, double x, double y, gpointer user_data);

#endif // MMP_UI_CALLBACKS_H
