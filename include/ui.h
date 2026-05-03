#ifndef MMP_UI_H
#define MMP_UI_H

#include <gtk/gtk.h>
#include "app_state.h"

void app_activate_cb(GtkApplication* app);
void app_open_cb(GtkApplication* app, GFile** files, int n_files, const char* hint, gpointer user_data);
void ui_update_queue(MmpApp* app);
void ui_update_playlists(MmpApp* app);
void ui_clear_filters(MmpApp* app);
void ui_add_filter(MmpApp* app, SongFilterFunc func, gpointer data, GDestroyNotify notify);
void ui_set_view(MmpApp* app, GList* base_list, bool owned, bool reverse);
void ui_refresh_view_list(MmpApp* app, GtkListBox* list, GList* base_list, bool reverse);
GList* ui_get_filtered_songs(MmpApp* app);
void ui_refresh_view(MmpApp* app);
bool search_filter_func(Song* song, gpointer user_data);
bool artist_filter_func(Song* song, gpointer user_data);
bool album_filter_func(Song* song, gpointer user_data);
GtkWidget* create_song_row_box(Song* song);
void free_song(Song* song);
void ui_add_song_to_list(MmpApp* app, GtkListBox* list, Song* song, bool prepend, bool own_song);
void ui_populate_songs(MmpApp* app, GList* songs, bool own_songs);

#endif // MMP_UI_H
