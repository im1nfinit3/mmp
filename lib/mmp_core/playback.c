#include "mmp_playback.h"
#include <gst/gst.h>

struct _MmpPlayback {
    GObject parent_instance;
    GstElement *playbin;
};

G_DEFINE_TYPE(MmpPlayback, mmp_playback, G_TYPE_OBJECT)

enum {
    SIGNAL_EOS,
    SIGNAL_TAG_RECEIVED,
    SIGNAL_ERROR,
    SIGNAL_STATE_CHANGED,
    N_SIGNALS
};
static guint signals[N_SIGNALS] = {0};

static void on_bus_message(GstBus *bus, GstMessage *msg, gpointer user_data)
{
    (void)bus;
    MmpPlayback *pb = MMP_PLAYBACK(user_data);

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            g_signal_emit(pb, signals[SIGNAL_EOS], 0);
            break;
        case GST_MESSAGE_TAG: {
            GstTagList *tags = NULL;
            gst_message_parse_tag(msg, &tags);
            if (tags) {
                gchar *artist = NULL, *title = NULL;
                gst_tag_list_get_string(tags, GST_TAG_ARTIST, &artist);
                gst_tag_list_get_string(tags, GST_TAG_TITLE, &title);
                g_signal_emit(pb, signals[SIGNAL_TAG_RECEIVED], 0,
                    artist ? artist : "Unknown Artist",
                    title  ? title  : "Unknown Track");
                g_free(artist);
                g_free(title);
                gst_tag_list_unref(tags);
            }
            break;
        }
        case GST_MESSAGE_ERROR: {
            GError *err = NULL;
            gchar  *debug = NULL;
            gst_message_parse_error(msg, &err, &debug);
            g_signal_emit(pb, signals[SIGNAL_ERROR], 0, err->message);
            g_error_free(err);
            g_free(debug);
            break;
        }
        default:
            break;
    }
}

static void mmp_playback_init(MmpPlayback *pb)
{
    pb->playbin = gst_element_factory_make("playbin", "player");

    GstBus *bus = gst_element_get_bus(pb->playbin);
    gst_bus_add_signal_watch(bus);
    g_signal_connect(bus, "message", G_CALLBACK(on_bus_message), pb);
    gst_object_unref(bus);
}

static void mmp_playback_finalize(GObject *obj)
{
    MmpPlayback *pb = MMP_PLAYBACK(obj);
    if (pb->playbin) {
        gst_element_set_state(pb->playbin, GST_STATE_NULL);
        gst_object_unref(pb->playbin);
    }
    G_OBJECT_CLASS(mmp_playback_parent_class)->finalize(obj);
}

static void mmp_playback_class_init(MmpPlaybackClass *klass)
{
    GObjectClass *gobj = G_OBJECT_CLASS(klass);
    gobj->finalize = mmp_playback_finalize;

    signals[SIGNAL_EOS] = g_signal_new(
        "eos", MMP_TYPE_PLAYBACK, G_SIGNAL_RUN_LAST,
        0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
    signals[SIGNAL_TAG_RECEIVED] = g_signal_new(
        "tag-received", MMP_TYPE_PLAYBACK, G_SIGNAL_RUN_LAST,
        0, NULL, NULL, NULL, G_TYPE_NONE, 2,
        G_TYPE_STRING, G_TYPE_STRING);
    signals[SIGNAL_ERROR] = g_signal_new(
        "error", MMP_TYPE_PLAYBACK, G_SIGNAL_RUN_LAST,
        0, NULL, NULL, NULL, G_TYPE_NONE, 1,
        G_TYPE_STRING);
    signals[SIGNAL_STATE_CHANGED] = g_signal_new(
        "state-changed", MMP_TYPE_PLAYBACK, G_SIGNAL_RUN_LAST,
        0, NULL, NULL, NULL, G_TYPE_NONE, 1,
        G_TYPE_BOOLEAN);
}

MmpPlayback *mmp_playback_new(void)
{
    return g_object_new(MMP_TYPE_PLAYBACK, NULL);
}

void mmp_playback_play_uri(MmpPlayback *pb, const char *uri)
{
    g_return_if_fail(MMP_IS_PLAYBACK(pb));
    gst_element_set_state(pb->playbin, GST_STATE_NULL);
    g_object_set(pb->playbin, "uri", uri, NULL);
    gst_element_set_state(pb->playbin, GST_STATE_PLAYING);
    g_signal_emit(pb, signals[SIGNAL_STATE_CHANGED], 0, TRUE);
}

void mmp_playback_toggle_pause(MmpPlayback *pb)
{
    g_return_if_fail(MMP_IS_PLAYBACK(pb));
    GstState state;
    gst_element_get_state(pb->playbin, &state, NULL, 0);
    if (state == GST_STATE_PLAYING) {
        gst_element_set_state(pb->playbin, GST_STATE_PAUSED);
        g_signal_emit(pb, signals[SIGNAL_STATE_CHANGED], 0, FALSE);
    } else {
        gst_element_set_state(pb->playbin, GST_STATE_PLAYING);
        g_signal_emit(pb, signals[SIGNAL_STATE_CHANGED], 0, TRUE);
    }
}

void mmp_playback_stop(MmpPlayback *pb)
{
    g_return_if_fail(MMP_IS_PLAYBACK(pb));
    gst_element_set_state(pb->playbin, GST_STATE_READY);
    g_signal_emit(pb, signals[SIGNAL_STATE_CHANGED], 0, FALSE);
}

void mmp_playback_seek(MmpPlayback *pb, double seconds)
{
    g_return_if_fail(MMP_IS_PLAYBACK(pb));
    gst_element_seek_simple(pb->playbin, GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
        (gint64)(seconds * GST_SECOND));
}

void mmp_playback_set_volume(MmpPlayback *pb, double volume)
{
    g_return_if_fail(MMP_IS_PLAYBACK(pb));
    g_object_set(pb->playbin, "volume", volume, NULL);
}

void mmp_playback_set_mute(MmpPlayback *pb, bool mute)
{
    g_return_if_fail(MMP_IS_PLAYBACK(pb));
    g_object_set(pb->playbin, "mute", mute, NULL);
}

bool mmp_playback_is_playing(MmpPlayback *pb)
{
    g_return_val_if_fail(MMP_IS_PLAYBACK(pb), false);
    GstState state;
    gst_element_get_state(pb->playbin, &state, NULL, 0);
    return state == GST_STATE_PLAYING;
}

double mmp_playback_get_position(MmpPlayback *pb)
{
    g_return_val_if_fail(MMP_IS_PLAYBACK(pb), 0.0);
    gint64 pos = 0;
    if (gst_element_query_position(pb->playbin, GST_FORMAT_TIME, &pos))
        return (double)pos / GST_SECOND;
    return 0.0;
}

double mmp_playback_get_duration(MmpPlayback *pb)
{
    g_return_val_if_fail(MMP_IS_PLAYBACK(pb), 0.0);
    gint64 dur = 0;
    if (gst_element_query_duration(pb->playbin, GST_FORMAT_TIME, &dur))
        return (double)dur / GST_SECOND;
    return 0.0;
}
