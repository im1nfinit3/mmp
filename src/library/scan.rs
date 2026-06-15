//! Directory scanner with Rust-native metadata extraction.
//!
//! Two-stage pipeline:
//! 1. Background thread: walkdir traversal -> path batches sent via `mpsc`
//! 2. Worker thread: metadata extraction -> `Song` batches sent to the library

use std::path::PathBuf;
use std::sync::mpsc;

use crate::library::{LibraryEvent, LibraryHandle, db};
use crate::library::{metadata, song::Song};

const WALK_BATCH_SIZE: usize = 200;
const SONG_BATCH_SIZE: usize = 25;

/// Start an async directory scan.
///
/// If `library_folder` is `None`, the platform default audio directory is used.
pub fn start_scan(
    library_handle: LibraryHandle,
    event_tx: mpsc::Sender<LibraryEvent>,
    library_folder: Option<PathBuf>,
) {
    let _ = event_tx.send(LibraryEvent::ScanStarted);
    let metadata_cache = library_handle.get_metadata_cache();

    let (path_tx, path_rx) = mpsc::channel::<Vec<PathBuf>>();

    std::thread::spawn(move || {
        let music_dir = library_folder
            .or_else(|| dirs::audio_dir())
            .unwrap_or_else(|| {
                dirs::home_dir()
                    .unwrap_or_else(|| PathBuf::from("."))
                    .join("Music")
            });

        let mut batch = Vec::with_capacity(WALK_BATCH_SIZE);

        for entry in walkdir::WalkDir::new(&music_dir)
            .follow_links(true)
            .into_iter()
            .filter_map(|entry| entry.ok())
        {
            let path = entry.path();
            if !path.is_file() || !metadata::is_supported_audio_path(path) {
                continue;
            }

            if let Some(fingerprint) = db::fingerprint_for_path(path)
                && metadata_cache.get(path) == Some(&fingerprint)
            {
                continue;
            }

            batch.push(path.to_path_buf());

            if batch.len() >= WALK_BATCH_SIZE {
                if path_tx.send(std::mem::take(&mut batch)).is_err() {
                    return;
                }
                batch = Vec::with_capacity(WALK_BATCH_SIZE);
            }
        }

        if !batch.is_empty() {
            let _ = path_tx.send(batch);
        }

        let _ = path_tx.send(Vec::new());
    });

    std::thread::spawn(move || {
        let mut songs = Vec::with_capacity(SONG_BATCH_SIZE);
        let mut total = 0usize;

        while let Ok(batch) = path_rx.recv() {
            if batch.is_empty() {
                break;
            }

            for path in batch {
                match metadata::read_track_metadata(&path) {
                    Ok(track_metadata) => {
                        let mut song = Song::new(path);
                        track_metadata.apply_to_song(&mut song);
                        songs.push(song);
                        total += 1;

                        if songs.len() >= SONG_BATCH_SIZE {
                            library_handle.add_songs(std::mem::take(&mut songs));
                            songs = Vec::with_capacity(SONG_BATCH_SIZE);
                        }
                    }
                    Err(err) => {
                        eprintln!("{err}");
                    }
                }
            }
        }

        if !songs.is_empty() {
            library_handle.add_songs(songs);
        }

        let _ = event_tx.send(LibraryEvent::ScanComplete { total });
    });
}
