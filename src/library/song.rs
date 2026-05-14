//! Pure data types for the music library.

use std::path::PathBuf;

pub const UNKNOWN_ARTIST: &str = "Unknown Artist";
pub const UNKNOWN_ALBUM: &str = "Unknown Album";

/// A single song/track in the library.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Song {
    pub path: PathBuf,
    pub title: String,
    pub artist: String,
    pub album: String,
    pub duration_str: String,
}

impl Song {
    pub fn new(path: PathBuf) -> Self {
        let filename = fallback_title_for_path(&path);
        Self {
            path,
            title: filename,
            artist: String::from(UNKNOWN_ARTIST),
            album: String::from(UNKNOWN_ALBUM),
            duration_str: String::new(),
        }
    }

    pub fn has_complete_metadata(&self) -> bool {
        self.title != fallback_title_for_path(&self.path)
            && self.artist != UNKNOWN_ARTIST
            && self.album != UNKNOWN_ALBUM
            && !self.duration_str.is_empty()
    }

    pub fn label(&self) -> String {
        if self.artist.is_empty() || self.artist == UNKNOWN_ARTIST {
            self.title.clone()
        } else {
            format!("{} — {}", self.title, self.artist)
        }
    }
}

pub fn fallback_title_for_path(path: &PathBuf) -> String {
    path.file_stem()
        .and_then(|s| s.to_str())
        .unwrap_or("Unknown")
        .to_string()
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum RepeatMode {
    Off,
    All,
    One,
}

impl RepeatMode {
    pub fn next(self) -> Self {
        match self {
            Self::Off => Self::All,
            Self::All => Self::One,
            Self::One => Self::Off,
        }
    }

    pub fn label(self) -> &'static str {
        match self {
            Self::Off => "Repeat: Off",
            Self::All => "Repeat: All",
            Self::One => "Repeat: One",
        }
    }
}
