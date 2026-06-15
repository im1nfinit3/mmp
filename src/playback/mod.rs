//! Rust-native audio playback engine for local music files.
//!
//! This keeps the existing UI-facing control surface while replacing the
//! GStreamer runtime with `rodio` and Rust-native metadata extraction.

mod queue;
pub use queue::QueueState;

use std::path::{Path, PathBuf};
use std::sync::mpsc;
use std::thread;
use std::time::Duration;

use rodio::{Decoder, DeviceSinkBuilder, MixerDeviceSink, Player};

// ---------------------------------------------------------------------------
// Playback errors
// ---------------------------------------------------------------------------

/// Errors that can occur during playback operations.
#[derive(Debug)]
pub enum PlaybackError {
    /// Could not open the file.
    FileOpen {
        path: PathBuf,
        source: std::io::Error,
    },
    /// Could not decode the audio stream.
    Decode { path: PathBuf, detail: String },
}

impl std::fmt::Display for PlaybackError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::FileOpen { path, source } => {
                write!(f, "Failed to open {}: {source}", path.display())
            }
            Self::Decode { path, detail } => {
                write!(f, "Failed to decode {}: {detail}", path.display())
            }
        }
    }
}

impl std::error::Error for PlaybackError {}

use crate::library::metadata::{self, TrackMetadata};

#[derive(Clone, Debug)]
pub enum PlaybackEvent {
    Position {
        elapsed: f64,
        duration: f64,
    },
    EndOfStream,
    Tags {
        title: Option<String>,
        artist: Option<String>,
    },
    Error(String),
    StateChanged(PlaybackState),
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum PlaybackState {
    Playing,
    Paused,
    Stopped,
}

enum PlaybackCommand {
    Play(PathBuf),
    TogglePause,
    Stop,
    Seek(f64),
    SetVolume(f64),
    SetMute(bool),
    Shutdown,
}

struct ActivePlayback {
    player: Player,
    duration: Option<Duration>,
    generation: u64,
}

pub struct Playback {
    cmd_tx: mpsc::Sender<PlaybackCommand>,
    worker: Option<thread::JoinHandle<()>>,
}

impl Playback {
    pub fn new(event_tx: mpsc::Sender<PlaybackEvent>) -> Self {
        let (cmd_tx, cmd_rx) = mpsc::channel();
        let worker = thread::spawn(move || run_playback_thread(cmd_rx, event_tx));

        Self {
            cmd_tx,
            worker: Some(worker),
        }
    }

    pub fn play_file(&mut self, path: &Path) {
        let _ = self.cmd_tx.send(PlaybackCommand::Play(path.to_path_buf()));
    }

    pub fn toggle_pause(&mut self) {
        let _ = self.cmd_tx.send(PlaybackCommand::TogglePause);
    }

    pub fn stop(&mut self) {
        let _ = self.cmd_tx.send(PlaybackCommand::Stop);
    }

    pub fn seek(&mut self, seconds: f64) {
        let _ = self.cmd_tx.send(PlaybackCommand::Seek(seconds));
    }

    pub fn set_volume(&mut self, volume: f64) {
        let _ = self.cmd_tx.send(PlaybackCommand::SetVolume(volume));
    }

    pub fn set_mute(&mut self, mute: bool) {
        let _ = self.cmd_tx.send(PlaybackCommand::SetMute(mute));
    }
}

impl Drop for Playback {
    fn drop(&mut self) {
        let _ = self.cmd_tx.send(PlaybackCommand::Shutdown);
        if let Some(worker) = self.worker.take() {
            let _ = worker.join();
        }
    }
}

fn run_playback_thread(
    cmd_rx: mpsc::Receiver<PlaybackCommand>,
    event_tx: mpsc::Sender<PlaybackEvent>,
) {
    let (eos_tx, eos_rx) = mpsc::channel::<u64>();
    let mut sink = open_sink();
    let mut active: Option<ActivePlayback> = None;
    let mut generation = 0u64;
    let mut volume = 0.7f64;
    let mut muted = false;

    loop {
        match cmd_rx.recv_timeout(Duration::from_millis(100)) {
            Ok(PlaybackCommand::Play(path)) => {
                if sink.is_none() {
                    sink = open_sink();
                }

                let Some(ref sink_handle) = sink else {
                    let _ = event_tx.send(PlaybackEvent::Error(
                        "No output audio device available".to_string(),
                    ));
                    continue;
                };

                if let Some(current) = active.take() {
                    current.player.stop();
                }

                generation = generation.wrapping_add(1);

                match start_playback(
                    sink_handle,
                    &path,
                    generation,
                    volume,
                    muted,
                    &event_tx,
                    &eos_tx,
                ) {
                    Ok(next) => {
                        active = Some(next);
                        let _ = event_tx.send(PlaybackEvent::StateChanged(PlaybackState::Playing));
                    }
                    Err(err) => {
                        let _ = event_tx.send(PlaybackEvent::Error(err.to_string()));
                    }
                }
            }
            Ok(PlaybackCommand::TogglePause) => {
                if let Some(active_playback) = active.as_ref() {
                    if active_playback.player.is_paused() {
                        active_playback.player.play();
                        let _ = event_tx.send(PlaybackEvent::StateChanged(PlaybackState::Playing));
                    } else {
                        active_playback.player.pause();
                        let _ = event_tx.send(PlaybackEvent::StateChanged(PlaybackState::Paused));
                    }
                }
            }
            Ok(PlaybackCommand::Stop) => {
                if let Some(current) = active.take() {
                    current.player.stop();
                }
                let _ = event_tx.send(PlaybackEvent::StateChanged(PlaybackState::Stopped));
            }
            Ok(PlaybackCommand::Seek(seconds)) => {
                if let Some(active_playback) = active.as_ref() {
                    let max_duration = active_playback
                        .duration
                        .unwrap_or_else(|| Duration::from_secs_f64(seconds.max(0.0)));
                    let target = Duration::from_secs_f64(seconds.max(0.0)).min(max_duration);
                    if let Err(err) = active_playback.player.try_seek(target) {
                        let _ = event_tx.send(PlaybackEvent::Error(format!("Seek failed: {err}")));
                    }
                }
            }
            Ok(PlaybackCommand::SetVolume(next_volume)) => {
                volume = next_volume.clamp(0.0, 1.0);
                apply_effective_volume(active.as_ref(), volume, muted);
            }
            Ok(PlaybackCommand::SetMute(next_muted)) => {
                muted = next_muted;
                apply_effective_volume(active.as_ref(), volume, muted);
            }
            Ok(PlaybackCommand::Shutdown) | Err(mpsc::RecvTimeoutError::Disconnected) => break,
            Err(mpsc::RecvTimeoutError::Timeout) => {}
        }

        while let Ok(done_generation) = eos_rx.try_recv() {
            if active
                .as_ref()
                .is_some_and(|current| current.generation == done_generation)
            {
                active = None;
                let _ = event_tx.send(PlaybackEvent::EndOfStream);
            }
        }

        if let Some(active_playback) = active.as_ref() {
            let elapsed = active_playback.player.get_pos().as_secs_f64();
            let duration = active_playback
                .duration
                .map(|duration| duration.as_secs_f64())
                .unwrap_or(0.0);
            let _ = event_tx.send(PlaybackEvent::Position { elapsed, duration });
        }
    }

    if let Some(current) = active.take() {
        current.player.stop();
    }
}

fn start_playback(
    sink_handle: &MixerDeviceSink,
    path: &Path,
    generation: u64,
    volume: f64,
    muted: bool,
    event_tx: &mpsc::Sender<PlaybackEvent>,
    eos_tx: &mpsc::Sender<u64>,
) -> Result<ActivePlayback, PlaybackError> {
    let metadata = metadata::read_track_metadata(path).ok();
    emit_track_tags(event_tx, metadata.as_ref());

    let file = std::fs::File::open(path).map_err(|source| PlaybackError::FileOpen {
        path: path.to_path_buf(),
        source,
    })?;
    let decoder = Decoder::try_from(file).map_err(|err| PlaybackError::Decode {
        path: path.to_path_buf(),
        detail: err.to_string(),
    })?;

    let player = Player::connect_new(sink_handle.mixer());
    player.set_volume(effective_volume(volume, muted) as f32);
    player.append(decoder);

    let eos_tx = eos_tx.clone();
    player.append(rodio::source::EmptyCallback::new(Box::new(move || {
        let _ = eos_tx.send(generation);
    })));

    Ok(ActivePlayback {
        player,
        duration: metadata.and_then(|metadata| metadata.duration),
        generation,
    })
}

fn apply_effective_volume(active: Option<&ActivePlayback>, volume: f64, muted: bool) {
    if let Some(active_playback) = active {
        active_playback
            .player
            .set_volume(effective_volume(volume, muted) as f32);
    }
}

fn effective_volume(volume: f64, muted: bool) -> f64 {
    if muted { 0.0 } else { volume }
}

fn emit_track_tags(event_tx: &mpsc::Sender<PlaybackEvent>, metadata: Option<&TrackMetadata>) {
    let title = metadata.and_then(|metadata| metadata.title.clone());
    let artist = metadata.and_then(|metadata| metadata.artist.clone());
    let _ = event_tx.send(PlaybackEvent::Tags { title, artist });
}

fn open_sink() -> Option<MixerDeviceSink> {
    match DeviceSinkBuilder::open_default_sink() {
        Ok(mut sink) => {
            sink.log_on_drop(false);
            Some(sink)
        }
        Err(err) => {
            eprintln!("Failed to open audio output: {err}");
            None
        }
    }
}
