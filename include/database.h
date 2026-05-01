#ifndef MMP_DATABASE_H
#define MMP_DATABASE_H

#include "app_state.h"
#include <stdbool.h>

typedef struct {
    int id;
    char* name;
} Playlist;

bool db_init(const char* db_path, sqlite3** db_out);
void db_close(sqlite3* db);

bool db_create_playlist(sqlite3* db, const char* name, int* playlist_id_out);
bool db_delete_playlist(sqlite3* db, int playlist_id);
bool db_rename_playlist(sqlite3* db, int playlist_id, const char* new_name);
bool db_add_song_to_playlist(sqlite3* db, int playlist_id, const Song* song);
bool db_remove_song_from_playlist(sqlite3* db, int playlist_id, const char* song_path);

GList* db_get_playlists(sqlite3* db); // List of Playlist*
GList* db_get_playlist_songs(sqlite3* db, int playlist_id); // List of Song*

void free_playlist(Playlist* p);
void free_song_list(GList* songs);

#endif // MMP_DATABASE_H
