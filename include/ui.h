#ifndef MMP_UI_H
#define MMP_UI_H

#include <gtk/gtk.h>
#include "app_state.h"

void app_activate_cb(GtkApplication* app);
void app_open_cb(GtkApplication* app, GFile** files, int n_files, const char* hint, gpointer user_data);
void ui_update_queue(MmpApp* app);
void ui_update_playlists(MmpApp* app);
void ui_refresh_library(MmpApp* app);
GtkWidget* create_song_row_box(Song* song);

#endif // MMP_UI_H
