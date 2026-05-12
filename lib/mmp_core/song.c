#include "mmp_library.h"

void free_song(Song *song)
{
    if (!song) return;
    g_free(song->path);
    g_free(song->title);
    g_free(song->artist);
    g_free(song->album);
    g_free(song->duration_str);
    g_free(song);
}

Song *mmp_song_copy(const Song *song)
{
    if (!song) return NULL;
    Song *copy = g_new0(Song, 1);
    copy->path         = g_strdup(song->path);
    copy->title        = g_strdup(song->title);
    copy->artist       = g_strdup(song->artist);
    copy->album        = g_strdup(song->album);
    copy->duration_str = g_strdup(song->duration_str);
    return copy;
}

GType mmp_song_get_type(void)
{
    static GType type = 0;
    if (G_UNLIKELY(!type)) {
        type = g_boxed_type_register_static(
            "MmpSong",
            (GBoxedCopyFunc)mmp_song_copy,
            (GBoxedFreeFunc)free_song);
    }
    return type;
}
