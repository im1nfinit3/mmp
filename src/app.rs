use std::path::PathBuf;
use std::time::{Duration, Instant};

use iced::task::Task;
use iced::{
    Element, Point, Subscription, Theme, application, keyboard,
};

use crate::core::{
    ActiveModal, AppCore, AppEffect, AppIntent,
};

use crate::system_accent::{self, UiPalette};

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

pub fn run() -> iced::Result {
    let startup_palette = system_accent::load_startup_palette();

    application(move || boot(startup_palette), update, view)
        .title(title)
        .theme(theme)
        .subscription(subscription)
        .settings(iced::Settings::default())
        .window_size((1240.0, 820.0))
        .run()
}

// ---------------------------------------------------------------------------
// Status bar model
// ---------------------------------------------------------------------------

pub struct StatusBar {
    /// Persistent status that overrides the idle message (e.g. scanning).
    pub persistent: Option<String>,
    /// Transient notification with expiry instant; overrides everything while active.
    pub notification: Option<(String, Instant)>,
    /// Total library track count — always shown on the right.
    pub track_count: usize,
}

impl StatusBar {
    pub fn new() -> Self {
        Self {
            persistent: None,
            notification: None,
            track_count: 0,
        }
    }

    /// The current left-side text to display.
    pub fn display_text(&self) -> &str {
        if let Some((msg, _)) = &self.notification {
            return msg;
        }
        if let Some(msg) = &self.persistent {
            return msg;
        }
        "Ready"
    }

    /// Whether a transient notification is currently visible.
    pub fn has_active_notification(&self) -> bool {
        self.notification.is_some()
    }
}

// ---------------------------------------------------------------------------
// Top-level application model
// ---------------------------------------------------------------------------

pub struct App {
    pub core: AppCore,
    pub active_modal: Option<ActiveModal>,
    pub status_bar: StatusBar,
    pub context_menu: Option<ContextMenu>,
    pub cursor_position: Point,
    pub palette: UiPalette,
}

/// All events that can drive UI updates.
#[derive(Debug, Clone)]
pub enum Message {
    /// Periodic timer tick — used to poll playback position and process background events.
    Tick,
    Intent(AppIntent),
    OpenSongMenu {
        path: PathBuf,
        queue_index: Option<usize>,
    },
    OpenPlaylistMenu(i64),
    CloseContextMenu,
    CursorMoved(Point),
    ModalTextChanged(String),
    ModalConfirm,
    ModalCancel,
    ModifiersChanged(bool),
}

#[derive(Debug, Clone)]
pub enum ContextMenu {
    Song {
        path: PathBuf,
        position: Point,
        current_playlist_id: Option<i64>,
        queue_index: Option<usize>,
        playlists: Vec<SongPlaylistMenuItem>,
    },
    Playlist {
        id: i64,
        position: Point,
    },
}

#[derive(Clone, Debug)]
pub struct SongPlaylistMenuItem {
    pub id: i64,
    pub name: String,
    pub contains_song: bool,
}

// ---------------------------------------------------------------------------
// Iced lifecycle functions
// ---------------------------------------------------------------------------

fn boot(startup_palette: UiPalette) -> (App, Task<Message>) {
    (
        App {
            core: AppCore::new(),
            active_modal: None,
            status_bar: StatusBar::new(),
            context_menu: None,
            cursor_position: Point::ORIGIN,
            palette: startup_palette,
        },
        Task::none(),
    )
}

fn title(_app: &App) -> String {
    String::from("My Music Player (mmp)")
}

fn theme(_app: &App) -> Theme {
    Theme::Dark
}

fn subscription(_app: &App) -> Subscription<Message> {
    iced::Subscription::batch([
        iced::time::every(Duration::from_millis(200)).map(|_| Message::Tick),
        keyboard::listen().filter_map(|event| match event {
            keyboard::Event::ModifiersChanged(modifiers) => {
                Some(Message::ModifiersChanged(modifiers.shift()))
            }
            _ => None,
        }),
    ])
}

fn update(app: &mut App, message: Message) -> Task<Message> {
    if let Message::Tick = &message {
        if let Some((_, expires_at)) = &app.status_bar.notification {
            if Instant::now() >= *expires_at {
                app.status_bar.notification = None;
            }
        }
    }

    let effects = match message {
        Message::Tick => app.core.tick(),
        Message::Intent(intent) => {
            app.context_menu = None;
            app.core.handle_intent(intent)
        }
        Message::OpenSongMenu { path, queue_index } => {
            let playlist_memberships = app.core.song_playlist_memberships(&path);
            let current_playlist_id = app.core.current_playlist_id();
            let playlists = app
                .core
                .state()
                .playlists
                .iter()
                .map(|playlist| SongPlaylistMenuItem {
                    id: playlist.id,
                    name: playlist.name.clone(),
                    contains_song: playlist_memberships.contains(&playlist.id),
                })
                .collect();
            app.context_menu = Some(ContextMenu::Song {
                path,
                position: app.cursor_position,
                current_playlist_id,
                queue_index,
                playlists,
            });
            Vec::new()
        }
        Message::OpenPlaylistMenu(id) => {
            app.context_menu = Some(ContextMenu::Playlist {
                id,
                position: app.cursor_position,
            });
            Vec::new()
        }
        Message::CloseContextMenu => {
            app.context_menu = None;
            Vec::new()
        }
        Message::CursorMoved(point) => {
            app.cursor_position = point;
            Vec::new()
        }
        Message::ModalTextChanged(value) => {
            if let Some(modal) = app.active_modal.as_mut() {
                match modal {
                    ActiveModal::RenamePlaylist { name, .. }
                    | ActiveModal::CreatePlaylistAndAddSong { name, .. }
                    | ActiveModal::SaveQueueAsPlaylist { name }
                    | ActiveModal::CreatePlaylistAndAddAllFiltered { name } => *name = value,
                }
            }
            Vec::new()
        }
        Message::ModalConfirm => {
            let Some(modal) = app.active_modal.clone() else {
                return Task::none();
            };

            let should_close = match &modal {
                ActiveModal::RenamePlaylist { name, .. }
                | ActiveModal::CreatePlaylistAndAddSong { name, .. }
                | ActiveModal::SaveQueueAsPlaylist { name }
                | ActiveModal::CreatePlaylistAndAddAllFiltered { name } => !name.trim().is_empty(),
            };

            let mut effects = match modal {
                ActiveModal::RenamePlaylist { id, name } => app
                    .core
                    .handle_intent(AppIntent::ConfirmRenamePlaylist { id, name }),
                ActiveModal::CreatePlaylistAndAddSong { path, name } => app
                    .core
                    .handle_intent(AppIntent::ConfirmCreatePlaylistAndAddSong { name, path }),
                ActiveModal::SaveQueueAsPlaylist { name } => app
                    .core
                    .handle_intent(AppIntent::ConfirmSaveQueueAsPlaylist(name)),
                ActiveModal::CreatePlaylistAndAddAllFiltered { name } => app
                    .core
                    .handle_intent(AppIntent::ConfirmCreatePlaylistAndAddAllFiltered(name)),
            };

            if should_close
                && !effects
                    .iter()
                    .any(|effect| matches!(effect, AppEffect::OpenModal(_)))
            {
                effects.push(AppEffect::CloseModal);
            }

            effects
        }
        Message::ModalCancel => vec![AppEffect::CloseModal],
        Message::ModifiersChanged(shift_held) => {
            app.core.set_shift_held(shift_held);
            Vec::new()
        }
    };

    apply_effects(app, effects)
}

fn apply_effects(app: &mut App, effects: Vec<AppEffect>) -> Task<Message> {
    for effect in effects {
        match effect {
            AppEffect::OpenModal(modal) => app.active_modal = Some(modal),
            AppEffect::CloseModal => app.active_modal = None,
            AppEffect::ShowNotification(message) => {
                app.status_bar.notification = Some((
                    message,
                    Instant::now() + Duration::from_secs(4),
                ));
            }
            AppEffect::SetPersistentStatus(status) => {
                app.status_bar.persistent = status;
            }
        }
    }

    // Keep track count in sync
    app.status_bar.track_count = app.core.state().total_songs;

    Task::none()
}

fn view(app: &App) -> Element<'_, Message> {
    let state = app.core.state();
    let palette = &app.palette;

    let body = crate::ui::view_shell(state, palette, &app.status_bar);

    crate::ui::view(body, &app.context_menu, &app.active_modal, state, palette)
}
