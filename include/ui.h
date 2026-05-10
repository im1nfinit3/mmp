#ifndef MMP_UI_H
#define MMP_UI_H

#include <gtk/gtk.h>
#include "app_state.h"

#define MMP_TYPE_SONG_ITEM (mmp_song_item_get_type())
G_DECLARE_FINAL_TYPE(MmpSongItem, mmp_song_item, MMP, SONG_ITEM, GObject)

struct _MmpSongItem {
    GObject parent_instance;
    Song* song;
};

void app_activate_cb(GtkApplication* app);
void app_open_cb(GtkApplication* app, GFile** files, int n_files, const char* hint, gpointer user_data);
void ui_update_queue(MmpApp* app);
void ui_update_playlists(MmpApp* app);
void ui_clear_filters(MmpApp* app);
void ui_add_filter(MmpApp* app, SongFilterFunc func, gpointer data, GDestroyNotify notify);
void ui_set_view(MmpApp* app, GList* base_list, bool owned, bool reverse);
GList* ui_get_filtered_songs(MmpApp* app);
void ui_refresh_view(MmpApp* app);
void ui_update_now_playing(MmpApp* app, const char* old_path);
bool search_filter_func(Song* song, gpointer user_data);
bool artist_filter_func(Song* song, gpointer user_data);
bool album_filter_func(Song* song, gpointer user_data);
void free_song(Song* song);
void ui_update_search_lowered_text(MmpApp* app, GtkSearchEntry* entry);

#endif // MMP_UI_H
