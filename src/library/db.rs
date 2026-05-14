//! SQLite persistence for the music library and playlists.

use rusqlite::{Connection, Result as SqlResult, params};
use std::path::{Path, PathBuf};
use std::time::UNIX_EPOCH;

use super::song::Song;

pub const CURRENT_METADATA_VERSION: i64 = 1;

/// A named playlist from the database.
#[derive(Clone, Debug)]
pub struct Playlist {
    pub id: i64,
    pub name: String,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct FileFingerprint {
    pub file_size: u64,
    pub modified_unix_ms: Option<i64>,
    pub metadata_version: i64,
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
    duration_str TEXT,
    file_size INTEGER,
    modified_unix_ms INTEGER,
    metadata_version INTEGER NOT NULL DEFAULT 0
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
    ensure_song_cache_columns(&conn)?;
    conn.execute_batch(PLAYLISTS_DDL)?;
    conn.execute_batch(PLAYLIST_SONGS_DDL)?;
    Ok(conn)
}

fn ensure_song_cache_columns(conn: &Connection) -> SqlResult<()> {
    conn.execute("ALTER TABLE songs ADD COLUMN file_size INTEGER", [])
        .ok();
    conn.execute("ALTER TABLE songs ADD COLUMN modified_unix_ms INTEGER", [])
        .ok();
    conn.execute(
        "ALTER TABLE songs ADD COLUMN metadata_version INTEGER NOT NULL DEFAULT 0",
        [],
    )
    .ok();
    Ok(())
}

// ---------------------------------------------------------------------------
// Songs (library cache)
// ---------------------------------------------------------------------------

/// Persist a song to the library database, preserving the row id by path.
pub fn save_song(conn: &Connection, song: &Song) -> SqlResult<()> {
    let fingerprint = fingerprint_for_path(&song.path);
    conn.execute(
        "INSERT INTO songs (
            path,
            title,
            artist,
            album,
            duration_str,
            file_size,
            modified_unix_ms,
            metadata_version
        )
         VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8)
         ON CONFLICT(path) DO UPDATE SET
            title = excluded.title,
            artist = excluded.artist,
            album = excluded.album,
            duration_str = excluded.duration_str,
            file_size = excluded.file_size,
            modified_unix_ms = excluded.modified_unix_ms,
            metadata_version = excluded.metadata_version",
        params![
            song.path.to_string_lossy(),
            song.title,
            song.artist,
            song.album,
            song.duration_str,
            fingerprint.map(|f| f.file_size as i64),
            fingerprint.and_then(|f| f.modified_unix_ms),
            fingerprint.map_or(0, |f| f.metadata_version),
        ],
    )?;
    Ok(())
}

pub fn fingerprint_for_path(path: &Path) -> Option<FileFingerprint> {
    let metadata = std::fs::metadata(path).ok()?;
    let modified_unix_ms = metadata.modified().ok().and_then(|modified| {
        modified
            .duration_since(UNIX_EPOCH)
            .ok()
            .and_then(|duration| i64::try_from(duration.as_millis()).ok())
    });

    Some(FileFingerprint {
        file_size: metadata.len(),
        modified_unix_ms,
        metadata_version: CURRENT_METADATA_VERSION,
    })
}

pub fn get_metadata_cache(
    conn: &Connection,
) -> SqlResult<std::collections::HashMap<PathBuf, FileFingerprint>> {
    let mut stmt = conn.prepare(
        "SELECT path, file_size, modified_unix_ms, metadata_version
         FROM songs
         WHERE file_size IS NOT NULL
           AND metadata_version = ?1
           AND COALESCE(title, '') != ''
           AND COALESCE(artist, '') != ''
           AND COALESCE(album, '') != ''
           AND COALESCE(duration_str, '') != ''",
    )?;
    let rows = stmt.query_map(params![CURRENT_METADATA_VERSION], |row| {
        let file_size: i64 = row.get(1)?;
        Ok((
            PathBuf::from(row.get::<_, String>(0)?),
            FileFingerprint {
                file_size: file_size.max(0) as u64,
                modified_unix_ms: row.get(2)?,
                metadata_version: row.get(3)?,
            },
        ))
    })?;

    let mut cache = std::collections::HashMap::new();
    for row in rows {
        let (path, fingerprint) = row?;
        cache.insert(path, fingerprint);
    }
    Ok(cache)
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
