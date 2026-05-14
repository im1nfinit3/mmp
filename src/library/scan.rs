//! Directory scanner with async metadata extraction via GstDiscoverer.
//!
//! Two-stage pipeline:
//! 1. Background thread: walkdir traversal → path batches sent via mpsc
//! 2. Main thread: poll for batches, queue `discover_uri_async`, extract
//!    metadata in `discovered` callbacks, send populated Songs to Library.
//!
//! Files with discovery errors are skipped (may be revisited in the future).

use std::path::PathBuf;
use std::sync::{mpsc, Arc, Mutex};

use gstreamer as gst;
use gstreamer::prelude::*;
use gstreamer_pbutils::Discoverer;

use crate::library::song::Song;
use crate::library::{LibraryEvent, LibraryHandle};

// ---------------------------------------------------------------------------
// Shared mutable state between timeout poll and signal callbacks
// ---------------------------------------------------------------------------

struct ScanState {
    path_rx: mpsc::Receiver<Vec<PathBuf>>,
    library_handle: LibraryHandle,
    event_tx: mpsc::Sender<LibraryEvent>,
    walkdir_done: bool,
    pending: usize,
    total: usize,
    poll_source: Option<glib::SourceId>,
}

impl ScanState {
    /// Check whether the scan is fully complete and clean up if so.
    fn check_complete(&mut self) {
        if self.walkdir_done && self.pending == 0 {
            let _ = self
                .event_tx
                .send(LibraryEvent::ScanComplete { total: self.total });
            if let Some(id) = self.poll_source.take() {
                id.remove();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

/// Start an async directory scan.
///
/// Spawns a background walkdir thread and sets up a main-thread poller that
/// feeds paths to `GstDiscoverer::discover_uri_async`.  Metadata is extracted
/// in the `discovered` signal callback and sent to the Library actor.
pub fn start_scan(
    library_handle: LibraryHandle,
    event_tx: mpsc::Sender<LibraryEvent>,
) {
    let _ = event_tx.send(LibraryEvent::ScanStarted);

    // -- Channel: walkdir (bg) → main thread --
    let (path_tx, path_rx) = mpsc::channel::<Vec<PathBuf>>();

    // -- Spawn walkdir on background thread --
    std::thread::spawn(move || {
        let music_dir = dirs::audio_dir().unwrap_or_else(|| {
            dirs::home_dir()
                .unwrap_or_else(|| PathBuf::from("."))
                .join("Music")
        });

        let supported = ["mp3", "flac", "ogg", "wav", "m4a"];
        let mut batch = Vec::with_capacity(200);

        for entry in walkdir::WalkDir::new(&music_dir)
            .follow_links(true)
            .into_iter()
            .filter_map(|e| e.ok())
        {
            let path = entry.path();
            if !path.is_file() {
                continue;
            }
            let ext = path
                .extension()
                .and_then(|e| e.to_str())
                .map(|e| e.to_lowercase())
                .unwrap_or_default();
            if !supported.contains(&ext.as_str()) {
                continue;
            }

            batch.push(path.to_path_buf());

            if batch.len() >= 200 {
                let _ = path_tx.send(std::mem::take(&mut batch));
                batch = Vec::with_capacity(200);
            }
        }

        // Flush remaining
        if !batch.is_empty() {
            let _ = path_tx.send(batch);
        }
        // Signal walkdir completion with an empty batch
        let _ = path_tx.send(Vec::new());
    });

    // -- Create GstDiscoverer on main thread --
    let discoverer = Discoverer::new(gst::ClockTime::from_seconds(2))
        .expect("Failed to create GstDiscoverer");

    // -- Shared state (main-thread only, but Rust needs Send+Sync for closures) --
    let state = Arc::new(Mutex::new(ScanState {
        path_rx,
        library_handle,
        event_tx,
        walkdir_done: false,
        pending: 0,
        total: 0,
        poll_source: None,
    }));

    // -- "discovered" signal: extract metadata, send to Library --
    {
        let state = Arc::clone(&state);
        discoverer.connect_discovered(move |_discoverer, info, error| {
            let mut s = state.lock().unwrap();

            if let Some(err) = error {
                eprintln!(
                    "Discovery error for {}: {:?}",
                    info.uri(),
                    err
                );
                // TODO: may revisit — currently skipping files with
                // discovery errors
                s.pending = s.pending.saturating_sub(1);
                s.check_complete();
                return;
            }

            // Reconstruct file path from URI
            let uri_str = info.uri().to_string();
            let path = PathBuf::from(
                uri_str.strip_prefix("file://").unwrap_or(&uri_str),
            );

            let mut song = Song::new(path);

            // Duration
            if let Some(dur) = info.duration() {
                let total_secs = dur.seconds();
                let mins = total_secs / 60;
                let secs = total_secs % 60;
                song.duration_str = format!("{}:{:02}", mins, secs);
            }

            // Tags — discoverer_info.tags() is deprecated since
            // GStreamer 1.20 but remains functional.  The non-deprecated
            // path uses per-stream tags via audio_streams().
            #[allow(deprecated)]
            if let Some(tags) = info.tags() {
                if let Some(title) = tags.get::<gst::tags::Title>() {
                    song.title = title.get().to_string();
                }
                if let Some(artist) = tags.get::<gst::tags::Artist>() {
                    song.artist = artist.get().to_string();
                }
                if let Some(album) = tags.get::<gst::tags::Album>() {
                    song.album = album.get().to_string();
                }
            }

            s.library_handle.add_songs(vec![song]);
            s.total += 1;
            s.pending = s.pending.saturating_sub(1);
            s.check_complete();
        });
    }

    // -- Timeout-based poll for path batches --
    let state_for_poll = Arc::clone(&state);
    let discoverer_for_poll = discoverer.clone();
    let poll_id = glib::timeout_add_local(
        std::time::Duration::from_millis(100),
        move || {
            let mut s = state_for_poll.lock().unwrap();

            // Drain all available path batches
            while let Ok(batch) = s.path_rx.try_recv() {
                if batch.is_empty() {
                    // walkdir sentinel — no more paths coming
                    s.walkdir_done = true;
                    s.check_complete();
                } else {
                    for path in &batch {
                        let uri = format!("file://{}", path.display());
                        if discoverer_for_poll
                            .discover_uri_async(&uri)
                            .is_ok()
                        {
                            s.pending += 1;
                        }
                    }
                }
            }

            glib::ControlFlow::Continue
        },
    );

    state.lock().unwrap().poll_source = Some(poll_id);

    // -- Start processing the queue --
    discoverer.start();
}

