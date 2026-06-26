//! Music library — data, persistence, filtering, and scanning.
//!
//! The Library runs as a background actor thread. The main thread communicates
//! with it via `LibraryHandle` (a cheaply-cloneable channel sender).
//!
//! Mutations are fire-and-forget. Queries block briefly on a channel round-trip
//! (microseconds for in-memory HashMap + Vec operations).

use std::collections::HashMap;
use std::fmt;
use std::path::PathBuf;
use std::sync::{Arc, Mutex, mpsc};

use self::song::Song;

// ---------------------------------------------------------------------------
// Library errors
// ---------------------------------------------------------------------------

/// Errors that can occur during library operations.
#[derive(Debug)]
pub enum LibraryError {
    /// Database operation failed.
    Db(String),
    /// A database connection was not available.
    Unavailable(String),
    /// The library actor thread disconnected.
    Disconnected,
}

impl fmt::Display for LibraryError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::Db(msg) => write!(f, "{msg}"),
            Self::Unavailable(msg) => write!(f, "{msg}"),
            Self::Disconnected => write!(f, "Library actor disconnected"),
        }
    }
}

impl std::error::Error for LibraryError {}

pub mod db;
pub mod metadata;
pub mod scan;
pub mod song;

// ---------------------------------------------------------------------------
// Commands sent from the main thread to the Library actor
// ---------------------------------------------------------------------------

/// A command sent to the Library actor thread.
enum LibraryCommand {
    /// Add fully-populated songs (with metadata) to the library.
    AddSongs { songs: Vec<Song> },
    /// Get path fingerprints for songs whose cached metadata is current.
    GetMetadataCache {
        reply: mpsc::Sender<HashMap<PathBuf, db::FileFingerprint>>,
    },

    /// Create a new playlist.
    CreatePlaylist {
        name: String,
        reply: mpsc::Sender<Result<i64, LibraryError>>,
    },
    /// Delete a playlist.
    DeletePlaylist { id: i64 },
    /// Rename a playlist.
    RenamePlaylist { id: i64, name: String },
    /// Add a song (by path) to a playlist.
    AddToPlaylist {
        playlist_id: i64,
        song_path: PathBuf,
    },
    /// Remove a song (by path) from a playlist.
    RemoveFromPlaylist {
        playlist_id: i64,
        song_path: PathBuf,
    },
    /// Get all playlists.
    GetPlaylists {
        reply: mpsc::Sender<Vec<db::Playlist>>,
    },
    /// Get songs in a playlist.
    GetPlaylistSongs {
        playlist_id: i64,
        reply: mpsc::Sender<Vec<Song>>,
    },
    /// Purge all songs from the library (both database and in-memory).
    PurgeAllSongs,
    /// Re-associate playlist–song pairs after a rescan.
    ReAssociatePlaylistSongs {
        /// (playlist_id, song_path) pairs captured before the purge.
        pairs: Vec<(i64, PathBuf)>,
    },
    /// Stop the actor thread.
    Shutdown,
}

// ---------------------------------------------------------------------------
// Events emitted by the Library actor for the UI layer
// ---------------------------------------------------------------------------

/// Events the Library actor sends to the UI.
#[derive(Clone, Debug)]
pub enum LibraryEvent {
    /// Songs loaded from the database cache on startup.
    SongsLoaded { songs: Vec<Song> },
    /// New songs were added (e.g. from a scan).
    SongsAdded { songs: Vec<Song> },
    /// A directory scan has started.
    ScanStarted,
    /// A directory scan has completed.
    ScanComplete { total: usize },
    /// A playlist was created, deleted, or renamed.
    PlaylistsChanged,
    /// A non-fatal error occurred.
    Error(String),
}

// ---------------------------------------------------------------------------
// Main-thread handle to the Library actor
// ---------------------------------------------------------------------------

/// A cheaply-cloneable handle to the Library actor thread.
///
/// Send commands to the Library; query methods block briefly on the response
/// channel round-trip.
#[derive(Clone)]
pub struct LibraryHandle {
    inner: Arc<LibraryHandleInner>,
}

struct LibraryHandleInner {
    tx: mpsc::Sender<LibraryCommand>,
    worker: Mutex<Option<std::thread::JoinHandle<()>>>,
}

impl LibraryHandle {
    /// Add songs to the library (fire-and-forget).
    pub fn add_songs(&self, songs: Vec<Song>) {
        let _ = self.inner.tx.send(LibraryCommand::AddSongs { songs });
    }

    pub fn get_metadata_cache(&self) -> HashMap<PathBuf, db::FileFingerprint> {
        let (reply, rx) = mpsc::channel();
        let _ = self
            .inner
            .tx
            .send(LibraryCommand::GetMetadataCache { reply });
        rx.recv().unwrap_or_default()
    }

    /// Create a playlist. Returns the new playlist id.
    pub fn create_playlist(&self, name: &str) -> Result<i64, LibraryError> {
        let (reply, rx) = mpsc::channel();
        if self
            .inner
            .tx
            .send(LibraryCommand::CreatePlaylist {
                name: name.to_string(),
                reply,
            })
            .is_err()
        {
            return Err(LibraryError::Disconnected);
        }
        rx.recv().unwrap_or(Err(LibraryError::Disconnected))
    }

    /// Delete a playlist by id.
    pub fn delete_playlist(&self, id: i64) {
        let _ = self.inner.tx.send(LibraryCommand::DeletePlaylist { id });
    }

    /// Rename a playlist.
    pub fn rename_playlist(&self, id: i64, name: &str) {
        let _ = self.inner.tx.send(LibraryCommand::RenamePlaylist {
            id,
            name: name.to_string(),
        });
    }

    /// Add a song (by path) to a playlist.
    pub fn add_to_playlist(&self, playlist_id: i64, song_path: PathBuf) {
        let _ = self.inner.tx.send(LibraryCommand::AddToPlaylist {
            playlist_id,
            song_path,
        });
    }

    /// Remove a song (by path) from a playlist.
    pub fn remove_from_playlist(&self, playlist_id: i64, song_path: PathBuf) {
        let _ = self.inner.tx.send(LibraryCommand::RemoveFromPlaylist {
            playlist_id,
            song_path,
        });
    }

    /// Get all playlists.
    pub fn get_playlists(&self) -> Vec<db::Playlist> {
        let (reply, rx) = mpsc::channel();
        let _ = self.inner.tx.send(LibraryCommand::GetPlaylists { reply });
        rx.recv().unwrap_or_default()
    }

    /// Get songs in a playlist, in position order.
    pub fn get_playlist_songs(&self, playlist_id: i64) -> Vec<Song> {
        let (reply, rx) = mpsc::channel();
        let _ = self
            .inner
            .tx
            .send(LibraryCommand::GetPlaylistSongs { playlist_id, reply });
        rx.recv().unwrap_or_default()
    }

    /// Purge all songs from the library (fire-and-forget).
    pub fn purge_all_songs(&self) {
        let _ = self.inner.tx.send(LibraryCommand::PurgeAllSongs);
    }

    /// Re-associate playlist–song pairs after a rescan (fire-and-forget).
    pub fn reassociate_playlist_songs(&self, pairs: Vec<(i64, PathBuf)>) {
        let _ = self
            .inner
            .tx
            .send(LibraryCommand::ReAssociatePlaylistSongs { pairs });
    }
}

impl Drop for LibraryHandle {
    fn drop(&mut self) {
        if Arc::strong_count(&self.inner) != 1 {
            return;
        }

        let _ = self.inner.tx.send(LibraryCommand::Shutdown);
        if let Some(worker) = self
            .inner
            .worker
            .lock()
            .ok()
            .and_then(|mut worker| worker.take())
        {
            let _ = worker.join();
        }
    }
}

// ---------------------------------------------------------------------------
// Library actor
// ---------------------------------------------------------------------------

/// Spawn the Library actor on a background thread.
///
/// Returns a `LibraryHandle` for sending commands, and sends startup events
/// (cached songs, playlists) via `event_tx`.
pub fn spawn(event_tx: mpsc::Sender<LibraryEvent>) -> LibraryHandle {
    let (cmd_tx, cmd_rx) = mpsc::channel::<LibraryCommand>();

    let worker = std::thread::spawn(move || {
        // -- Open databases --
        let library_db = match db::open_library_db() {
            Ok(conn) => Some(conn),
            Err(err) => {
                let _ = event_tx.send(LibraryEvent::Error(format!(
                    "Failed to open library database: {err}"
                )));
                None
            }
        };
        let playlists_db = match db::open_playlists_db() {
            Ok(conn) => Some(conn),
            Err(err) => {
                let _ = event_tx.send(LibraryEvent::Error(format!(
                    "Failed to open playlists database: {err}"
                )));
                None
            }
        };

        // -- Load cached songs --
        let mut songs: Vec<Song> = library_db
            .as_ref()
            .and_then(|conn| db::get_all_songs(conn).ok())
            .unwrap_or_default();

        let mut by_path: HashMap<PathBuf, usize> = songs
            .iter()
            .enumerate()
            .map(|(i, s)| (s.path.clone(), i))
            .collect();

        // -- Load cached playlists --
        let mut playlists: Vec<db::Playlist> = playlists_db
            .as_ref()
            .and_then(|conn| db::get_playlists(conn).ok())
            .unwrap_or_default();

        if !playlists.is_empty() {
            let _ = event_tx.send(LibraryEvent::PlaylistsChanged);
        }

        // -- Notify UI of cached data --
        if !songs.is_empty() {
            let _ = event_tx.send(LibraryEvent::SongsLoaded {
                songs: songs.clone(),
            });
        }

        // -- Event loop --
        for cmd in cmd_rx {
            match cmd {
                LibraryCommand::AddSongs { songs: new_songs } => {
                    let mut metadata_save_error: Option<String> = None;
                    for song in &new_songs {
                        if let Some(conn) = &library_db
                            && let Err(err) = db::save_song(conn, song)
                            && metadata_save_error.is_none()
                        {
                            metadata_save_error =
                                Some(format!("Failed to save library metadata: {err}"));
                        }
                        if let Some(&idx) = by_path.get(&song.path) {
                            songs[idx] = song.clone();
                        } else {
                            by_path.insert(song.path.clone(), songs.len());
                            songs.push(song.clone());
                        }
                    }
                    if let Some(error) = metadata_save_error {
                        let _ = event_tx.send(LibraryEvent::Error(error));
                    }
                    if !new_songs.is_empty() {
                        let _ = event_tx.send(LibraryEvent::SongsAdded { songs: new_songs });
                    }
                }

                LibraryCommand::GetMetadataCache { reply } => {
                    let result = library_db
                        .as_ref()
                        .and_then(|conn| db::get_metadata_cache(conn).ok())
                        .unwrap_or_default();
                    let _ = reply.send(result);
                }

                LibraryCommand::CreatePlaylist { name, reply } => {
                    let result = playlists_db
                        .as_ref()
                        .ok_or_else(|| LibraryError::Unavailable("No playlists database".into()))
                        .and_then(|conn| {
                            db::create_playlist(conn, &name)
                                .map_err(|e| LibraryError::Db(e.to_string()))
                        });
                    if result.is_ok() {
                        if let Some(conn) = playlists_db.as_ref()
                            && let Ok(pls) = db::get_playlists(conn)
                        {
                            playlists = pls;
                        }
                        let _ = event_tx.send(LibraryEvent::PlaylistsChanged);
                    } else if let Err(error) = &result {
                        let _ = event_tx.send(LibraryEvent::Error(format!(
                            "Failed to create playlist: {error}"
                        )));
                    }
                    let _ = reply.send(result);
                }

                LibraryCommand::DeletePlaylist { id } => {
                    if let Some(conn) = &playlists_db {
                        match db::delete_playlist(conn, id) {
                            Ok(()) => {}
                            Err(err) => {
                                let _ = event_tx.send(LibraryEvent::Error(format!(
                                    "Failed to delete playlist: {err}"
                                )));
                            }
                        }
                        if let Ok(pls) = db::get_playlists(conn) {
                            playlists = pls;
                        }
                    } else {
                        let _ = event_tx.send(LibraryEvent::Error(String::from(
                            "Failed to delete playlist: playlists database unavailable",
                        )));
                    }
                    let _ = event_tx.send(LibraryEvent::PlaylistsChanged);
                }

                LibraryCommand::RenamePlaylist { id, name } => {
                    if let Some(conn) = &playlists_db {
                        if let Err(err) = db::rename_playlist(conn, id, &name) {
                            let _ = event_tx.send(LibraryEvent::Error(format!(
                                "Failed to rename playlist: {err}"
                            )));
                        }
                        if let Ok(pls) = db::get_playlists(conn) {
                            playlists = pls;
                        }
                    } else {
                        let _ = event_tx.send(LibraryEvent::Error(String::from(
                            "Failed to rename playlist: playlists database unavailable",
                        )));
                    }
                    let _ = event_tx.send(LibraryEvent::PlaylistsChanged);
                }

                LibraryCommand::AddToPlaylist {
                    playlist_id,
                    song_path,
                } => {
                    if let Some(conn) = &playlists_db {
                        // Look up the Song by path in our in-memory store
                        if let Some(&idx) = by_path.get(&song_path)
                            && let Err(err) =
                                db::add_song_to_playlist(conn, playlist_id, &songs[idx])
                        {
                            let _ = event_tx.send(LibraryEvent::Error(format!(
                                "Failed to add song to playlist: {err}"
                            )));
                        }
                    } else {
                        let _ = event_tx.send(LibraryEvent::Error(String::from(
                            "Failed to add song to playlist: playlists database unavailable",
                        )));
                    }
                    let _ = event_tx.send(LibraryEvent::PlaylistsChanged);
                }

                LibraryCommand::RemoveFromPlaylist {
                    playlist_id,
                    song_path,
                } => {
                    if let Some(conn) = &playlists_db {
                        if let Err(err) =
                            db::remove_song_from_playlist(conn, playlist_id, &song_path)
                        {
                            let _ = event_tx.send(LibraryEvent::Error(format!(
                                "Failed to remove song from playlist: {err}"
                            )));
                        }
                    } else {
                        let _ = event_tx.send(LibraryEvent::Error(String::from(
                            "Failed to remove song from playlist: playlists database unavailable",
                        )));
                    }
                    let _ = event_tx.send(LibraryEvent::PlaylistsChanged);
                }

                LibraryCommand::GetPlaylists { reply } => {
                    let _ = reply.send(playlists.clone());
                }

                LibraryCommand::GetPlaylistSongs { playlist_id, reply } => {
                    let result = match playlists_db.as_ref() {
                        Some(conn) => match db::get_playlist_songs(conn, playlist_id) {
                            Ok(songs) => songs,
                            Err(err) => {
                                let _ = event_tx.send(LibraryEvent::Error(format!(
                                    "Failed to load playlist songs: {err}"
                                )));
                                Vec::new()
                            }
                        },
                        None => {
                            let _ = event_tx.send(LibraryEvent::Error(String::from(
                                "Failed to load playlist songs: playlists database unavailable",
                            )));
                            Vec::new()
                        }
                    };
                    let _ = reply.send(result);
                }

                LibraryCommand::ReAssociatePlaylistSongs { pairs } => {
                    let mut re_linked = 0usize;
                    for (playlist_id, path) in &pairs {
                        if let Some(&idx) = by_path.get(path) {
                            if let Some(conn) = &playlists_db {
                                if db::add_song_to_playlist(conn, *playlist_id, &songs[idx]).is_ok()
                                {
                                    re_linked += 1;
                                }
                            }
                        }
                    }
                    if re_linked > 0 {
                        let _ = event_tx.send(LibraryEvent::PlaylistsChanged);
                    }
                }

                LibraryCommand::PurgeAllSongs => {
                    // Clear songs from library DB
                    if let Some(conn) = &library_db {
                        let _ = conn.execute_batch("DELETE FROM songs");
                    }
                    // Clear playlist_songs from playlists DB
                    // (cascade doesn't apply across separate DB files)
                    if let Some(conn) = &playlists_db {
                        let _ = conn.execute_batch("DELETE FROM playlist_songs");
                    }
                    // Clear in-memory state
                    songs.clear();
                    by_path.clear();
                    // Notify UI
                    let _ = event_tx.send(LibraryEvent::SongsLoaded {
                        songs: Vec::new(),
                    });
                    let _ = event_tx.send(LibraryEvent::PlaylistsChanged);
                }

                LibraryCommand::Shutdown => break,
            }
        }
    });

    LibraryHandle {
        inner: Arc::new(LibraryHandleInner {
            tx: cmd_tx,
            worker: Mutex::new(Some(worker)),
        }),
    }
}
