//! Persistent application settings stored as a TOML config file.
//!
//! The file lives at `~/.config/mmp/mmp.conf` and is human-editable.
//! Hand-written comments are preserved on round-trip (only known keys
//! are overwritten; everything else in the file passes through).

use std::fmt;
use std::fs;
use std::path::PathBuf;

use crate::core::Page;
use crate::library::db;
use crate::library::song::Song;

// ---------------------------------------------------------------------------
// Settings errors
// ---------------------------------------------------------------------------

/// Errors that can occur when reading or writing settings.
#[derive(Debug)]
pub enum SettingsError {
    /// An I/O error occurred (read/write/create dir).
    Io(String),
    /// A value in the config file could not be parsed.
    #[allow(dead_code)]
    Parse { key: String, detail: String },
}

impl fmt::Display for SettingsError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::Io(msg) => write!(f, "{msg}"),
            Self::Parse { key, detail } => {
                write!(f, "Invalid value for '{key}': {detail}")
            }
        }
    }
}

impl std::error::Error for SettingsError {}

impl From<std::io::Error> for SettingsError {
    fn from(e: std::io::Error) -> Self {
        Self::Io(e.to_string())
    }
}

// ---------------------------------------------------------------------------
// Sort method for song-list views
// ---------------------------------------------------------------------------

/// Default sort order applied to song-list views (except Recently Added).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum SortMethod {
    AlphabeticalArtist,
    AlphabeticalAlbum,
    AlphabeticalTitle,
    TimeAddedNewestFirst,
    TimeAddedOldestFirst,
}

impl SortMethod {
    pub const ALL: &'static [Self] = &[
        Self::AlphabeticalArtist,
        Self::AlphabeticalAlbum,
        Self::AlphabeticalTitle,
        Self::TimeAddedNewestFirst,
        Self::TimeAddedOldestFirst,
    ];

    pub fn label(&self) -> &'static str {
        match self {
            Self::AlphabeticalArtist => "Alphabetical (artist)",
            Self::AlphabeticalAlbum => "Alphabetical (album)",
            Self::AlphabeticalTitle => "Alphabetical (title)",
            Self::TimeAddedNewestFirst => "Time added (newest first)",
            Self::TimeAddedOldestFirst => "Time added (oldest first)",
        }
    }

    pub fn from_str(s: &str) -> Self {
        match s {
            "AlphabeticalArtist" => Self::AlphabeticalArtist,
            "AlphabeticalAlbum" => Self::AlphabeticalAlbum,
            "AlphabeticalTitle" => Self::AlphabeticalTitle,
            "TimeAddedOldestFirst" => Self::TimeAddedOldestFirst,
            _ => Self::TimeAddedNewestFirst,
        }
    }

    pub fn as_str(&self) -> &'static str {
        match self {
            Self::AlphabeticalArtist => "AlphabeticalArtist",
            Self::AlphabeticalAlbum => "AlphabeticalAlbum",
            Self::AlphabeticalTitle => "AlphabeticalTitle",
            Self::TimeAddedNewestFirst => "TimeAddedNewestFirst",
            Self::TimeAddedOldestFirst => "TimeAddedOldestFirst",
        }
    }

    /// Sort a slice of songs in-place by this method.
    pub fn sort_songs(&self, songs: &mut [Song]) {
        match self {
            Self::AlphabeticalArtist => {
                songs.sort_by(|a, b| {
                    a.artist
                        .to_lowercase()
                        .cmp(&b.artist.to_lowercase())
                        .then_with(|| a.album.to_lowercase().cmp(&b.album.to_lowercase()))
                        .then_with(|| a.title.to_lowercase().cmp(&b.title.to_lowercase()))
                });
            }
            Self::AlphabeticalAlbum => {
                songs.sort_by(|a, b| {
                    a.album
                        .to_lowercase()
                        .cmp(&b.album.to_lowercase())
                        .then_with(|| a.title.to_lowercase().cmp(&b.title.to_lowercase()))
                });
            }
            Self::AlphabeticalTitle => {
                songs.sort_by(|a, b| a.title.to_lowercase().cmp(&b.title.to_lowercase()));
            }
            Self::TimeAddedNewestFirst => {
                songs.sort_by(|a, b| b.db_id.cmp(&a.db_id));
            }
            Self::TimeAddedOldestFirst => {
                songs.sort_by(|a, b| a.db_id.cmp(&b.db_id));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Settings model
// ---------------------------------------------------------------------------

/// The application settings surfaced in the Settings page.
#[derive(Clone, Debug)]
pub struct Settings {
    /// Custom music library folder. `None` = use platform default (`dirs::audio_dir()`).
    pub library_folder: Option<String>,
    /// Whether to scan the library for new/changed files on startup.
    pub scan_on_startup: bool,
    /// Which page to show when the app starts.
    pub default_view: String,
    /// Default sort method for song-list views (except Recently Added).
    pub default_sort: SortMethod,
}

impl Default for Settings {
    fn default() -> Self {
        Self {
            library_folder: None,
            scan_on_startup: true,
            default_view: String::from("RecentlyAdded"),
            default_sort: SortMethod::TimeAddedNewestFirst,
        }
    }
}

impl Settings {
    /// Expand a leading `~/` or `~` to the user's home directory.
    fn expand_tilde(path: &str) -> PathBuf {
        let trimmed = path.trim();
        if trimmed == "~" {
            return dirs::home_dir().unwrap_or_else(|| PathBuf::from("."));
        }
        if let Some(rest) = trimmed.strip_prefix("~/") {
            let home = dirs::home_dir().unwrap_or_else(|| PathBuf::from("."));
            home.join(rest)
        } else {
            PathBuf::from(trimmed)
        }
    }

    /// Resolve the effective library folder path, expanding `~` to the home dir.
    pub fn effective_library_folder(&self) -> PathBuf {
        match &self.library_folder {
            Some(path) if !path.trim().is_empty() => Self::expand_tilde(path),
            _ => dirs::audio_dir().unwrap_or_else(|| {
                dirs::home_dir()
                    .unwrap_or_else(|| PathBuf::from("."))
                    .join("Music")
            }),
        }
    }

    /// Parse `default_view` back into a `Page`.
    pub fn startup_page(&self) -> Page {
        match self.default_view.as_str() {
            "Songs" => Page::Songs,
            "Albums" => Page::Albums,
            "Artists" => Page::Artists,
            "Queue" => Page::Queue,
            _ => Page::RecentlyAdded,
        }
    }

    /// All valid page names for the default-view dropdown.
    pub const ALL_VIEWS: &'static [&'static str] =
        &["RecentlyAdded", "Songs", "Albums", "Artists", "Queue"];
}

// ---------------------------------------------------------------------------
// Persistence (TOML via toml_edit, format-preserving)
// ---------------------------------------------------------------------------

const CONFIG_FILENAME: &str = "mmp.conf";

/// Full path to the config file.
fn config_path() -> PathBuf {
    db::config_dir().join(CONFIG_FILENAME)
}

/// Load settings from `~/.config/mmp/mmp.conf`.
///
/// Returns `Default` if the file is missing, empty, or unreadable.
pub fn load_settings() -> Settings {
    let path = config_path();

    let text = match fs::read_to_string(&path) {
        Ok(t) => t,
        Err(_) => return Settings::default(),
    };

    // Strip UTF-8 BOM if present (Windows Notepad compat).
    let text = text.strip_prefix('\u{FEFF}').unwrap_or(&text);

    let doc: toml_edit::DocumentMut = match text.parse() {
        Ok(d) => d,
        Err(_) => return Settings::default(),
    };

    Settings {
        library_folder: doc
            .get("library_folder")
            .and_then(|v| v.as_str())
            .filter(|s| !s.is_empty())
            .map(String::from),

        scan_on_startup: doc
            .get("scan_on_startup")
            .and_then(|v| v.as_bool())
            .unwrap_or(true),

        default_view: doc
            .get("default_view")
            .and_then(|v| v.as_str())
            .filter(|v| Settings::ALL_VIEWS.contains(v))
            .map(String::from)
            .unwrap_or_else(|| String::from("RecentlyAdded")),

        default_sort: doc
            .get("default_sort")
            .and_then(|v| v.as_str())
            .map(SortMethod::from_str)
            .unwrap_or(SortMethod::TimeAddedNewestFirst),
    }
}

/// Persist settings to `~/.config/mmp/mmp.conf`.
///
/// This is format-preserving: any existing keys, comments, or formatting
/// in the file that aren't known settings are passed through unchanged.
pub fn save_settings(settings: &Settings) -> Result<(), SettingsError> {
    let path = config_path();
    let dir = path.parent().unwrap();
    fs::create_dir_all(dir)?;

    // Try to load the existing document to preserve comments/formatting.
    let (mut doc, created) = match fs::read_to_string(&path) {
        Ok(text) => match text.parse::<toml_edit::DocumentMut>() {
            Ok(d) => (d, false),
            Err(_) => (toml_edit::DocumentMut::new(), true),
        },
        Err(_) => (toml_edit::DocumentMut::new(), true),
    };

    // Write each known key.
    doc["library_folder"] = toml_edit::value(
        settings.library_folder.as_deref().unwrap_or(""),
    );
    doc["scan_on_startup"] = toml_edit::value(settings.scan_on_startup);
    doc["default_view"] = toml_edit::value(&settings.default_view);
    doc["default_sort"] = toml_edit::value(settings.default_sort.as_str());

    // Serialize and prepend the header comment for brand-new files.
    let mut content = doc.to_string();
    if created {
        content = format!(
            "# mmp configuration — edit while the app is closed.\n\
             # Settings changed in the app will overwrite this file.\n\
             # Unrecognised keys are preserved.\n\
             \n\
             {content}"
        );
    }

    // Atomic write: tmp file + rename.
    let tmp_path = dir.join("mmp.conf.tmp");
    fs::write(&tmp_path, content)?;
    fs::rename(&tmp_path, &path)?;

    Ok(())
}
