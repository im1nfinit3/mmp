#ifndef MMP_UI_INTERNAL_H
#define MMP_UI_INTERNAL_H

#include "mmp_ui.h"
#include "mmp_library.h"
#include "mmp_playback.h"

#define MMP_TYPE_SONG_ITEM (mmp_song_item_get_type())
G_DECLARE_FINAL_TYPE(MmpSongItem, mmp_song_item, MMP, SONG_ITEM, GObject)

struct _MmpSongItem {
    GObject parent_instance;
    Song *song;
};

struct _MmpUI {
    GtkWindow      *window;

    GtkLabel       *current_track_label;
    GtkLabel       *elapsed_time_label;
    GtkLabel       *duration_label;
    GtkScale       *track_progress_scale;
    GtkButton      *play_pause_button;
    GtkButton      *shuffle_button;
    GtkButton      *repeat_button;

    GtkListView    *song_view;
    GListStore     *song_store;
    GtkListView    *queue_view;
    GListStore     *queue_store;
    GtkListBox     *albums_list;
    GtkListBox     *artists_list;
    GtkSearchEntry *songs_search_entry;
    GtkStack       *content_stack;
    GtkListBox     *navigation_list;

    GList          *current_view_filters;
    GList          *current_view_base_list;
    bool            current_view_base_list_owned;
    bool            current_view_reverse;
    char           *selected_artist_filter;
    char           *selected_album_filter;
    char           *search_lowered_text;
    int             current_playlist_id;
    bool            is_programmatic_change;

    MmpLibrary     *library;
    MmpPlayback    *playback;

    GList          *queue_fallback_songs;
    guint           tick_timer_id;

    char           *last_playing_path;
};

void ui_update_queue(MmpUI *ui);
void ui_update_now_playing(MmpUI *ui, const char *old_path);
void ui_update_playlists(MmpUI *ui);
void ui_clear_filters(MmpUI *ui);
void ui_add_filter(MmpUI *ui, SongFilterFunc func, gpointer data, GDestroyNotify notify);
void ui_set_view(MmpUI *ui, GList *base_list, bool owned, bool reverse);
GList *ui_get_filtered_songs(MmpUI *ui);
void ui_refresh_view(MmpUI *ui);
bool search_filter_func(Song *song, gpointer user_data);
bool artist_filter_func(Song *song, gpointer user_data);
bool album_filter_func(Song *song, gpointer user_data);
void ui_update_search_lowered_text(MmpUI *ui, GtkSearchEntry *entry);

#endif
