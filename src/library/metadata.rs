use std::fmt;
use std::path::{Path, PathBuf};
use std::time::Duration;

use lofty::file::{TaggedFile, TaggedFileExt};
use lofty::tag::Accessor;
use rodio::{Decoder, Source};

use super::song::Song;

// ---------------------------------------------------------------------------
// Metadata errors
// ---------------------------------------------------------------------------

/// Errors that can occur during metadata extraction.
#[derive(Debug)]
pub enum MetadataError {
    /// Failed to read audio file tags.
    TagRead { path: PathBuf, detail: String },
    /// I/O error accessing the file.
    Io(std::io::Error),
}

impl fmt::Display for MetadataError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::TagRead { path, detail } => {
                write!(f, "failed to read tags for {}: {detail}", path.display())
            }
            Self::Io(err) => write!(f, "{err}"),
        }
    }
}

impl std::error::Error for MetadataError {
    fn source(&self) -> Option<&(dyn std::error::Error + 'static)> {
        match self {
            Self::Io(err) => Some(err),
            Self::TagRead { .. } => None,
        }
    }
}

impl From<std::io::Error> for MetadataError {
    fn from(err: std::io::Error) -> Self {
        Self::Io(err)
    }
}

pub const SUPPORTED_EXTENSIONS: [&str; 5] = ["mp3", "flac", "ogg", "wav", "m4a"];

#[derive(Clone, Debug, Default)]
pub struct TrackMetadata {
    pub title: Option<String>,
    pub artist: Option<String>,
    pub album: Option<String>,
    pub duration: Option<Duration>,
}

impl TrackMetadata {
    pub fn apply_to_song(&self, song: &mut Song) {
        if let Some(title) = self.title.as_ref() {
            song.title = title.clone();
        }
        if let Some(artist) = self.artist.as_ref() {
            song.artist = artist.clone();
        }
        if let Some(album) = self.album.as_ref() {
            song.album = album.clone();
        }
        if let Some(duration) = self.duration {
            song.duration_str = format_duration(duration);
        }
    }
}

pub fn is_supported_audio_path(path: &Path) -> bool {
    path.extension()
        .and_then(|ext| ext.to_str())
        .map(|ext| ext.to_ascii_lowercase())
        .is_some_and(|ext| SUPPORTED_EXTENSIONS.contains(&ext.as_str()))
}

pub fn read_track_metadata(path: &Path) -> Result<TrackMetadata, MetadataError> {
    let tagged_file = lofty::read_from_path(path).map_err(|err| MetadataError::TagRead {
        path: path.to_path_buf(),
        detail: err.to_string(),
    })?;

    let mut metadata = TrackMetadata::default();
    apply_lofty_tags(&mut metadata, &tagged_file);
    metadata.duration = read_duration_value(path);
    Ok(metadata)
}

pub fn apply_lofty_tags(metadata: &mut TrackMetadata, tagged_file: &TaggedFile) {
    let primary = tagged_file
        .primary_tag()
        .or_else(|| tagged_file.first_tag());

    if let Some(tag) = primary {
        metadata.title = metadata
            .title
            .take()
            .or_else(|| sanitized(tag.title().as_deref()));
        metadata.artist = metadata
            .artist
            .take()
            .or_else(|| sanitized(tag.artist().as_deref()));
        metadata.album = metadata
            .album
            .take()
            .or_else(|| sanitized(tag.album().as_deref()));
    }

    if metadata.title.is_some() && metadata.artist.is_some() && metadata.album.is_some() {
        return;
    }

    for tag in tagged_file.tags() {
        if metadata.title.is_none() {
            metadata.title = sanitized(tag.title().as_deref());
        }
        if metadata.artist.is_none() {
            metadata.artist = sanitized(tag.artist().as_deref());
        }
        if metadata.album.is_none() {
            metadata.album = sanitized(tag.album().as_deref());
        }
        if metadata.title.is_some() && metadata.artist.is_some() && metadata.album.is_some() {
            break;
        }
    }
}

pub fn read_duration_value(path: &Path) -> Option<Duration> {
    let file = std::fs::File::open(path).ok()?;
    let decoder = Decoder::try_from(file).ok()?;
    decoder.total_duration()
}

pub fn format_duration(duration: Duration) -> String {
    let total_secs = duration.as_secs();
    let mins = total_secs / 60;
    let secs = total_secs % 60;
    format!("{}:{:02}", mins, secs)
}

fn sanitized(value: Option<&str>) -> Option<String> {
    value
        .map(str::trim)
        .filter(|value| !value.is_empty())
        .map(ToOwned::to_owned)
}
