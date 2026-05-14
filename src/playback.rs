use gstreamer as gst;
use gstreamer::prelude::*;
use gstreamer_pbutils::Discoverer;
use glib;
use std::path::{Path, PathBuf};
use std::sync::mpsc;
use std::time::Duration;

use crate::app::{RepeatMode, Song};

// ---------------------------------------------------------------------------
// Messages emitted by the playback engine for the UI layer
// ---------------------------------------------------------------------------

/// Events the playback engine sends to the UI.
#[derive(Clone, Debug)]
pub enum PlaybackEvent {
    /// Current playback position (seconds) and total duration (seconds).
    Position { elapsed: f64, duration: f64 },
    /// Track finished (end of stream).
    EndOfStream,
    /// New metadata tags discovered for the current track.
    Tags {
        title: Option<String>,
        artist: Option<String>,
    },
    /// An error occurred.
    Error(String),
    /// Playback state changed.
    StateChanged(PlaybackState),
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum PlaybackState {
    Playing,
    Paused,
    Stopped,
}

// ---------------------------------------------------------------------------
// Playback engine
// ---------------------------------------------------------------------------

pub struct Playback {
    playbin: gst::Element,
    discoverer: Discoverer,
    event_tx: mpsc::Sender<PlaybackEvent>,
    /// Timer ID for periodic UI updates.
    update_timer_id: Option<glib::SourceId>,
}

impl Playback {
    /// Create a new playback engine.
    /// `event_tx` is used to send events (position, EOS, etc.) to the UI.
    pub fn new(event_tx: mpsc::Sender<PlaybackEvent>) -> Self {
        let playbin = gst::ElementFactory::make("playbin")
            .name("playbin")
            .build()
            .expect("Failed to create playbin element");

        // GstDiscoverer with 2-second timeout
        let discoverer = Discoverer::new(
            gst::ClockTime::from_seconds(2),
        )
        .expect("Failed to create GstDiscoverer");

        let mut pb = Self {
            playbin,
            discoverer,
            event_tx,
            update_timer_id: None,
        };
        pb.setup_bus_watch();
        pb
    }

    // -- Bus message handling --

    fn setup_bus_watch(&mut self) {
        let bus = self.playbin.bus().expect("playbin has no bus");
        let tx = self.event_tx.clone();

        bus.connect_message(None, move |_, msg: &gst::Message| {
            match msg.view() {
                gst::MessageView::Eos(_) => {
                    let _ = tx.send(PlaybackEvent::EndOfStream);
                }
                gst::MessageView::Error(err) => {
                    let _ = tx.send(PlaybackEvent::Error(format!(
                        "GStreamer error: {} ({:?})",
                        err.error(),
                        err.debug()
                    )));
                }
                gst::MessageView::Tag(tag_msg) => {
                    // Extract title and artist from tags
                    let tag_list = tag_msg.tags();
                    let title = tag_list
                        .get::<gst::tags::Title>()
                        .map(|t| t.get().to_string());
                    let artist = tag_list
                        .get::<gst::tags::Artist>()
                        .map(|a| a.get().to_string());
                    if title.is_some() || artist.is_some() {
                        let _ = tx.send(PlaybackEvent::Tags { title, artist });
                    }
                }
                _ => {}
            }
        });
    }

    // -- Playback control --

    /// Play the file at `path`. Returns immediately; playback starts async.
    pub fn play_file(&mut self, path: &Path) {
        self.playbin.set_property(
            "uri",
            format!("file://{}", path.display()),
        );
        let _ = self.playbin.set_state(gst::State::Playing);
        self.event_tx
            .send(PlaybackEvent::StateChanged(PlaybackState::Playing))
            .ok();
    }

    /// Toggle between Playing and Paused.
    pub fn toggle_pause(&mut self) {
        let (change_result, current, _pending) = self.playbin.state(None);
        let is_playing = change_result.is_ok() && current == gst::State::Playing;

        if is_playing {
            let _ = self.playbin.set_state(gst::State::Paused);
            self.event_tx
                .send(PlaybackEvent::StateChanged(PlaybackState::Paused))
                .ok();
        } else {
            let _ = self.playbin.set_state(gst::State::Playing);
            self.event_tx
                .send(PlaybackEvent::StateChanged(PlaybackState::Playing))
                .ok();
        }
    }

    /// Stop playback entirely.
    pub fn stop(&mut self) {
        let _ = self.playbin.set_state(gst::State::Null);
        self.event_tx
            .send(PlaybackEvent::StateChanged(PlaybackState::Stopped))
            .ok();
    }

    /// Seek to `seconds` in the current track.
    pub fn seek(&mut self, seconds: f64) {
        let pos = gst::ClockTime::from_seconds(seconds as u64);
        let _ = self.playbin.seek_simple(
            gst::SeekFlags::FLUSH | gst::SeekFlags::KEY_UNIT,
            pos,
        );
    }

    /// Set volume in range [0.0, 1.0].
    pub fn set_volume(&mut self, volume: f64) {
        self.playbin.set_property("volume", volume.clamp(0.0, 1.0));
    }

    /// Mute or unmute.
    pub fn set_mute(&mut self, mute: bool) {
        self.playbin.set_property("mute", mute);
    }

    /// Query current position (seconds) and duration (seconds).
    pub fn query_position(&self) -> Option<(f64, f64)> {
        let pos = self.playbin.query_position::<gst::ClockTime>();
        let dur = self.playbin.query_duration::<gst::ClockTime>();
        match (pos, dur) {
            (Some(p), Some(d)) => Some((p.seconds() as f64, d.seconds() as f64)),
            _ => None,
        }
    }

    // -- Periodic UI update timer --

    /// Start a 500 ms timer that sends `PlaybackEvent::Position` updates.
    pub fn start_ui_timer(&mut self) {
        if self.update_timer_id.is_some() {
            return; // already running
        }
        let tx = self.event_tx.clone();
        let playbin = self.playbin.clone();
        let id = glib::timeout_add_local(Duration::from_millis(500), move || {
            let pos = playbin.query_position::<gst::ClockTime>();
            let dur = playbin.query_duration::<gst::ClockTime>();
            if let (Some(p), Some(d)) = (pos, dur) {
                let _ = tx.send(PlaybackEvent::Position {
                    elapsed: p.seconds() as f64,
                    duration: d.seconds() as f64,
                });
            } else {
                let _ = tx.send(PlaybackEvent::Position {
                    elapsed: 0.0,
                    duration: 0.0,
                });
            }
            glib::ControlFlow::Continue
        });
        self.update_timer_id = Some(id);
    }

    /// Stop the periodic UI update timer.
    pub fn stop_ui_timer(&mut self) {
        if let Some(id) = self.update_timer_id.take() {
            id.remove();
        }
    }

    // -- Metadata extraction via GstDiscoverer --

    /// Extract metadata (title, artist, album, duration) from a file.
    /// Returns the Song with populated fields.
    pub fn extract_metadata(&self, song: &mut Song) {
        let uri = format!("file://{}", song.path.display());
        match self.discoverer.discover_uri(&uri) {
            Ok(info) => {
                // Duration
                if let Some(dur) = info.duration() {
                    let total_secs = dur.seconds();
                    let mins = total_secs / 60;
                    let secs = total_secs % 60;
                    song.duration_str = format!("{}:{:02}", mins, secs);
                }

                // Tags
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
            }
            Err(err) => {
                eprintln!(
                    "Failed to discover metadata for {}: {:?}",
                    song.path.display(),
                    err
                );
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Queue operations (pure logic, no GStreamer dependency)
// ---------------------------------------------------------------------------

/// Queue state for linear + shuffle playback.
pub struct QueueState {
    /// Ordered list of file paths in the playback queue.
    pub tracks: Vec<PathBuf>,
    /// Index of the currently playing track (None if nothing playing).
    pub current: Option<usize>,
    /// Indices into `tracks` of unplayed songs (for shuffle mode).
    pub unplayed_pool: Vec<usize>,
    pub shuffle: bool,
    pub repeat: RepeatMode,
}

impl QueueState {
    pub fn new() -> Self {
        Self {
            tracks: Vec::new(),
            current: None,
            unplayed_pool: Vec::new(),
            shuffle: false,
            repeat: RepeatMode::Off,
        }
    }

    /// Add a track to the end of the queue.
    /// Returns the index of the newly added track.
    pub fn push(&mut self, path: PathBuf) -> usize {
        let idx = self.tracks.len();
        self.tracks.push(path);
        if self.shuffle {
            self.unplayed_pool.push(idx);
        }
        idx
    }

    /// Insert a track right after the current track.
    /// Returns the index of the inserted track.
    pub fn insert_after_current(&mut self, path: PathBuf) -> Option<usize> {
        let current = self.current?;
        let insert_idx = current + 1;
        self.tracks.insert(insert_idx, path);

        // Adjust all indices in unplayed_pool >= insert_idx
        for idx in &mut self.unplayed_pool {
            if *idx >= insert_idx {
                *idx += 1;
            }
        }
        // Shift current if needed (it won't be, since we insert after)
        if self.shuffle {
            self.unplayed_pool.push(insert_idx);
        }

        Some(insert_idx)
    }

    /// Remove a track at the given index.
    pub fn remove(&mut self, index: usize) {
        if index >= self.tracks.len() {
            return;
        }
        self.tracks.remove(index);

        // Adjust current
        if let Some(ref mut cur) = self.current {
            if index == *cur {
                // Current track was removed — move to the next, or clear
                if *cur < self.tracks.len() {
                    // next track shifted into position, stay put
                } else if !self.tracks.is_empty() {
                    *cur = self.tracks.len() - 1;
                } else {
                    self.current = None;
                }
            } else if index < *cur {
                *cur -= 1;
            }
        }

        // Adjust unplayed_pool: remove index, shift others
        self.unplayed_pool.retain(|&i| i != index);
        for idx in &mut self.unplayed_pool {
            if *idx > index {
                *idx -= 1;
            }
        }
    }

    /// Remove the node at the given index (returns path if found).
    pub fn remove_node(&mut self, node_index: usize) -> Option<PathBuf> {
        if node_index >= self.tracks.len() {
            return None;
        }
        let path = self.tracks[node_index].clone();
        self.remove(node_index);
        Some(path)
    }

    /// Clear the entire queue.
    pub fn clear(&mut self) {
        self.tracks.clear();
        self.current = None;
        self.unplayed_pool.clear();
    }

    /// Toggle shuffle mode on/off.
    pub fn toggle_shuffle(&mut self) {
        self.shuffle = !self.shuffle;
        if self.shuffle {
            self.rebuild_unplayed_pool();
        } else {
            self.unplayed_pool.clear();
        }
    }

    /// Cycle repeat mode: Off → All → One → Off.
    pub fn cycle_repeat(&mut self) {
        self.repeat = self.repeat.next();
    }

    /// Rebuild the unplayed pool: all queue indices EXCEPT the current one.
    pub fn rebuild_unplayed_pool(&mut self) {
        self.unplayed_pool.clear();
        for i in 0..self.tracks.len() {
            if Some(i) != self.current {
                self.unplayed_pool.push(i);
            }
        }
    }

    /// Determine the next track index to play.
    /// Returns None if playback should stop.
    pub fn next_track(&mut self) -> Option<usize> {
        if self.repeat == RepeatMode::One {
            return self.current;
        }

        if self.shuffle {
            if self.unplayed_pool.is_empty() {
                if self.repeat == RepeatMode::All {
                    self.rebuild_unplayed_pool();
                    return self.next_track(); // recurse (tail-recursive, won't blow stack)
                }
                return None;
            }
            // Pick random index from unplayed pool
            let pool_idx = rand::random::<usize>() % self.unplayed_pool.len();
            let track_idx = self.unplayed_pool.swap_remove(pool_idx);
            return Some(track_idx);
        }

        // Linear mode
        let current = self.current?;
        let next = current + 1;
        if next < self.tracks.len() {
            Some(next)
        } else if self.repeat == RepeatMode::All {
            Some(0)
        } else {
            None
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_linear_queue() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.push(PathBuf::from("c.mp3"));
        q.current = Some(0);

        assert_eq!(q.next_track(), Some(1));
        q.current = Some(1);
        assert_eq!(q.next_track(), Some(2));
        q.current = Some(2);
        assert_eq!(q.next_track(), None); // end of queue, repeat off
    }

    #[test]
    fn test_repeat_all_linear() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.repeat = RepeatMode::All;
        q.current = Some(1);
        assert_eq!(q.next_track(), Some(0)); // wraps to head
    }

    #[test]
    fn test_repeat_one() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.repeat = RepeatMode::One;
        q.current = Some(0);
        assert_eq!(q.next_track(), Some(0)); // stays
    }

    #[test]
    fn test_shuffle_exhausts_pool() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.shuffle = true;
        q.current = Some(0);
        // pool should have 1 item (index 1)
        assert_eq!(q.unplayed_pool.len(), 1);
        let next = q.next_track().unwrap();
        assert_eq!(next, 1);
        // pool exhausted
        assert!(q.unplayed_pool.is_empty());
        assert_eq!(q.next_track(), None);
    }

    #[test]
    fn test_shuffle_repeat_all_rebuilds_pool() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.shuffle = true;
        q.repeat = RepeatMode::All;
        q.current = Some(0);
        // exhaust pool
        let _ = q.next_track();
        // pool should be rebuilt, next_track should succeed
        assert!(!q.unplayed_pool.is_empty());
    }

    #[test]
    fn test_remove_current_track() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.push(PathBuf::from("c.mp3"));
        q.current = Some(1);
        q.remove(1);
        // b removed, current should shift to what was c (now at index 1)
        assert_eq!(q.current, Some(1));
        assert_eq!(q.tracks.len(), 2);
        assert_eq!(q.tracks[0], PathBuf::from("a.mp3"));
        assert_eq!(q.tracks[1], PathBuf::from("c.mp3"));
    }

    #[test]
    fn test_remove_before_current() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.push(PathBuf::from("c.mp3"));
        q.current = Some(2);
        q.remove(0);
        assert_eq!(q.current, Some(1)); // shifted down
        assert_eq!(q.tracks.len(), 2);
    }

    #[test]
    fn test_insert_after_current() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.current = Some(0);
        let idx = q.insert_after_current(PathBuf::from("x.mp3")).unwrap();
        assert_eq!(idx, 1);
        assert_eq!(q.tracks[1], PathBuf::from("x.mp3"));
        assert_eq!(q.tracks[2], PathBuf::from("b.mp3"));
    }
}
