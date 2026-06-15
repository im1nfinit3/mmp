//! Persistent application settings stored via SQLite (key-value).

use rusqlite::{Connection, params};
use std::fmt;
use std::path::PathBuf;

use crate::app_core::Page;
use crate::library::db;
use crate::library::song::Song;

// ---------------------------------------------------------------------------
// Settings errors
// ---------------------------------------------------------------------------

/// Errors that can occur when reading or writing settings.
#[derive(Debug)]
pub enum SettingsError {
    /// A database operation failed.
    Db(String),
    /// Failed to save a specific setting key.
    Save { key: String, detail: String },
}

impl fmt::Display for SettingsError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::Db(msg) => write!(f, "{msg}"),
            Self::Save { key, detail } => {
                write!(f, "Failed to save setting '{key}': {detail}")
            }
        }
    }
}

impl std::error::Error for SettingsError {}

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
// Persistence
// ---------------------------------------------------------------------------

const SETTINGS_DB_FILENAME: &str = "settings.db";
const SETTINGS_DDL: &str = "
CREATE TABLE IF NOT EXISTS settings (
    key   TEXT PRIMARY KEY,
    value TEXT NOT NULL
);
";

/// Open (or create) the settings database.
fn open_settings_db() -> Result<Connection, SettingsError> {
    let path = db::config_dir().join(SETTINGS_DB_FILENAME);
    let conn = Connection::open(&path)
        .map_err(|e| SettingsError::Db(format!("Failed to open settings db: {e}")))?;
    conn.execute_batch("PRAGMA journal_mode=WAL;").ok();
    conn.execute_batch(SETTINGS_DDL)
        .map_err(|e| SettingsError::Db(format!("Failed to create settings table: {e}")))?;
    Ok(conn)
}

/// Load settings from the database. Returns `Default` if the DB or row is missing.
pub fn load_settings() -> Settings {
    let Ok(conn) = open_settings_db() else {
        return Settings::default();
    };

    let get_str = |key: &str| -> Option<String> {
        conn.query_row(
            "SELECT value FROM settings WHERE key = ?1",
            params![key],
            |row| row.get(0),
        )
        .ok()
    };

    Settings {
        library_folder: get_str("library_folder"),
        scan_on_startup: get_str("scan_on_startup")
            .and_then(|v| v.parse::<bool>().ok())
            .unwrap_or(true),
        default_view: get_str("default_view")
            .filter(|v| Settings::ALL_VIEWS.contains(&v.as_str()))
            .unwrap_or_else(|| String::from("RecentlyAdded")),
        default_sort: get_str("default_sort").map_or(SortMethod::TimeAddedNewestFirst, |v| {
            SortMethod::from_str(&v)
        }),
    }
}

/// Persist the given settings to the database.
pub fn save_settings(settings: &Settings) -> Result<(), SettingsError> {
    let conn = open_settings_db()?;

    let upsert = |key: &str, value: &str| -> Result<(), SettingsError> {
        conn.execute(
            "INSERT INTO settings (key, value) VALUES (?1, ?2)
             ON CONFLICT(key) DO UPDATE SET value = excluded.value",
            params![key, value],
        )
        .map_err(|e| SettingsError::Save {
            key: key.to_string(),
            detail: e.to_string(),
        })?;
        Ok(())
    };

    upsert(
        "library_folder",
        settings.library_folder.as_deref().unwrap_or(""),
    )?;
    upsert("scan_on_startup", &settings.scan_on_startup.to_string())?;
    upsert("default_view", &settings.default_view)?;
    upsert("default_sort", settings.default_sort.as_str())?;

    Ok(())
}
