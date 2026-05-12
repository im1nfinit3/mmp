#ifndef MMP_TYPES_H
#define MMP_TYPES_H

#include <glib.h>
#include <stdbool.h>

typedef struct {
    char *path;
    char *title;
    char *artist;
    char *album;
    char *duration_str;
} Song;

typedef enum {
    REPEAT_OFF,
    REPEAT_ALL,
    REPEAT_ONE
} RepeatMode;

typedef bool (*SongFilterFunc)(Song *song, gpointer user_data);

typedef struct {
    SongFilterFunc filter;
    gpointer       user_data;
    GDestroyNotify notify;
} SongFilter;

#endif
