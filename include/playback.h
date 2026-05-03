#ifndef MMP_PLAYBACK_H
#define MMP_PLAYBACK_H

#include "app_state.h"

void playback_init(MmpApp* app);
void playback_play_track(MmpApp* app, GList* node);
GList* playback_add_to_playlist(MmpApp* app, const char* path, bool play_now);
void playback_add_songs_to_playlist(MmpApp* app, GList* songs);
void playback_open_file(MmpApp* app, const char* path);
void playback_toggle_pause(MmpApp* app);
void playback_seek(MmpApp* app, double seconds);
void playback_set_volume(MmpApp* app, double volume);
void playback_set_mute(MmpApp* app, bool mute);
void playback_play_next(MmpApp* app, const char* path);
void playback_skip_next(MmpApp* app);
void playback_remove_from_playlist(MmpApp* app, GList* node);
void playback_clear_playlist(MmpApp* app);
void playback_shuffle_toggle(MmpApp* app);
void playback_repeat_toggle(MmpApp* app);
void playback_rebuild_unplayed_pool(MmpApp* app);
void playback_get_metadata(MmpApp* app, Song* song);
gboolean playback_update_ui(MmpApp* app);

#endif // MMP_PLAYBACK_H
