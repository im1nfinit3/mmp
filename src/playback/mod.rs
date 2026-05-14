//! GStreamer-based audio playback engine.

mod queue;
pub use queue::QueueState;

use gstreamer as gst;
use gstreamer::prelude::*;
use std::path::Path;
use std::sync::mpsc;
use std::time::Duration;

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

        let mut pb = Self {
            playbin,
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
        self.playbin
            .set_property("uri", format!("file://{}", path.display()));
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
        let _ = self
            .playbin
            .seek_simple(gst::SeekFlags::FLUSH | gst::SeekFlags::KEY_UNIT, pos);
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
}
