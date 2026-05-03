#include "database.h"
#include <stdio.h>
#include <string.h>

static bool execute_sql(sqlite3* db, const char* sql) {
    char* err_msg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, 0, &err_msg);
    if (rc != SQLITE_OK) {
        g_printerr("SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}

bool db_init(const char* db_path, sqlite3** db_out) {
    int rc = sqlite3_open(db_path, db_out);
    if (rc != SQLITE_OK) {
        g_printerr("Cannot open database: %s\n", sqlite3_errmsg(*db_out));
        sqlite3_close(*db_out);
        return false;
    }

    const char* schema = 
        "CREATE TABLE IF NOT EXISTS playlists ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    name TEXT UNIQUE NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS songs ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    path TEXT UNIQUE NOT NULL,"
        "    title TEXT,"
        "    artist TEXT,"
        "    album TEXT,"
        "    duration_str TEXT"
        ");"
        "CREATE TABLE IF NOT EXISTS playlist_songs ("
        "    playlist_id INTEGER,"
        "    song_id INTEGER,"
        "    position INTEGER,"
        "    PRIMARY KEY (playlist_id, song_id),"
        "    FOREIGN KEY(playlist_id) REFERENCES playlists(id) ON DELETE CASCADE,"
        "    FOREIGN KEY(song_id) REFERENCES songs(id) ON DELETE CASCADE"
        ");";

    return execute_sql(*db_out, schema);
}

void db_close(sqlite3* db) {
    if (db) {
        sqlite3_close(db);
    }
}

bool db_create_playlist(sqlite3* db, const char* name, int* playlist_id_out) {
    const char* sql = "INSERT INTO playlists (name) VALUES (?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    if (playlist_id_out) {
        *playlist_id_out = (int)sqlite3_last_insert_rowid(db);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool db_delete_playlist(sqlite3* db, int playlist_id) {
    const char* sql = "DELETE FROM playlists WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, playlist_id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_rename_playlist(sqlite3* db, int playlist_id, const char* new_name) {
    const char* sql = "UPDATE playlists SET name = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, new_name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, playlist_id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

static int get_or_insert_song(sqlite3* db, const Song* song) {
    const char* select_sql = "SELECT id FROM songs WHERE path = ?;";
    sqlite3_stmt* select_stmt;
    if (sqlite3_prepare_v2(db, select_sql, -1, &select_stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(select_stmt, 1, song->path, -1, SQLITE_STATIC);

    int song_id = -1;
    if (sqlite3_step(select_stmt) == SQLITE_ROW) {
        song_id = sqlite3_column_int(select_stmt, 0);
    }
    sqlite3_finalize(select_stmt);

    if (song_id != -1) return song_id;

    const char* insert_sql = "INSERT INTO songs (path, title, artist, album) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* insert_stmt;
    if (sqlite3_prepare_v2(db, insert_sql, -1, &insert_stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(insert_stmt, 1, song->path, -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 2, song->title, -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 3, song->artist, -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 4, song->album, -1, SQLITE_STATIC);

    if (sqlite3_step(insert_stmt) == SQLITE_DONE) {
        song_id = (int)sqlite3_last_insert_rowid(db);
    }
    sqlite3_finalize(insert_stmt);

    return song_id;
}

bool db_add_song_to_playlist(sqlite3* db, int playlist_id, const Song* song) {
    int song_id = get_or_insert_song(db, song);
    if (song_id == -1) return false;

    const char* pos_sql = "SELECT COALESCE(MAX(position), 0) + 1 FROM playlist_songs WHERE playlist_id = ?;";
    sqlite3_stmt* pos_stmt;
    if (sqlite3_prepare_v2(db, pos_sql, -1, &pos_stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(pos_stmt, 1, playlist_id);
    int position = 1;
    if (sqlite3_step(pos_stmt) == SQLITE_ROW) {
        position = sqlite3_column_int(pos_stmt, 0);
    }
    sqlite3_finalize(pos_stmt);

    const char* sql = "INSERT OR IGNORE INTO playlist_songs (playlist_id, song_id, position) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, playlist_id);
    sqlite3_bind_int(stmt, 2, song_id);
    sqlite3_bind_int(stmt, 3, position);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_remove_song_from_playlist(sqlite3* db, int playlist_id, const char* song_path) {
    const char* sql = "DELETE FROM playlist_songs WHERE playlist_id = ? AND song_id = (SELECT id FROM songs WHERE path = ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, playlist_id);
    sqlite3_bind_text(stmt, 2, song_path, -1, SQLITE_STATIC);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

GList* db_get_playlists(sqlite3* db) {
    const char* sql = "SELECT id, name FROM playlists ORDER BY name;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return NULL;

    GList* list = NULL;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Playlist* p = g_new0(Playlist, 1);
        p->id = sqlite3_column_int(stmt, 0);
        p->name = g_strdup((const char*)sqlite3_column_text(stmt, 1));
        list = g_list_append(list, p);
    }
    sqlite3_finalize(stmt);
    return list;
}

GList* db_get_playlist_songs(sqlite3* db, int playlist_id) {
    const char* sql = 
        "SELECT s.path, s.title, s.artist, s.album, s.duration_str "
        "FROM songs s "
        "JOIN playlist_songs ps ON s.id = ps.song_id "
        "WHERE ps.playlist_id = ? "
        "ORDER BY ps.position;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return NULL;

    sqlite3_bind_int(stmt, 1, playlist_id);

    GList* list = NULL;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Song* s = g_new0(Song, 1);
        s->path = g_strdup((const char*)sqlite3_column_text(stmt, 0));
        s->title = g_strdup((const char*)sqlite3_column_text(stmt, 1));
        s->artist = g_strdup((const char*)sqlite3_column_text(stmt, 2));
        s->album = g_strdup((const char*)sqlite3_column_text(stmt, 3));
        const char* dur = (const char*)sqlite3_column_text(stmt, 4);
        if (dur) s->duration_str = g_strdup(dur);
        list = g_list_append(list, s);
    }
    sqlite3_finalize(stmt);
    return list;
}

bool db_save_song(sqlite3* db, const Song* song) {
    const char* sql = "INSERT OR REPLACE INTO songs (path, title, artist, album, duration_str) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, song->path, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, song->title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, song->artist, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, song->album, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, song->duration_str, -1, SQLITE_STATIC);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

GList* db_get_all_songs(sqlite3* db) {
    const char* sql = "SELECT path, title, artist, album, duration_str FROM songs;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return NULL;

    GList* list = NULL;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Song* s = g_new0(Song, 1);
        s->path = g_strdup((const char*)sqlite3_column_text(stmt, 0));
        s->title = g_strdup((const char*)sqlite3_column_text(stmt, 1));
        s->artist = g_strdup((const char*)sqlite3_column_text(stmt, 2));
        s->album = g_strdup((const char*)sqlite3_column_text(stmt, 3));
        const char* dur = (const char*)sqlite3_column_text(stmt, 4);
        if (dur) s->duration_str = g_strdup(dur);
        list = g_list_append(list, s);
    }
    sqlite3_finalize(stmt);
    return list;
}

void free_playlist(Playlist* p) {
    if (p) {
        g_free(p->name);
        g_free(p);
    }
}

void free_song_list(GList* songs) {
    g_list_free_full(songs, (GDestroyNotify)g_free); // This is not quite right as it doesn't free the internal fields
}
