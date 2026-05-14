//! Directory scanner for the music library.
//!
//! Phase A: path-only scan on background thread (behavior-identical to current code).
//! Phase C: will be rewritten with async `GstDiscoverer` for scan-time metadata.

use std::path::PathBuf;

use relm4::ComponentSender;

use crate::app::{AppModel, AppMsg};

/// Walk the user's music directory on a background thread, sending batches of
/// supported audio file paths to the main thread via the Relm4 sender.
pub fn scan_directory(sender: ComponentSender<AppModel>) {
    let music_dir = dirs::audio_dir().unwrap_or_else(|| {
        dirs::home_dir()
            .unwrap_or_else(|| PathBuf::from("."))
            .join("Music")
    });

    let supported = ["mp3", "flac", "ogg", "wav", "m4a"];
    let mut count = 0usize;
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

        count += 1;
        batch.push(path.to_path_buf());

        // Send batches of 200 to avoid flooding the message queue
        if batch.len() >= 200 {
            let _ = sender.input(AppMsg::BatchScan(std::mem::take(&mut batch)));
            batch = Vec::with_capacity(200);
        }
    }

    // Send remaining
    if !batch.is_empty() {
        let _ = sender.input(AppMsg::BatchScan(batch));
    }
    let _ = sender.input(AppMsg::ScanComplete(count));
}
