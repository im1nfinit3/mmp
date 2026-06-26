use std::path::PathBuf;

use crate::library::song::{RepeatMode, Song};
use crate::settings::{Settings, SortMethod};

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum Page {
    RecentlyAdded,
    Songs,
    Albums,
    Artists,
    Queue,
    Settings,
    Playlist(i64),
}

#[derive(Clone, Debug)]
pub enum ActiveModal {
    RenamePlaylist { id: i64, name: String },
    CreatePlaylistAndAddSong { path: PathBuf, name: String },
    SaveQueueAsPlaylist { name: String },
    CreatePlaylistAndAddAllFiltered { name: String },
}

#[derive(Clone, Debug)]
pub enum AppEffect {
    OpenModal(ActiveModal),
    CloseModal,
    ShowNotification(String),
    SetPersistentStatus(Option<String>),
}

#[derive(Clone, Debug)]
pub enum AppIntent {
    PlayPause,
    Previous,
    Next,
    Seek(f64),
    SetVolume(f64),
    ToggleMute,
    ToggleShuffle,
    ToggleRepeat,
    SelectPage(Page),
    UpdateSongsSearch(String),
    UpdateAlbumsSearch(String),
    UpdateArtistsSearch(String),
    ActivateAlbum(String),
    ActivateArtist(String),
    PlaySong(PathBuf),
    QueueSong(PathBuf),
    OpenRenamePlaylist { id: i64 },
    OpenCreatePlaylistAndAddSong { path: PathBuf },
    OpenSaveQueueAsPlaylist,
    ConfirmRenamePlaylist { id: i64, name: String },
    ConfirmCreatePlaylistAndAddSong { name: String, path: PathBuf },
    ConfirmSaveQueueAsPlaylist(String),
    AddSongToPlaylist { playlist_id: i64, path: PathBuf },
    RemoveSongFromPlaylist { playlist_id: i64, path: PathBuf },
    RemoveFromQueue(usize),
    ClearQueue,
    DeletePlaylist(i64),
    QueueAllFiltered,
    AddAllFilteredToPlaylist(i64),
    QueuePlaylist(i64),
    OpenCreatePlaylistAndAddAllFiltered,
    ConfirmCreatePlaylistAndAddAllFiltered(String),
    ForceRescan,
    SetLibraryFolder(String),
    SetScanOnStartup(bool),
    SetDefaultView(String),
    SetDefaultSort(SortMethod),
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum PlaybackStatus {
    Playing,
    Paused,
    Stopped,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct SongView {
    pub path: PathBuf,
    pub title: String,
    pub artist: String,
    pub album: String,
    pub duration: String,
    pub is_current: bool,
    pub queue_index: Option<usize>,
}

#[derive(Clone, Debug)]
pub struct AppState {
    pub page: Page,
    pub playlists: Vec<crate::library::db::Playlist>,
    pub songs: Vec<SongView>,
    pub queue: Vec<SongView>,
    pub albums: Vec<String>,
    pub artists: Vec<String>,
    pub songs_search: String,
    pub albums_search: String,
    pub artists_search: String,
    pub current_track_label: String,
    pub current_song_title: String,
    pub current_song_artist: String,
    pub current_song_album: String,
    pub playback: PlaybackStatus,
    pub elapsed_seconds: f64,
    pub duration_seconds: f64,
    pub volume: f64,
    pub muted: bool,
    pub shuffle: bool,
    pub repeat: RepeatMode,
    pub scan_started: bool,
    pub total_songs: usize,
    pub settings: Settings,
    pub default_sort: SortMethod,
}

pub struct SongFilters {
    pub search: String,
    pub selected_artist: Option<String>,
    pub selected_album: Option<String>,
}

impl SongFilters {
    pub fn new() -> Self {
        Self {
            search: String::new(),
            selected_artist: None,
            selected_album: None,
        }
    }

    pub fn set_search(&mut self, value: String) {
        self.search = value;
        self.selected_artist = None;
        self.selected_album = None;
    }

    pub fn select_artist(&mut self, artist: String) {
        self.search = artist.clone();
        self.selected_artist = Some(artist);
        self.selected_album = None;
    }

    pub fn select_album(&mut self, album: String) {
        self.search = album.clone();
        self.selected_album = Some(album);
        self.selected_artist = None;
    }

    pub fn matches(&self, song: &Song) -> bool {
        let lowered = self.search.to_lowercase();
        if !lowered.is_empty() {
            let haystacks = [
                song.title.to_lowercase(),
                song.artist.to_lowercase(),
                song.album.to_lowercase(),
            ];

            if !haystacks.iter().any(|value| value.contains(&lowered)) {
                return false;
            }
        }

        if let Some(artist) = &self.selected_artist
            && &song.artist != artist
        {
            return false;
        }

        if let Some(album) = &self.selected_album
            && &song.album != album
        {
            return false;
        }

        true
    }
}
