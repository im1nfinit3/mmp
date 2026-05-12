#ifndef MMP_PLAYBACK_H
#define MMP_PLAYBACK_H

#include <glib-object.h>

typedef enum {
    MMP_PLAYBACK_STOPPED,
    MMP_PLAYBACK_PLAYING,
    MMP_PLAYBACK_PAUSED
} MmpPlaybackState;

#define MMP_TYPE_PLAYBACK (mmp_playback_get_type())
G_DECLARE_FINAL_TYPE(MmpPlayback, mmp_playback, MMP, PLAYBACK, GObject)

MmpPlayback *mmp_playback_new(void);

void mmp_playback_play_uri     (MmpPlayback *pb, const char *uri);
void mmp_playback_toggle_pause (MmpPlayback *pb);
void mmp_playback_stop         (MmpPlayback *pb);
void mmp_playback_seek         (MmpPlayback *pb, double seconds);
void mmp_playback_set_volume   (MmpPlayback *pb, double volume);
void mmp_playback_set_mute     (MmpPlayback *pb, bool mute);

bool   mmp_playback_is_playing  (MmpPlayback *pb);
double mmp_playback_get_position(MmpPlayback *pb);
double mmp_playback_get_duration(MmpPlayback *pb);

#endif
