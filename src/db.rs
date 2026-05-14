use rusqlite::{params, Connection, Result as SqlResult};
use std::path::{Path, PathBuf};

use crate::app::Song;

/// A named playlist from the database.
#[derive(Clone, Debug)]
pub struct Playlist {
    pub id: i64,
    pub name: String,
}

// ---------------------------------------------------------------------------
// Schema DDL
// ---------------------------------------------------------------------------

const SONGS_DDL: &str = "
CREATE TABLE IF NOT EXISTS songs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    path TEXT UNIQUE NOT NULL,
    title TEXT,
    artist TEXT,
    album TEXT,
    duration_str TEXT
);
";

const PLAYLISTS_DDL: &str = "
CREATE TABLE IF NOT EXISTS playlists (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL
);
";

const PLAYLIST_SONGS_DDL: &str = "
CREATE TABLE IF NOT EXISTS playlist_songs (
    playlist_id INTEGER NOT NULL,
    song_id INTEGER NOT NULL,
    position INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY (playlist_id, song_id),
    FOREIGN KEY (playlist_id) REFERENCES playlists(id) ON DELETE CASCADE,
    FOREIGN KEY (song_id) REFERENCES songs(id) ON DELETE CASCADE
);
";

// ---------------------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------------------

/// Open (or create) a database at `db_path` and ensure all tables exist.
pub fn open(path: &Path) -> SqlResult<Connection> {
    let conn = Connection::open(path)?;
    conn.execute_batch("PRAGMA journal_mode=WAL; PRAGMA foreign_keys=ON;")?;
    conn.execute_batch(SONGS_DDL)?;
    conn.execute_batch(PLAYLISTS_DDL)?;
    conn.execute_batch(PLAYLIST_SONGS_DDL)?;
    Ok(conn)
}

// ---------------------------------------------------------------------------
// Songs (library cache)
// ---------------------------------------------------------------------------

/// Persist a song to the library database (INSERT OR REPLACE by path).
pub fn save_song(conn: &Connection, song: &Song) -> SqlResult<()> {
    conn.execute(
        "INSERT OR REPLACE INTO songs (path, title, artist, album, duration_str)
         VALUES (?1, ?2, ?3, ?4, ?5)",
        params![
            song.path.to_string_lossy(),
            song.title,
            song.artist,
            song.album,
            song.duration_str,
        ],
    )?;
    Ok(())
}

/// Load every cached song from the library database.
pub fn get_all_songs(conn: &Connection) -> SqlResult<Vec<Song>> {
    let mut stmt = conn.prepare("SELECT path, title, artist, album, duration_str FROM songs")?;
    let rows = stmt.query_map([], |row| {
        Ok(Song {
            path: PathBuf::from(row.get::<_, String>(0)?),
            title: row.get::<_, Option<String>>(1)?.unwrap_or_default(),
            artist: row.get::<_, Option<String>>(2)?.unwrap_or_default(),
            album: row.get::<_, Option<String>>(3)?.unwrap_or_default(),
            duration_str: row.get::<_, Option<String>>(4)?.unwrap_or_default(),
        })
    })?;

    let mut songs = Vec::new();
    for row in rows {
        songs.push(row?);
    }
    Ok(songs)
}

/// Look up a song's database id by its file path.
pub fn get_song_id(conn: &Connection, path: &Path) -> SqlResult<Option<i64>> {
    let mut stmt = conn.prepare("SELECT id FROM songs WHERE path = ?1")?;
    let mut rows = stmt.query_map(params![path.to_string_lossy()], |row| row.get(0))?;
    match rows.next() {
        Some(Ok(id)) => Ok(Some(id)),
        Some(Err(e)) => Err(e),
        None => Ok(None),
    }
}

/// Get a song's database id (inserting it if missing).
fn get_or_insert_song_id(conn: &Connection, song: &Song) -> SqlResult<i64> {
    if let Some(id) = get_song_id(conn, &song.path)? {
        return Ok(id);
    }
    save_song(conn, song)?;
    Ok(conn.last_insert_rowid())
}

// ---------------------------------------------------------------------------
// Playlists
// ---------------------------------------------------------------------------

/// Create a new playlist, returning its id.
pub fn create_playlist(conn: &Connection, name: &str) -> SqlResult<i64> {
    conn.execute("INSERT INTO playlists (name) VALUES (?1)", params![name])?;
    Ok(conn.last_insert_rowid())
}

/// Delete a playlist (cascade deletes its song entries).
pub fn delete_playlist(conn: &Connection, playlist_id: i64) -> SqlResult<()> {
    conn.execute("DELETE FROM playlists WHERE id = ?1", params![playlist_id])?;
    Ok(())
}

/// Rename a playlist.
pub fn rename_playlist(conn: &Connection, playlist_id: i64, new_name: &str) -> SqlResult<()> {
    conn.execute(
        "UPDATE playlists SET name = ?1 WHERE id = ?2",
        params![new_name, playlist_id],
    )?;
    Ok(())
}

/// List all playlists, ordered by name.
pub fn get_playlists(conn: &Connection) -> SqlResult<Vec<Playlist>> {
    let mut stmt = conn.prepare("SELECT id, name FROM playlists ORDER BY name")?;
    let rows = stmt.query_map([], |row| {
        Ok(Playlist {
            id: row.get(0)?,
            name: row.get(1)?,
        })
    })?;

    let mut playlists = Vec::new();
    for row in rows {
        playlists.push(row?);
    }
    Ok(playlists)
}

/// Add a song to a playlist. The song is inserted into `songs` if not already present.
pub fn add_song_to_playlist(conn: &Connection, playlist_id: i64, song: &Song) -> SqlResult<()> {
    let song_id = get_or_insert_song_id(conn, song)?;

    // Find the max position for this playlist
    let max_pos: i64 = conn
        .query_row(
            "SELECT COALESCE(MAX(position), -1) FROM playlist_songs WHERE playlist_id = ?1",
            params![playlist_id],
            |row| row.get(0),
        )
        .unwrap_or(-1);

    conn.execute(
        "INSERT OR IGNORE INTO playlist_songs (playlist_id, song_id, position) VALUES (?1, ?2, ?3)",
        params![playlist_id, song_id, max_pos + 1],
    )?;
    Ok(())
}

/// Remove a song from a playlist by path.
pub fn remove_song_from_playlist(
    conn: &Connection,
    playlist_id: i64,
    song_path: &Path,
) -> SqlResult<()> {
    conn.execute(
        "DELETE FROM playlist_songs
         WHERE playlist_id = ?1
           AND song_id = (SELECT id FROM songs WHERE path = ?2)",
        params![playlist_id, song_path.to_string_lossy()],
    )?;
    Ok(())
}

/// Load all songs in a playlist, in position order.
pub fn get_playlist_songs(conn: &Connection, playlist_id: i64) -> SqlResult<Vec<Song>> {
    let mut stmt = conn.prepare(
        "SELECT s.path, s.title, s.artist, s.album, s.duration_str
         FROM songs s
         JOIN playlist_songs ps ON s.id = ps.song_id
         WHERE ps.playlist_id = ?1
         ORDER BY ps.position",
    )?;

    let rows = stmt.query_map(params![playlist_id], |row| {
        Ok(Song {
            path: PathBuf::from(row.get::<_, String>(0)?),
            title: row.get::<_, Option<String>>(1)?.unwrap_or_default(),
            artist: row.get::<_, Option<String>>(2)?.unwrap_or_default(),
            album: row.get::<_, Option<String>>(3)?.unwrap_or_default(),
            duration_str: row.get::<_, Option<String>>(4)?.unwrap_or_default(),
        })
    })?;

    let mut songs = Vec::new();
    for row in rows {
        songs.push(row?);
    }
    Ok(songs)
}

// ---------------------------------------------------------------------------
// Helpers for data-dir layout
// ---------------------------------------------------------------------------

/// Returns `~/.config/mmp/`, creating it if needed.
pub fn config_dir() -> PathBuf {
    let dir = dirs::config_dir()
        .unwrap_or_else(|| PathBuf::from("."))
        .join("mmp");
    std::fs::create_dir_all(&dir).ok();
    dir
}

/// Open the library database at `~/.config/mmp/library.db`.
pub fn open_library_db() -> SqlResult<Connection> {
    open(&config_dir().join("library.db"))
}

/// Open the playlists database at `~/.config/mmp/playlists.db`.
pub fn open_playlists_db() -> SqlResult<Connection> {
    open(&config_dir().join("playlists.db"))
}
