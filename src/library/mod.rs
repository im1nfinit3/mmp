//! Music library — data, persistence, filtering, and scanning.
//!
//! The Library runs as a background actor thread. The main thread communicates
//! with it via `LibraryHandle` (a cheaply-cloneable channel sender).
//!
//! Mutations are fire-and-forget. Queries block briefly on a channel round-trip
//! (microseconds for in-memory HashMap + Vec operations).

use std::collections::HashMap;
use std::path::PathBuf;
use std::sync::mpsc;

use self::song::Song;

pub mod db;
pub mod filter;
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
    /// Query songs matching a filter predicate.
    GetSongs {
        filter: filter::FilterFn,
        reply: mpsc::Sender<Vec<Song>>,
    },
    /// Get all songs.
    GetAllSongs { reply: mpsc::Sender<Vec<Song>> },
    /// Get path fingerprints for songs whose cached metadata is current.
    GetMetadataCache {
        reply: mpsc::Sender<HashMap<PathBuf, db::FileFingerprint>>,
    },
    /// Get unique artist names.
    GetUniqueArtists { reply: mpsc::Sender<Vec<String>> },
    /// Get unique album names.
    GetUniqueAlbums { reply: mpsc::Sender<Vec<String>> },
    /// Create a new playlist.
    CreatePlaylist {
        name: String,
        reply: mpsc::Sender<Result<i64, String>>,
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
    /// Get all playlists.
    GetPlaylists {
        reply: mpsc::Sender<Vec<db::Playlist>>,
    },
    /// Get songs in a playlist.
    GetPlaylistSongs {
        playlist_id: i64,
        reply: mpsc::Sender<Vec<Song>>,
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
    tx: mpsc::Sender<LibraryCommand>,
}

impl LibraryHandle {
    /// Add songs to the library (fire-and-forget).
    pub fn add_songs(&self, songs: Vec<Song>) {
        let _ = self.tx.send(LibraryCommand::AddSongs { songs });
    }

    /// Get all songs matching a filter predicate.
    ///
    /// Blocks until the Library actor responds (microseconds).
    pub fn get_songs(&self, filter: filter::FilterFn) -> Vec<Song> {
        let (reply, rx) = mpsc::channel();
        let _ = self.tx.send(LibraryCommand::GetSongs { filter, reply });
        rx.recv().unwrap_or_default()
    }

    /// Get every song in the library.
    pub fn get_all_songs(&self) -> Vec<Song> {
        let (reply, rx) = mpsc::channel();
        let _ = self.tx.send(LibraryCommand::GetAllSongs { reply });
        rx.recv().unwrap_or_default()
    }

    pub fn get_metadata_cache(&self) -> HashMap<PathBuf, db::FileFingerprint> {
        let (reply, rx) = mpsc::channel();
        let _ = self.tx.send(LibraryCommand::GetMetadataCache { reply });
        rx.recv().unwrap_or_default()
    }

    /// Get sorted unique artist names.
    pub fn get_unique_artists(&self) -> Vec<String> {
        let (reply, rx) = mpsc::channel();
        let _ = self.tx.send(LibraryCommand::GetUniqueArtists { reply });
        rx.recv().unwrap_or_default()
    }

    /// Get sorted unique album names.
    pub fn get_unique_albums(&self) -> Vec<String> {
        let (reply, rx) = mpsc::channel();
        let _ = self.tx.send(LibraryCommand::GetUniqueAlbums { reply });
        rx.recv().unwrap_or_default()
    }

    /// Create a playlist. Returns the new playlist id.
    pub fn create_playlist(&self, name: &str) -> Result<i64, String> {
        let (reply, rx) = mpsc::channel();
        let _ = self.tx.send(LibraryCommand::CreatePlaylist {
            name: name.to_string(),
            reply,
        });
        rx.recv()
            .unwrap_or(Err("Library actor disconnected".into()))
    }

    /// Delete a playlist by id.
    pub fn delete_playlist(&self, id: i64) {
        let _ = self.tx.send(LibraryCommand::DeletePlaylist { id });
    }

    /// Rename a playlist.
    pub fn rename_playlist(&self, id: i64, name: &str) {
        let _ = self.tx.send(LibraryCommand::RenamePlaylist {
            id,
            name: name.to_string(),
        });
    }

    /// Add a song (by path) to a playlist.
    pub fn add_to_playlist(&self, playlist_id: i64, song_path: PathBuf) {
        let _ = self.tx.send(LibraryCommand::AddToPlaylist {
            playlist_id,
            song_path,
        });
    }

    /// Get all playlists.
    pub fn get_playlists(&self) -> Vec<db::Playlist> {
        let (reply, rx) = mpsc::channel();
        let _ = self.tx.send(LibraryCommand::GetPlaylists { reply });
        rx.recv().unwrap_or_default()
    }

    /// Get songs in a playlist, in position order.
    pub fn get_playlist_songs(&self, playlist_id: i64) -> Vec<Song> {
        let (reply, rx) = mpsc::channel();
        let _ = self
            .tx
            .send(LibraryCommand::GetPlaylistSongs { playlist_id, reply });
        rx.recv().unwrap_or_default()
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

    std::thread::spawn(move || {
        // -- Open databases --
        let library_db = db::open_library_db().ok();
        let playlists_db = db::open_playlists_db().ok();

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
                    for song in &new_songs {
                        if let Some(conn) = &library_db {
                            if let Err(err) = db::save_song(conn, song) {
                                eprintln!(
                                    "Failed to save metadata for {}: {:?}",
                                    song.path.display(),
                                    err
                                );
                            }
                        }
                        if let Some(&idx) = by_path.get(&song.path) {
                            songs[idx] = song.clone();
                        } else {
                            by_path.insert(song.path.clone(), songs.len());
                            songs.push(song.clone());
                        }
                    }
                    if !new_songs.is_empty() {
                        let _ = event_tx.send(LibraryEvent::SongsAdded { songs: new_songs });
                    }
                }

                LibraryCommand::GetSongs { filter, reply } => {
                    let result: Vec<Song> = songs.iter().filter(|s| filter(s)).cloned().collect();
                    let _ = reply.send(result);
                }

                LibraryCommand::GetAllSongs { reply } => {
                    let _ = reply.send(songs.clone());
                }

                LibraryCommand::GetMetadataCache { reply } => {
                    let result = library_db
                        .as_ref()
                        .and_then(|conn| db::get_metadata_cache(conn).ok())
                        .unwrap_or_default();
                    let _ = reply.send(result);
                }

                LibraryCommand::GetUniqueArtists { reply } => {
                    let result = filter::unique_artists(&songs);
                    let _ = reply.send(result);
                }

                LibraryCommand::GetUniqueAlbums { reply } => {
                    let result = filter::unique_albums(&songs);
                    let _ = reply.send(result);
                }

                LibraryCommand::CreatePlaylist { name, reply } => {
                    let result = playlists_db
                        .as_ref()
                        .ok_or("No playlists database".to_string())
                        .and_then(|conn| {
                            db::create_playlist(conn, &name).map_err(|e| e.to_string())
                        });
                    if result.is_ok() {
                        if let Some(conn) = playlists_db.as_ref()
                            && let Ok(pls) = db::get_playlists(conn)
                        {
                            playlists = pls;
                        }
                        let _ = event_tx.send(LibraryEvent::PlaylistsChanged);
                    }
                    let _ = reply.send(result);
                }

                LibraryCommand::DeletePlaylist { id } => {
                    if let Some(conn) = &playlists_db {
                        let _ = db::delete_playlist(conn, id);
                        if let Ok(pls) = db::get_playlists(conn) {
                            playlists = pls;
                        }
                    }
                    let _ = event_tx.send(LibraryEvent::PlaylistsChanged);
                }

                LibraryCommand::RenamePlaylist { id, name } => {
                    if let Some(conn) = &playlists_db {
                        let _ = db::rename_playlist(conn, id, &name);
                        if let Ok(pls) = db::get_playlists(conn) {
                            playlists = pls;
                        }
                    }
                    let _ = event_tx.send(LibraryEvent::PlaylistsChanged);
                }

                LibraryCommand::AddToPlaylist {
                    playlist_id,
                    song_path,
                } => {
                    if let Some(conn) = &playlists_db {
                        // Look up the Song by path in our in-memory store
                        if let Some(&idx) = by_path.get(&song_path) {
                            let _ = db::add_song_to_playlist(conn, playlist_id, &songs[idx]);
                        }
                    }
                }

                LibraryCommand::GetPlaylists { reply } => {
                    let _ = reply.send(playlists.clone());
                }

                LibraryCommand::GetPlaylistSongs { playlist_id, reply } => {
                    let result = playlists_db
                        .as_ref()
                        .and_then(|conn| db::get_playlist_songs(conn, playlist_id).ok())
                        .unwrap_or_default();
                    let _ = reply.send(result);
                }

                LibraryCommand::Shutdown => break,
            }
        }
    });

    LibraryHandle { tx: cmd_tx }
}
