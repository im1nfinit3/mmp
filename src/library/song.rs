//! Pure data types for the music library.

use std::path::PathBuf;

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
        let filename = path
            .file_stem()
            .and_then(|s| s.to_str())
            .unwrap_or("Unknown")
            .to_string();
        Self {
            path,
            title: filename,
            artist: String::from("Unknown Artist"),
            album: String::from("Unknown Album"),
            duration_str: String::new(),
        }
    }

    pub fn label(&self) -> String {
        if self.artist.is_empty() || self.artist == "Unknown Artist" {
            self.title.clone()
        } else {
            format!("{} — {}", self.title, self.artist)
        }
    }
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
