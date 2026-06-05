use std::path::PathBuf;
use std::sync::mpsc;
use std::time::{Duration, Instant};

use crate::library::db::Playlist;
use crate::library::scan;
use crate::library::song::{RepeatMode, Song};
use crate::library::{LibraryEvent, LibraryHandle};
use crate::playback::{Playback, PlaybackEvent, PlaybackState, QueueState};

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
    CreatePlaylist { name: String },
    RenamePlaylist { id: i64, name: String },
    CreatePlaylistAndAddSong { path: PathBuf, name: String },
    SaveQueueAsPlaylist { name: String },
}

#[derive(Clone, Debug)]
pub enum AppEffect {
    OpenModal(ActiveModal),
    CloseModal,
    ShowNotification(String),
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
    OpenCreatePlaylist,
    OpenRenamePlaylist { id: i64 },
    OpenCreatePlaylistAndAddSong { path: PathBuf },
    OpenSaveQueueAsPlaylist,
    ConfirmCreatePlaylist(String),
    ConfirmRenamePlaylist { id: i64, name: String },
    ConfirmCreatePlaylistAndAddSong { name: String, path: PathBuf },
    ConfirmSaveQueueAsPlaylist(String),
    AddSongToPlaylist { playlist_id: i64, path: PathBuf },
    RemoveSongFromPlaylist { playlist_id: i64, path: PathBuf },
    RemoveFromQueue(usize),
    DeletePlaylist(i64),
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
    pub playlists: Vec<Playlist>,
    pub songs: Vec<SongView>,
    pub queue: Vec<SongView>,
    pub albums: Vec<String>,
    pub artists: Vec<String>,
    pub songs_search: String,
    pub albums_search: String,
    pub artists_search: String,
    pub current_track_label: String,
    pub playback: PlaybackStatus,
    pub elapsed_seconds: f64,
    pub duration_seconds: f64,
    pub volume: f64,
    pub muted: bool,
    pub shuffle: bool,
    pub repeat: RepeatMode,
    pub scan_started: bool,
    pub status_message: Option<String>,
}

struct SongFilters {
    search: String,
    selected_artist: Option<String>,
    selected_album: Option<String>,
}

impl SongFilters {
    fn new() -> Self {
        Self {
            search: String::new(),
            selected_artist: None,
            selected_album: None,
        }
    }

    fn set_search(&mut self, value: String) {
        self.search = value;
        self.selected_artist = None;
        self.selected_album = None;
    }

    fn select_artist(&mut self, artist: String) {
        self.search = artist.clone();
        self.selected_artist = Some(artist);
        self.selected_album = None;
    }

    fn select_album(&mut self, album: String) {
        self.search = album.clone();
        self.selected_album = Some(album);
        self.selected_artist = None;
    }

    fn matches(&self, song: &Song) -> bool {
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

pub struct AppCore {
    library_handle: LibraryHandle,
    library_rx: mpsc::Receiver<LibraryEvent>,
    library_event_tx: mpsc::Sender<LibraryEvent>,
    playback: Playback,
    playback_rx: mpsc::Receiver<PlaybackEvent>,
    queue: QueueState,
    all_songs: Vec<Song>,
    state: AppState,
    song_filters: SongFilters,
    started_at: Instant,
    startup_scan_requested: bool,
}

impl AppCore {
    pub fn new() -> Self {
        let (library_event_tx, library_rx) = mpsc::channel();
        let library_handle = crate::library::spawn(library_event_tx.clone());
        let (playback_tx, playback_rx) = mpsc::channel();
        let mut playback = Playback::new(playback_tx);
        playback.set_volume(0.7);

        let mut core = Self {
            library_handle,
            library_rx,
            library_event_tx,
            playback,
            playback_rx,
            queue: QueueState::new(),
            all_songs: Vec::new(),
            state: AppState {
                page: Page::RecentlyAdded,
                playlists: Vec::new(),
                songs: Vec::new(),
                queue: Vec::new(),
                albums: Vec::new(),
                artists: Vec::new(),
                songs_search: String::new(),
                albums_search: String::new(),
                artists_search: String::new(),
                current_track_label: String::from("No track selected"),
                playback: PlaybackStatus::Stopped,
                elapsed_seconds: 0.0,
                duration_seconds: 0.0,
                volume: 0.7,
                muted: false,
                shuffle: false,
                repeat: RepeatMode::Off,
                scan_started: false,
                status_message: None,
            },
            song_filters: SongFilters::new(),
            started_at: Instant::now(),
            startup_scan_requested: false,
        };
        core.refresh_library_views();
        core
    }

    pub fn state(&self) -> &AppState {
        &self.state
    }

    pub fn current_playlist_id(&self) -> Option<i64> {
        match self.state.page {
            Page::Playlist(id) => Some(id),
            _ => None,
        }
    }

    pub fn song_playlist_memberships(&self, path: &std::path::Path) -> Vec<i64> {
        self.state
            .playlists
            .iter()
            .filter_map(|playlist| {
                self.library_handle
                    .get_playlist_songs(playlist.id)
                    .iter()
                    .any(|song| song.path == path)
                    .then_some(playlist.id)
            })
            .collect()
    }

    pub fn tick(&mut self) -> Vec<AppEffect> {
        if !self.startup_scan_requested && self.started_at.elapsed() >= Duration::from_millis(300) {
            self.startup_scan_requested = true;
            self.state.scan_started = true;
            scan::start_scan(self.library_handle.clone(), self.library_event_tx.clone());
        }

        let mut effects = Vec::new();

        while let Ok(event) = self.playback_rx.try_recv() {
            effects.extend(self.handle_playback_event(event));
        }

        while let Ok(event) = self.library_rx.try_recv() {
            effects.extend(self.handle_library_event(event));
        }

        self.refresh_derived_views();
        effects
    }

    pub fn handle_intent(&mut self, intent: AppIntent) -> Vec<AppEffect> {
        let effects = match intent {
            AppIntent::PlayPause => {
                if self.queue.current.is_none() {
                    if self.queue.tracks.is_empty() {
                        Vec::new()
                    } else {
                        self.play_track_at(0)
                    }
                } else {
                    self.playback.toggle_pause();
                    Vec::new()
                }
            }
            AppIntent::Previous => {
                if let Some(current) = self.queue.current {
                    if current > 0 {
                        self.play_track_at(current - 1)
                    } else {
                        Vec::new()
                    }
                } else {
                    Vec::new()
                }
            }
            AppIntent::Next => self.advance_track(),
            AppIntent::Seek(seconds) => {
                self.playback.seek(seconds);
                Vec::new()
            }
            AppIntent::SetVolume(volume) => {
                self.state.volume = volume.clamp(0.0, 1.0);
                self.playback.set_volume(self.state.volume);
                if self.state.muted {
                    self.state.muted = false;
                    self.playback.set_mute(false);
                }
                Vec::new()
            }
            AppIntent::ToggleMute => {
                self.state.muted = !self.state.muted;
                self.playback.set_mute(self.state.muted);
                Vec::new()
            }
            AppIntent::ToggleShuffle => {
                self.queue.toggle_shuffle();
                self.state.shuffle = self.queue.shuffle;
                Vec::new()
            }
            AppIntent::ToggleRepeat => {
                self.queue.cycle_repeat();
                self.state.repeat = self.queue.repeat;
                Vec::new()
            }
            AppIntent::SelectPage(page) => {
                self.state.page = page;
                Vec::new()
            }
            AppIntent::UpdateSongsSearch(search) => {
                self.song_filters.set_search(search.clone());
                self.state.songs_search = search;
                Vec::new()
            }
            AppIntent::UpdateAlbumsSearch(search) => {
                self.state.albums_search = search;
                Vec::new()
            }
            AppIntent::UpdateArtistsSearch(search) => {
                self.state.artists_search = search;
                Vec::new()
            }
            AppIntent::ActivateAlbum(album) => {
                self.song_filters.select_album(album.clone());
                self.state.songs_search = album;
                self.state.page = Page::Songs;
                Vec::new()
            }
            AppIntent::ActivateArtist(artist) => {
                self.song_filters.select_artist(artist.clone());
                self.state.songs_search = artist;
                self.state.page = Page::Songs;
                Vec::new()
            }
            AppIntent::PlaySong(path) => {
                let index = self.queue.push(path);
                self.play_track_at(index)
            }
            AppIntent::QueueSong(path) => {
                self.queue.push(path);
                Vec::new()
            }
            AppIntent::OpenCreatePlaylist => {
                vec![AppEffect::OpenModal(ActiveModal::CreatePlaylist {
                    name: String::new(),
                })]
            }
            AppIntent::OpenRenamePlaylist { id } => {
                let name = self
                    .state
                    .playlists
                    .iter()
                    .find(|playlist| playlist.id == id)
                    .map(|playlist| playlist.name.clone())
                    .unwrap_or_default();
                vec![AppEffect::OpenModal(ActiveModal::RenamePlaylist { id, name })]
            }
            AppIntent::OpenCreatePlaylistAndAddSong { path } => {
                vec![AppEffect::OpenModal(ActiveModal::CreatePlaylistAndAddSong {
                    path,
                    name: String::new(),
                })]
            }
            AppIntent::OpenSaveQueueAsPlaylist => {
                vec![AppEffect::OpenModal(ActiveModal::SaveQueueAsPlaylist {
                    name: String::new(),
                })]
            }
            AppIntent::ConfirmCreatePlaylist(name) => self.create_playlist(&name),
            AppIntent::ConfirmRenamePlaylist { id, name } => self.rename_playlist(id, &name),
            AppIntent::ConfirmCreatePlaylistAndAddSong { name, path } => {
                self.create_playlist_and_add_song(&name, path)
            }
            AppIntent::ConfirmSaveQueueAsPlaylist(name) => self.save_queue_as_playlist(&name),
            AppIntent::AddSongToPlaylist { playlist_id, path } => {
                self.library_handle.add_to_playlist(playlist_id, path);
                vec![AppEffect::ShowNotification(String::from(
                    "Song added to playlist",
                ))]
            }
            AppIntent::RemoveSongFromPlaylist { playlist_id, path } => {
                self.library_handle.remove_from_playlist(playlist_id, path);
                vec![AppEffect::ShowNotification(String::from(
                    "Song removed from playlist",
                ))]
            }
            AppIntent::RemoveFromQueue(index) => self.remove_from_queue(index),
            AppIntent::DeletePlaylist(id) => {
                self.library_handle.delete_playlist(id);
                if self.state.page == Page::Playlist(id) {
                    self.state.page = Page::Songs;
                }
                Vec::new()
            }
        };

        self.refresh_derived_views();
        effects
    }

    fn handle_playback_event(&mut self, event: PlaybackEvent) -> Vec<AppEffect> {
        match event {
            PlaybackEvent::Position { elapsed, duration } => {
                self.state.elapsed_seconds = elapsed;
                self.state.duration_seconds = duration;
                Vec::new()
            }
            PlaybackEvent::EndOfStream => self.advance_track(),
            PlaybackEvent::Tags { title, artist } => {
                self.state.current_track_label = match (title, artist) {
                    (Some(title), Some(artist)) if !artist.is_empty() => {
                        format!("{title} - {artist}")
                    }
                    (Some(title), _) => title,
                    _ => self.current_track_label_for_path(),
                };
                Vec::new()
            }
            PlaybackEvent::Error(error) => {
                let mut effects = vec![AppEffect::ShowNotification(error)];
                effects.extend(self.advance_track());
                effects
            }
            PlaybackEvent::StateChanged(state) => {
                self.state.playback = match state {
                    PlaybackState::Playing => PlaybackStatus::Playing,
                    PlaybackState::Paused => PlaybackStatus::Paused,
                    PlaybackState::Stopped => PlaybackStatus::Stopped,
                };
                Vec::new()
            }
        }
    }

    fn handle_library_event(&mut self, event: LibraryEvent) -> Vec<AppEffect> {
        match event {
            LibraryEvent::SongsLoaded { songs } => {
                self.all_songs = songs;
                self.refresh_library_views();
                Vec::new()
            }
            LibraryEvent::SongsAdded { songs } => {
                for song in songs {
                    if let Some(existing) = self.all_songs.iter_mut().find(|item| item.path == song.path)
                    {
                        *existing = song;
                    } else {
                        self.all_songs.push(song);
                    }
                }
                self.refresh_library_views();
                Vec::new()
            }
            LibraryEvent::PlaylistsChanged => {
                self.state.playlists = self.library_handle.get_playlists();
                self.refresh_derived_views();
                Vec::new()
            }
            LibraryEvent::ScanStarted => {
                self.state.scan_started = true;
                vec![AppEffect::ShowNotification(String::from("Scanning music library..."))]
            }
            LibraryEvent::ScanComplete { total } => {
                self.refresh_library_views();
                vec![AppEffect::ShowNotification(format!(
                    "Scan complete. {total} new tracks indexed."
                ))]
            }
            LibraryEvent::Error(error) => vec![AppEffect::ShowNotification(error)],
        }
    }

    fn refresh_library_views(&mut self) {
        self.state.playlists = self.library_handle.get_playlists();
        self.refresh_derived_views();
    }

    fn refresh_derived_views(&mut self) {
        self.state.albums = self
            .library_handle
            .get_unique_albums()
            .into_iter()
            .filter(|album| album.to_lowercase().contains(&self.state.albums_search.to_lowercase()))
            .collect();
        self.state.artists = self
            .library_handle
            .get_unique_artists()
            .into_iter()
            .filter(|artist| {
                artist
                    .to_lowercase()
                    .contains(&self.state.artists_search.to_lowercase())
            })
            .collect();
        self.state.songs = self.current_song_list();
        self.state.queue = self.current_queue_list();
        self.state.shuffle = self.queue.shuffle;
        self.state.repeat = self.queue.repeat;
    }

    fn current_song_list(&self) -> Vec<SongView> {
        let songs = match self.state.page {
            Page::Playlist(id) => self.library_handle.get_playlist_songs(id),
            Page::RecentlyAdded => {
                let mut songs = self.all_songs.clone();
                songs.reverse();
                songs
            }
            _ => self.all_songs.clone(),
        };

        songs.into_iter()
            .filter(|song| self.song_filters.matches(song))
            .map(|song| self.to_song_view(song))
            .collect()
    }

    fn current_queue_list(&self) -> Vec<SongView> {
        self.queue
            .tracks
            .iter()
            .enumerate()
            .map(|(index, path)| {
                let song = self
                    .all_songs
                    .iter()
                    .find(|song| song.path == *path)
                    .cloned()
                    .unwrap_or_else(|| Song::new(path.clone()));

                let mut view = self.to_song_view(song);
                view.is_current = self.queue.current == Some(index);
                view.queue_index = Some(index);
                view
            })
            .collect()
    }

    fn to_song_view(&self, song: Song) -> SongView {
        SongView {
            is_current: self.current_track_path().as_ref() == Some(&song.path),
            path: song.path,
            title: song.title,
            artist: song.artist,
            album: song.album,
            duration: song.duration_str,
            queue_index: None,
        }
    }

    fn current_track_path(&self) -> Option<PathBuf> {
        self.queue
            .current
            .and_then(|index| self.queue.tracks.get(index).cloned())
    }

    fn current_track_label_for_path(&self) -> String {
        self.current_track_path()
            .and_then(|path| self.all_songs.iter().find(|song| song.path == path).cloned())
            .map(|song| {
                if song.artist.is_empty() {
                    song.title
                } else {
                    format!("{} - {}", song.title, song.artist)
                }
            })
            .unwrap_or_else(|| String::from("No track selected"))
    }

    fn play_track_at(&mut self, index: usize) -> Vec<AppEffect> {
        let Some(path) = self.queue.tracks.get(index).cloned() else {
            return Vec::new();
        };

        self.queue.current = Some(index);
        self.playback.play_file(&path);
        self.state.current_track_label = self.current_track_label_for_path();
        self.state.elapsed_seconds = 0.0;
        self.state.duration_seconds = 0.0;
        self.refresh_derived_views();
        Vec::new()
    }

    fn advance_track(&mut self) -> Vec<AppEffect> {
        if let Some(next) = self.queue.next_track() {
            self.play_track_at(next)
        } else {
            self.playback.stop();
            self.queue.current = None;
            self.state.current_track_label = String::from("No track selected");
            self.state.elapsed_seconds = 0.0;
            self.state.duration_seconds = 0.0;
            self.refresh_derived_views();
            Vec::new()
        }
    }

    fn create_playlist(&mut self, name: &str) -> Vec<AppEffect> {
        let trimmed = name.trim();
        if trimmed.is_empty() {
            return vec![AppEffect::ShowNotification(String::from(
                "Playlist name cannot be empty",
            ))];
        }

        match self.library_handle.create_playlist(trimmed) {
            Ok(id) => {
                self.state.playlists = self.library_handle.get_playlists();
                self.state.page = Page::Playlist(id);
                vec![AppEffect::ShowNotification(format!(
                    "Created playlist \"{trimmed}\""
                ))]
            }
            Err(error) => vec![AppEffect::ShowNotification(error)],
        }
    }

    fn rename_playlist(&mut self, id: i64, name: &str) -> Vec<AppEffect> {
        let trimmed = name.trim();
        if trimmed.is_empty() {
            return vec![AppEffect::ShowNotification(String::from(
                "Playlist name cannot be empty",
            ))];
        }

        self.library_handle.rename_playlist(id, trimmed);
        self.state.playlists = self.library_handle.get_playlists();
        vec![AppEffect::ShowNotification(format!(
            "Renamed playlist to \"{trimmed}\""
        ))]
    }

    fn create_playlist_and_add_song(&mut self, name: &str, path: PathBuf) -> Vec<AppEffect> {
        let trimmed = name.trim();
        if trimmed.is_empty() {
            return vec![AppEffect::ShowNotification(String::from(
                "Playlist name cannot be empty",
            ))];
        }

        match self.library_handle.create_playlist(trimmed) {
            Ok(id) => {
                self.library_handle.add_to_playlist(id, path);
                self.state.playlists = self.library_handle.get_playlists();
                self.state.page = Page::Playlist(id);
                vec![AppEffect::ShowNotification(format!(
                    "Created playlist \"{trimmed}\""
                ))]
            }
            Err(error) => vec![AppEffect::ShowNotification(error)],
        }
    }

    fn save_queue_as_playlist(&mut self, name: &str) -> Vec<AppEffect> {
        let trimmed = name.trim();
        if trimmed.is_empty() {
            return vec![AppEffect::ShowNotification(String::from(
                "Playlist name cannot be empty",
            ))];
        }

        match self.library_handle.create_playlist(trimmed) {
            Ok(id) => {
                for path in &self.queue.tracks {
                    self.library_handle.add_to_playlist(id, path.clone());
                }
                self.state.playlists = self.library_handle.get_playlists();
                self.state.page = Page::Playlist(id);
                vec![AppEffect::ShowNotification(format!(
                    "Saved queue as \"{trimmed}\""
                ))]
            }
            Err(error) => vec![AppEffect::ShowNotification(error)],
        }
    }

    fn remove_from_queue(&mut self, index: usize) -> Vec<AppEffect> {
        if index >= self.queue.tracks.len() {
            return Vec::new();
        }

        let removed_current = self.queue.current == Some(index);
        self.queue.remove(index);

        if removed_current {
            if let Some(current) = self.queue.current {
                let mut effects = self.play_track_at(current);
                effects.push(AppEffect::ShowNotification(String::from(
                    "Song removed from queue",
                )));
                return effects;
            }

            self.playback.stop();
            self.state.current_track_label = String::from("No track selected");
            self.state.elapsed_seconds = 0.0;
            self.state.duration_seconds = 0.0;
        }

        self.refresh_derived_views();
        vec![AppEffect::ShowNotification(String::from(
            "Song removed from queue",
        ))]
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn song(title: &str, artist: &str, album: &str) -> Song {
        Song {
            path: PathBuf::from(format!("{title}.mp3")),
            title: title.to_string(),
            artist: artist.to_string(),
            album: album.to_string(),
            duration_str: String::from("3:00"),
        }
    }

    #[test]
    fn song_filters_match_search_text() {
        let mut filters = SongFilters::new();
        filters.set_search(String::from("bowie"));

        assert!(filters.matches(&song("Heroes", "David Bowie", "Heroes")));
        assert!(!filters.matches(&song("Paranoid Android", "Radiohead", "OK Computer")));
    }

    #[test]
    fn song_filters_can_lock_to_album() {
        let mut filters = SongFilters::new();
        filters.select_album(String::from("Discovery"));

        assert!(filters.matches(&song("One More Time", "Daft Punk", "Discovery")));
        assert!(!filters.matches(&song("Digital Love", "Daft Punk", "Homework")));
        assert_eq!(filters.search, "Discovery");
    }

    #[test]
    fn song_filters_can_lock_to_artist() {
        let mut filters = SongFilters::new();
        filters.select_artist(String::from("Björk"));

        assert!(filters.matches(&song("Jóga", "Björk", "Homogenic")));
        assert!(!filters.matches(&song("Teardrop", "Massive Attack", "Mezzanine")));
        assert_eq!(filters.search, "Björk");
    }
}
