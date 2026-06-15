use std::collections::HashMap;
use std::path::PathBuf;
use std::sync::{LazyLock, Mutex};
use std::time::Duration;

use iced::task::Task;
use iced::widget::{
    Space, button, column, container, mouse_area, opaque, row, scrollable, slider, stack, svg,
    text, text_input,
};
use iced::{
    Alignment, Background, Border, Color, Element, Length, Point, Shadow, Subscription, Theme,
    application, keyboard,
    widget::{pick_list, toggler},
};

use crate::app_core::{
    ActiveModal, AppCore, AppEffect, AppIntent, AppState, Page, PlaybackStatus, SongView,
};
use crate::library::song::RepeatMode;
use crate::settings;
use crate::system_accent::{self, UiPalette};

const ICON_PREVIOUS: &[u8] = include_bytes!("icons/previous.svg");
const ICON_PLAY: &[u8] = include_bytes!("icons/play.svg");
const ICON_PAUSE: &[u8] = include_bytes!("icons/pause.svg");
const ICON_NEXT: &[u8] = include_bytes!("icons/next.svg");
const ICON_VOLUME_HIGH: &[u8] = include_bytes!("icons/volume-high.svg");
const ICON_VOLUME_MUTE: &[u8] = include_bytes!("icons/volume-mute.svg");
const ICON_SHUFFLE_OFF: &[u8] = include_bytes!("icons/shuffle-off.svg");
const ICON_SHUFFLE_ON: &[u8] = include_bytes!("icons/shuffle.svg");
const ICON_REPEAT_ALL: &[u8] = include_bytes!("icons/repeat-all.svg");
const ICON_REPEAT_ONE: &[u8] = include_bytes!("icons/repeat-one.svg");

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

struct App {
    core: AppCore,
    active_modal: Option<ActiveModal>,
    notification: Option<String>,
    context_menu: Option<ContextMenu>,
    cursor_position: Point,
    palette: UiPalette,
}

#[derive(Debug, Clone)]
enum Message {
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
    ClearNotification,
    ModifiersChanged(bool),
}

fn boot(startup_palette: UiPalette) -> (App, Task<Message>) {
    (
        App {
            core: AppCore::new(),
            active_modal: None,
            notification: None,
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
                    ActiveModal::CreatePlaylist { name }
                    | ActiveModal::RenamePlaylist { name, .. }
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
                ActiveModal::CreatePlaylist { name }
                | ActiveModal::RenamePlaylist { name, .. }
                | ActiveModal::CreatePlaylistAndAddSong { name, .. }
                | ActiveModal::SaveQueueAsPlaylist { name }
                | ActiveModal::CreatePlaylistAndAddAllFiltered { name } => !name.trim().is_empty(),
            };

            let mut effects = match modal {
                ActiveModal::CreatePlaylist { name } => app
                    .core
                    .handle_intent(AppIntent::ConfirmCreatePlaylist(name)),
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
        Message::ClearNotification => {
            app.notification = None;
            Vec::new()
        }
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
            AppEffect::ShowNotification(message) => app.notification = Some(message),
        }
    }

    Task::none()
}

fn view(app: &App) -> Element<'_, Message> {
    let state = app.core.state();
    let palette = &app.palette;

    let body = mouse_area(
        container(
            column![
                view_header(state, app.notification.as_deref(), palette),
                row![view_nav(state, palette), view_content(state, palette)]
                    .spacing(18)
                    .height(Length::Fill)
            ]
            .spacing(18)
            .height(Length::Fill),
        )
        .padding([22, 24])
        .width(Length::Fill)
        .height(Length::Fill)
        .style(app_shell_style),
    )
    .on_move(Message::CursorMoved);

    let mut layers: Vec<Element<'_, Message>> = vec![opaque(body)];

    if let Some(context_menu) = &app.context_menu {
        layers.push(opaque(view_context_menu_overlay(context_menu)));
    }

    if let Some(modal) = &app.active_modal {
        layers.push(opaque(view_modal_overlay(state, modal, palette)));
    }

    stack(layers)
        .width(Length::Fill)
        .height(Length::Fill)
        .into()
}

fn view_header<'a>(
    state: &'a AppState,
    notification: Option<&'a str>,
    palette: &'a UiPalette,
) -> Element<'a, Message> {
    let play_icon = match state.playback {
        PlaybackStatus::Playing => ICON_PAUSE,
        PlaybackStatus::Paused | PlaybackStatus::Stopped => ICON_PLAY,
    };

    let controls = row![
        icon_button(ICON_PREVIOUS, Message::Intent(AppIntent::Previous)),
        icon_button(play_icon, Message::Intent(AppIntent::PlayPause)),
        icon_button(ICON_NEXT, Message::Intent(AppIntent::Next)),
        toggle_icon_button(
            if state.shuffle {
                ICON_SHUFFLE_ON
            } else {
                ICON_SHUFFLE_OFF
            },
            state.shuffle,
            Message::Intent(AppIntent::ToggleShuffle),
            palette,
        ),
        toggle_icon_button(
            match state.repeat {
                RepeatMode::Off | RepeatMode::All => ICON_REPEAT_ALL,
                RepeatMode::One => ICON_REPEAT_ONE,
            },
            state.repeat != RepeatMode::Off,
            Message::Intent(AppIntent::ToggleRepeat),
            palette,
        ),
    ]
    .spacing(10)
    .width(336);

    let progress = column![
        text(&state.current_track_label).size(20),
        row![
            text(format_time(state.elapsed_seconds)).size(14).width(56),
            slider(
                0.0..=state.duration_seconds.max(1.0),
                state.elapsed_seconds.min(state.duration_seconds.max(1.0)),
                |value| Message::Intent(AppIntent::Seek(value))
            )
            .step(1.0)
            .width(Length::Fill)
            .style({
                let palette = *palette;
                move |theme, status| slider_style(theme, status, &palette)
            }),
            text(format_time(state.duration_seconds)).size(14).width(56),
        ]
        .spacing(10)
        .align_y(Alignment::Center)
    ]
    .spacing(12)
    .width(Length::Fill);

    let volume = row![
        slider(0.0..=1.0, state.volume, |value| {
            Message::Intent(AppIntent::SetVolume(value))
        })
        .step(0.01)
        .width(140)
        .style({
            let palette = *palette;
            move |theme, status| slider_style(theme, status, &palette)
        }),
        icon_button(
            if state.muted {
                ICON_VOLUME_MUTE
            } else {
                ICON_VOLUME_HIGH
            },
            Message::Intent(AppIntent::ToggleMute)
        )
    ]
    .spacing(12)
    .align_y(Alignment::Center)
    .width(236);

    let mut header = column![
        row![controls, progress, volume]
            .spacing(22)
            .align_y(Alignment::Center)
    ]
    .spacing(14);

    if let Some(note) = notification.or(state.status_message.as_deref()) {
        header = header.push(
            row![
                text(note).size(13),
                button("Dismiss")
                    .padding([6, 10])
                    .style(ghost_button_style)
                    .on_press(Message::ClearNotification)
            ]
            .spacing(12)
            .align_y(Alignment::Center),
        );
    }

    container(header)
        .padding([20, 22])
        .style(header_panel_style)
        .width(Length::Fill)
        .into()
}

fn view_nav<'a>(state: &'a AppState, palette: &'a UiPalette) -> Element<'a, Message> {
    let static_pages = [
        (Page::RecentlyAdded, "Recently added"),
        (Page::Albums, "Albums"),
        (Page::Artists, "Artists"),
        (Page::Songs, "Songs"),
        (Page::Queue, "Queue"),
    ];

    let mut nav = column![].spacing(8).width(240);

    for (page, label) in static_pages {
        nav = nav.push(page_button(state, page, label, palette));
    }

    nav = nav.push(Space::new().height(28));
    nav = nav.push(text("PLAYLISTS").size(13).color(COLOR_DIM));

    for playlist in &state.playlists {
        let is_active = matches!(state.page, Page::Playlist(id) if id == playlist.id);

        let base_row = mouse_area(
            button(text(&playlist.name).size(16))
                .width(Length::Fill)
                .padding([13, 14])
                .style({
                    let palette = *palette;
                    move |theme, status| nav_button_style(theme, status, is_active, &palette)
                })
                .on_press(Message::Intent(AppIntent::SelectPage(Page::Playlist(
                    playlist.id,
                )))),
        )
        .on_right_press(Message::OpenPlaylistMenu(playlist.id));

        nav = nav.push(base_row);
    }

    nav = nav.push(Space::new().height(16));
    nav = nav.push(
        button("Settings")
            .width(Length::Fill)
            .padding([13, 14])
            .style({
                let palette = *palette;
                move |theme, status| {
                    nav_button_style(theme, status, state.page == Page::Settings, &palette)
                }
            })
            .on_press(Message::Intent(AppIntent::SelectPage(Page::Settings))),
    );

    container(scrollable(nav))
        .width(260)
        .height(Length::Fill)
        .padding(16)
        .style(nav_panel_style)
        .into()
}

fn page_button<'a>(
    state: &'a AppState,
    page: Page,
    label: &'a str,
    palette: &'a UiPalette,
) -> Element<'a, Message> {
    let selected = state.page == page;

    button(text(label).size(16))
        .width(Length::Fill)
        .padding([13, 14])
        .style({
            let palette = *palette;
            move |theme, status| nav_button_style(theme, status, selected, &palette)
        })
        .on_press(Message::Intent(AppIntent::SelectPage(page)))
        .into()
}

fn view_content<'a>(state: &'a AppState, palette: &'a UiPalette) -> Element<'a, Message> {
    let content: Element<'a, Message> = match &state.page {
        Page::RecentlyAdded => view_song_list(
            "Recently added",
            "Search songs",
            &state.songs_search,
            &state.songs,
            palette,
        ),
        Page::Songs => view_song_list(
            "Songs",
            "Search songs",
            &state.songs_search,
            &state.songs,
            palette,
        ),
        Page::Playlist(id) => {
            let title = state
                .playlists
                .iter()
                .find(|playlist| playlist.id == *id)
                .map(|playlist| playlist.name.as_str())
                .unwrap_or("Playlist");
            view_song_list(
                title,
                "Filter playlist",
                &state.songs_search,
                &state.songs,
                palette,
            )
        }
        Page::Albums => view_string_list(
            "Albums",
            "Search albums",
            &state.albums_search,
            &state.albums,
            palette,
            |value| Message::Intent(AppIntent::UpdateAlbumsSearch(value)),
            |value| Message::Intent(AppIntent::ActivateAlbum(value)),
        ),
        Page::Artists => view_string_list(
            "Artists",
            "Search artists",
            &state.artists_search,
            &state.artists,
            palette,
            |value| Message::Intent(AppIntent::UpdateArtistsSearch(value)),
            |value| Message::Intent(AppIntent::ActivateArtist(value)),
        ),
        Page::Queue => view_queue(state, palette),
        Page::Settings => view_settings(state, palette),
    };

    container(content)
        .width(Length::Fill)
        .height(Length::Fill)
        .padding([18, 20])
        .style(content_panel_style)
        .into()
}

fn view_song_list<'a>(
    title: &'a str,
    placeholder: &'a str,
    search: &'a str,
    songs: &'a [SongView],
    palette: &'a UiPalette,
) -> Element<'a, Message> {
    let mut list = column![
        row![
            text(title).size(22),
            Space::new().width(Length::Fill),
            text_input(placeholder, search)
                .on_input(|value| Message::Intent(AppIntent::UpdateSongsSearch(value)))
                .padding([10, 14])
                .width(360)
                .style({
                    let palette = *palette;
                    move |theme, status| search_input_style(theme, status, &palette)
                })
        ]
        .align_y(Alignment::Center)
    ]
    .spacing(18)
    .width(Length::Fill);

    if songs.is_empty() {
        list = list.push(
            container(text("No tracks yet").size(15).color(COLOR_DIM))
                .padding(24)
                .style(song_list_panel_style),
        );
    } else {
        let mut rows = column![].spacing(8);
        for song in songs {
            rows = rows.push(song_row(song, palette));
        }

        list = list.push(
            container(scrollable(rows))
                .padding(10)
                .height(Length::Fill)
                .style(song_list_panel_style),
        );
    }

    list.into()
}

fn song_row<'a>(song: &'a SongView, palette: &'a UiPalette) -> Element<'a, Message> {
    let title = if song.is_current {
        format!("▶ {}", song.title)
    } else {
        song.title.clone()
    };

    mouse_area(
        container(
            row![
                button(
                    column![
                        text(title).size(17),
                        row![
                            text(&song.artist).size(13).color(COLOR_DIM),
                            text("•").size(13).color(COLOR_DIM),
                            text(&song.album).size(13).color(COLOR_DIM),
                        ]
                        .spacing(8)
                    ]
                    .spacing(6)
                )
                .style(plain_button_style)
                .width(Length::Fill)
                .padding(0)
                .on_press(Message::Intent(AppIntent::PlaySong(song.path.clone()))),
                text(&song.duration).size(14).color(COLOR_DIM).width(56),
            ]
            .spacing(12)
            .align_y(Alignment::Center),
        )
        .padding([14, 16])
        .width(Length::Fill)
        .style({
            let palette = *palette;
            move |theme| song_row_style(theme, song.is_current, &palette)
        }),
    )
    .on_right_press(Message::OpenSongMenu {
        path: song.path.clone(),
        queue_index: song.queue_index,
    })
    .into()
}

fn view_string_list<'a, FSearch, FActivate>(
    title: &'a str,
    placeholder: &'a str,
    search: &'a str,
    values: &'a [String],
    palette: &'a UiPalette,
    on_search: FSearch,
    on_activate: FActivate,
) -> Element<'a, Message>
where
    FSearch: Fn(String) -> Message + 'static + Copy,
    FActivate: Fn(String) -> Message + 'static + Copy,
{
    let mut rows = column![
        row![
            text(title).size(22),
            Space::new().width(Length::Fill),
            text_input(placeholder, search)
                .on_input(on_search)
                .padding([10, 14])
                .width(360)
                .style({
                    let palette = *palette;
                    move |theme, status| search_input_style(theme, status, &palette)
                })
        ]
        .align_y(Alignment::Center)
    ]
    .spacing(18);

    let mut list = column![].spacing(8);
    for value in values {
        list = list.push(
            button(text(value).size(16))
                .width(Length::Fill)
                .padding([14, 16])
                .style({
                    let palette = *palette;
                    move |theme, status| list_button_style(theme, status, &palette)
                })
                .on_press(on_activate(value.clone())),
        );
    }

    rows = rows.push(
        container(scrollable(list))
            .padding(10)
            .height(Length::Fill)
            .style(song_list_panel_style),
    );

    rows.into()
}

fn view_queue<'a>(state: &'a AppState, palette: &'a UiPalette) -> Element<'a, Message> {
    let mut list = column![
        row![
            text("Queue").size(22),
            Space::new().width(Length::Fill),
            button("Save queue as playlist")
                .padding([10, 14])
                .style(ghost_button_style)
                .on_press(Message::Intent(AppIntent::OpenSaveQueueAsPlaylist))
        ]
        .align_y(Alignment::Center)
    ]
    .spacing(18);

    if state.queue.is_empty() {
        list = list.push(
            container(text("Queue is empty").size(15).color(COLOR_DIM))
                .padding(24)
                .style(song_list_panel_style),
        );
    } else {
        let mut rows = column![].spacing(8);
        for song in &state.queue {
            rows = rows.push(song_row(song, palette));
        }

        list = list.push(
            container(scrollable(rows))
                .padding(10)
                .height(Length::Fill)
                .style(song_list_panel_style),
        );
    }

    list.into()
}

fn view_settings<'a>(state: &'a AppState, palette: &'a UiPalette) -> Element<'a, Message> {
    let folder_value = state
        .settings
        .library_folder
        .as_deref()
        .unwrap_or("")
        .to_string();

    let scan_toggle = toggler(state.settings.scan_on_startup)
        .label("Scan music library on startup")
        .on_toggle(|value| Message::Intent(AppIntent::SetScanOnStartup(value)))
        .spacing(10)
        .style({
            let palette = *palette;
            move |theme, status| toggler_style(theme, status, &palette)
        });

    const SORT_LABELS: &[&str] = &[
        "Alphabetical (artist)",
        "Alphabetical (album)",
        "Alphabetical (title)",
        "Time added (newest first)",
        "Time added (oldest first)",
    ];

    container(
        column![
            text("Settings").size(22),
            Space::new().height(16),
            text("Library folder").size(16).color(COLOR_DIM),
            text_input("Default: ~/Music (or your system audio dir)", &folder_value,)
                .on_input(|value| Message::Intent(AppIntent::SetLibraryFolder(value)))
                .padding([10, 14])
                .width(600)
                .style({
                    let palette = *palette;
                    move |theme, status| search_input_style(theme, status, &palette)
                }),
            text("Leave empty to use your system's default Music directory.",)
                .size(13)
                .color(COLOR_DIM),
            Space::new().height(16),
            scan_toggle,
            Space::new().height(16),
            text("Default view").size(16).color(COLOR_DIM),
            row![
                pick_list(
                    settings::Settings::ALL_VIEWS,
                    Some(state.settings.default_view.as_str()),
                    |selected| Message::Intent(AppIntent::SetDefaultView(selected.to_string())),
                )
                .padding([10, 14])
                .width(360)
                .style({
                    let palette = *palette;
                    move |theme, status| pick_list_style(theme, status, &palette)
                })
                .menu_style({
                    let palette = *palette;
                    move |theme| menu_style(theme, &palette)
                }),
                Space::new().width(Length::Fill),
            ],
            Space::new().height(16),
            text("Default sort").size(16).color(COLOR_DIM),
            text("Applies to Songs and Playlist views")
                .size(13)
                .color(COLOR_DIM),
            row![
                pick_list(
                    SORT_LABELS,
                    Some(state.settings.default_sort.label()),
                    |selected| {
                        let sort = crate::settings::SortMethod::ALL
                            .iter()
                            .find(|s| s.label() == selected)
                            .copied()
                            .unwrap_or(crate::settings::SortMethod::TimeAddedNewestFirst);
                        Message::Intent(AppIntent::SetDefaultSort(sort))
                    },
                )
                .padding([10, 14])
                .width(360)
                .style({
                    let palette = *palette;
                    move |theme, status| pick_list_style(theme, status, &palette)
                })
                .menu_style({
                    let palette = *palette;
                    move |theme| menu_style(theme, &palette)
                }),
                Space::new().width(Length::Fill),
            ],
            Space::new().height(24),
            text("About").size(16).color(COLOR_DIM),
            text("mmp — A native Rust music player built with Iced")
                .size(14)
                .color(COLOR_DIM),
        ]
        .spacing(10),
    )
    .padding([18, 20])
    .style(content_panel_style)
    .into()
}

fn view_modal_overlay<'a>(
    _state: &'a AppState,
    modal: &'a ActiveModal,
    palette: &'a UiPalette,
) -> Element<'a, Message> {
    let title = match modal {
        ActiveModal::CreatePlaylist { .. } => "Create playlist",
        ActiveModal::RenamePlaylist { .. } => "Rename playlist",
        ActiveModal::CreatePlaylistAndAddSong { .. } => "Create playlist and add song",
        ActiveModal::SaveQueueAsPlaylist { .. } => "Save queue as playlist",
        ActiveModal::CreatePlaylistAndAddAllFiltered { .. } => {
            "Create playlist and add all matching"
        }
    };

    let body: Element<'a, Message> = match modal {
        ActiveModal::CreatePlaylist { name }
        | ActiveModal::RenamePlaylist { name, .. }
        | ActiveModal::CreatePlaylistAndAddSong { name, .. }
        | ActiveModal::SaveQueueAsPlaylist { name }
        | ActiveModal::CreatePlaylistAndAddAllFiltered { name } => column![
            text_input("Playlist name", name)
                .on_input(Message::ModalTextChanged)
                .on_submit(Message::ModalConfirm)
                .padding([10, 14])
                .width(Length::Fill)
                .style({
                    let palette = *palette;
                    move |theme, status| search_input_style(theme, status, &palette)
                })
        ]
        .spacing(12)
        .into(),
    };

    container(
        container(
            column![
                text(title).size(22),
                body,
                row![
                    Space::new().width(Length::Fill),
                    button("Cancel")
                        .padding([10, 14])
                        .style(ghost_button_style)
                        .on_press(Message::ModalCancel),
                    button("Confirm")
                        .padding([10, 14])
                        .style(control_button_style)
                        .on_press(Message::ModalConfirm)
                ]
                .spacing(10)
            ]
            .spacing(16),
        )
        .padding(22)
        .width(420)
        .style(modal_panel_style),
    )
    .width(Length::Fill)
    .height(Length::Fill)
    .center_x(Length::Fill)
    .center_y(Length::Fill)
    .style(modal_backdrop_style)
    .into()
}

fn icon_button<'a>(icon: &'static [u8], message: Message) -> iced::widget::Button<'a, Message> {
    button(icon_svg(icon))
        .padding(10)
        .width(44)
        .height(44)
        .style(control_button_style)
        .on_press(message)
}

fn toggle_icon_button<'a>(
    icon: &'static [u8],
    active: bool,
    message: Message,
    palette: &'a UiPalette,
) -> iced::widget::Button<'a, Message> {
    button(icon_svg(icon))
        .padding(10)
        .width(44)
        .height(44)
        .style({
            let palette = *palette;
            move |theme, status| toggle_icon_button_style(theme, status, active, &palette)
        })
        .on_press(message)
}

static ICON_CACHE: LazyLock<Mutex<HashMap<&'static [u8], iced::widget::svg::Handle>>> =
    LazyLock::new(|| Mutex::new(HashMap::new()));

fn icon_svg<'a>(icon: &'static [u8]) -> iced::widget::Svg<'a, Theme> {
    let handle = ICON_CACHE
        .lock()
        .expect("icon cache lock")
        .entry(icon)
        .or_insert_with(|| iced::widget::svg::Handle::from_memory(icon))
        .clone();

    svg(handle)
        .width(20)
        .height(20)
        .style(|_theme, _status| iced::widget::svg::Style {
            color: Some(COLOR_TEXT),
        })
}

fn menu_item_button<'a>(label: &'a str, message: Message) -> iced::widget::Button<'a, Message> {
    button(text(label).size(14))
        .width(Length::Fill)
        .padding([10, 12])
        .style(menu_item_button_style)
        .on_press(message)
}

fn menu_item_button_with_state(
    label: String,
    message: Option<Message>,
) -> Element<'static, Message> {
    let button = button(text(label).size(14))
        .width(Length::Fill)
        .padding([10, 12])
        .style(menu_item_button_style);

    match message {
        Some(message) => button.on_press(message).into(),
        None => button.into(),
    }
}

fn menu_item_button_variant<'a>(
    label: &'a str,
    message: Message,
    alternate: bool,
) -> iced::widget::Button<'a, Message> {
    button(text(label).size(14))
        .width(Length::Fill)
        .padding([10, 12])
        .style(move |theme, status| menu_item_button_variant_style(theme, status, alternate))
        .on_press(message)
}

fn playlist_context_menu<'a>(playlist_id: i64) -> iced::widget::Column<'a, Message> {
    column![
        menu_item_button(
            "Create playlist",
            Message::Intent(AppIntent::OpenCreatePlaylist),
        ),
        menu_item_button(
            "Rename playlist",
            Message::Intent(AppIntent::OpenRenamePlaylist { id: playlist_id }),
        ),
        menu_item_button(
            "Delete playlist",
            Message::Intent(AppIntent::DeletePlaylist(playlist_id)),
        ),
    ]
    .spacing(2)
}

fn view_context_menu_overlay<'a>(context_menu: &'a ContextMenu) -> Element<'a, Message> {
    let ((menu_width, menu_height), position, menu_content): (
        (f32, f32),
        Point,
        Element<'a, Message>,
    ) = match context_menu {
        ContextMenu::Song {
            path,
            position,
            current_playlist_id,
            queue_index,
            playlists,
        } => {
            let (menu_width, menu_height) = song_context_menu_size(
                playlists.len(),
                current_playlist_id.is_some(),
                queue_index.is_some(),
            );

            (
                (menu_width, menu_height),
                *position,
                song_context_menu_for_path(
                    path.clone(),
                    *current_playlist_id,
                    *queue_index,
                    playlists,
                    menu_height,
                ),
            )
        }
        ContextMenu::Playlist { id, position } => {
            ((210.0, 150.0), *position, playlist_context_menu(*id).into())
        }
    };

    let (x, y) = clamp_menu_position(position, menu_width, menu_height);
    let menu = container(menu_content)
        .padding(8)
        .width(menu_width)
        .style(menu_panel_style);

    mouse_area(
        container(
            column![
                Space::new().height(y),
                row![
                    Space::new().width(x),
                    menu,
                    Space::new().width(Length::Fill),
                ],
                Space::new().height(Length::Fill),
            ]
            .width(Length::Fill)
            .height(Length::Fill),
        )
        .width(Length::Fill)
        .height(Length::Fill),
    )
    .on_press(Message::CloseContextMenu)
    .into()
}

fn song_context_menu_for_path<'a>(
    path: PathBuf,
    current_playlist_id: Option<i64>,
    queue_index: Option<usize>,
    playlists: &'a [SongPlaylistMenuItem],
    menu_height: f32,
) -> Element<'a, Message> {
    let mut menu = column![
        menu_item_button(
            "Play now",
            Message::Intent(AppIntent::PlaySong(path.clone()))
        ),
        menu_item_button(
            "Queue track",
            Message::Intent(AppIntent::QueueSong(path.clone())),
        ),
    ]
    .spacing(2);

    if let Some(playlist_id) = current_playlist_id {
        menu = menu.push(menu_separator());
        menu = menu.push(menu_item_button(
            "Remove from this playlist",
            Message::Intent(AppIntent::RemoveSongFromPlaylist {
                playlist_id,
                path: path.clone(),
            }),
        ));
    }

    if let Some(idx) = queue_index {
        menu = menu.push(menu_separator());
        menu = menu.push(menu_item_button(
            "Remove from queue",
            Message::Intent(AppIntent::RemoveFromQueue(idx)),
        ));
        menu = menu.push(menu_item_button(
            "Clear queue",
            Message::Intent(AppIntent::ClearQueue),
        ));
    }

    menu = menu.push(menu_separator());
    menu = menu.push(menu_item_button(
        "Queue all matching",
        Message::Intent(AppIntent::QueueAllFiltered),
    ));

    menu = menu.push(menu_separator());
    menu = menu.push(
        text("Add to playlist")
            .size(13)
            .color(COLOR_DIM)
            .width(Length::Fill),
    );

    if playlists.is_empty() {
        menu = menu.push(
            container(text("No playlists yet").size(14).color(COLOR_DIM))
                .padding([10, 12])
                .width(Length::Fill),
        );
    } else {
        for playlist in playlists {
            let label = if playlist.contains_song {
                format!("✓ {} (already added)", playlist.name)
            } else {
                playlist.name.clone()
            };
            let message = (!playlist.contains_song).then_some(Message::Intent(
                AppIntent::AddSongToPlaylist {
                    playlist_id: playlist.id,
                    path: path.clone(),
                },
            ));
            menu = menu.push(menu_item_button_with_state(label, message));
        }
    }

    menu = menu.push(menu_item_button_variant(
        "Create new",
        Message::Intent(AppIntent::OpenCreatePlaylistAndAddSong { path: path.clone() }),
        true,
    ));

    menu = menu.push(menu_separator());
    menu = menu.push(
        text("Add all matching to playlist")
            .size(13)
            .color(COLOR_DIM)
            .width(Length::Fill),
    );

    if playlists.is_empty() {
        menu = menu.push(
            container(text("No playlists yet").size(14).color(COLOR_DIM))
                .padding([10, 12])
                .width(Length::Fill),
        );
    } else {
        for playlist in playlists {
            menu = menu.push(menu_item_button(
                &playlist.name,
                Message::Intent(AppIntent::AddAllFilteredToPlaylist(playlist.id)),
            ));
        }
    }

    menu = menu.push(menu_item_button_variant(
        "Create new",
        Message::Intent(AppIntent::OpenCreatePlaylistAndAddAllFiltered),
        true,
    ));

    container(scrollable(menu).height(menu_height)).into()
}

#[derive(Clone, Debug)]
struct SongPlaylistMenuItem {
    id: i64,
    name: String,
    contains_song: bool,
}

const COLOR_BG: Color = Color::from_rgb(0.10, 0.10, 0.11);
const COLOR_PANEL_ALT: Color = Color::from_rgb(0.08, 0.08, 0.09);
const COLOR_ROW_ACTIVE: Color = Color::from_rgb(0.17, 0.17, 0.18);
const COLOR_BORDER: Color = Color::from_rgb(0.23, 0.23, 0.24);
const COLOR_TEXT: Color = Color::from_rgb(0.92, 0.92, 0.93);
const COLOR_DIM: Color = Color::from_rgb(0.58, 0.58, 0.60);
const COLOR_BACKDROP: Color = Color::from_rgba(0.0, 0.0, 0.0, 0.55);
const COLOR_SURFACE: Color = Color::from_rgb(0.12, 0.12, 0.13);
const COLOR_SURFACE_SOFT: Color = Color::from_rgb(0.15, 0.15, 0.16);
const COLOR_BORDER_SUBTLE: Color = Color::from_rgb(0.18, 0.18, 0.20);
const WINDOW_WIDTH: f32 = 1240.0;
const WINDOW_HEIGHT: f32 = 820.0;
const CONTEXT_MENU_WIDTH: f32 = 220.0;
const RADIUS_PANEL: f32 = 14.0;
const RADIUS_CONTROL: f32 = 10.0;
const RADIUS_ROW: f32 = 12.0;
const RADIUS_INPUT: f32 = 12.0;

fn app_shell_style(_theme: &Theme) -> iced::widget::container::Style {
    panel_style(COLOR_BG, 0.0)
}

fn header_panel_style(_theme: &Theme) -> iced::widget::container::Style {
    panel_style(COLOR_SURFACE, RADIUS_PANEL)
}

fn nav_panel_style(_theme: &Theme) -> iced::widget::container::Style {
    panel_style(COLOR_PANEL_ALT, RADIUS_PANEL)
}

fn content_panel_style(_theme: &Theme) -> iced::widget::container::Style {
    panel_style(COLOR_SURFACE, RADIUS_PANEL)
}

fn song_list_panel_style(_theme: &Theme) -> iced::widget::container::Style {
    panel_style(COLOR_PANEL_ALT, RADIUS_PANEL - 2.0)
}

fn menu_panel_style(_theme: &Theme) -> iced::widget::container::Style {
    panel_style(COLOR_SURFACE_SOFT, RADIUS_ROW)
}

fn modal_panel_style(_theme: &Theme) -> iced::widget::container::Style {
    panel_style(COLOR_SURFACE, RADIUS_PANEL)
}

fn modal_backdrop_style(_theme: &Theme) -> iced::widget::container::Style {
    iced::widget::container::Style {
        background: Some(Background::Color(COLOR_BACKDROP)),
        ..Default::default()
    }
}

fn song_row_style(
    _theme: &Theme,
    is_current: bool,
    palette: &UiPalette,
) -> iced::widget::container::Style {
    iced::widget::container::Style {
        background: Some(Background::Color(if is_current {
            Color::from_rgb(0.18, 0.19, 0.22)
        } else {
            Color::from_rgb(0.10, 0.10, 0.11)
        })),
        text_color: Some(COLOR_TEXT),
        border: Border {
            radius: RADIUS_ROW.into(),
            width: 1.0,
            color: if is_current {
                palette.accent_border
            } else {
                Color::from_rgba(1.0, 1.0, 1.0, 0.04)
            },
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

fn nav_button_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
    selected: bool,
    palette: &UiPalette,
) -> iced::widget::button::Style {
    let base_bg = if selected {
        COLOR_ROW_ACTIVE
    } else {
        Color::TRANSPARENT
    };

    let hover_bg = if selected {
        COLOR_ROW_ACTIVE
    } else {
        palette.accent_soft
    };

    let background = match status {
        iced::widget::button::Status::Hovered => hover_bg,
        iced::widget::button::Status::Pressed => COLOR_BORDER,
        iced::widget::button::Status::Disabled | iced::widget::button::Status::Active => base_bg,
    };

    iced::widget::button::Style {
        background: Some(Background::Color(background)),
        text_color: COLOR_TEXT,
        border: Border {
            radius: RADIUS_ROW.into(),
            width: 0.0,
            color: Color::TRANSPARENT,
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

fn control_button_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
) -> iced::widget::button::Style {
    let background = match status {
        iced::widget::button::Status::Hovered => Color::from_rgb(0.20, 0.20, 0.22),
        iced::widget::button::Status::Pressed => Color::from_rgb(0.24, 0.24, 0.26),
        iced::widget::button::Status::Disabled | iced::widget::button::Status::Active => {
            Color::from_rgb(0.16, 0.16, 0.18)
        }
    };

    iced::widget::button::Style {
        background: Some(Background::Color(background)),
        text_color: COLOR_TEXT,
        border: Border {
            radius: RADIUS_CONTROL.into(),
            width: 1.0,
            color: COLOR_BORDER_SUBTLE,
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

fn toggle_icon_button_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
    active: bool,
    palette: &UiPalette,
) -> iced::widget::button::Style {
    let background = match (active, status) {
        (true, iced::widget::button::Status::Hovered) => palette.accent_toggle_bg_hover,
        (true, iced::widget::button::Status::Pressed) => palette.accent_toggle_bg_pressed,
        (true, iced::widget::button::Status::Disabled)
        | (true, iced::widget::button::Status::Active) => palette.accent_toggle_bg,
        (false, iced::widget::button::Status::Hovered) => Color::from_rgb(0.20, 0.20, 0.22),
        (false, iced::widget::button::Status::Pressed) => Color::from_rgb(0.24, 0.24, 0.26),
        (false, iced::widget::button::Status::Disabled)
        | (false, iced::widget::button::Status::Active) => Color::from_rgb(0.16, 0.16, 0.18),
    };

    let border_color = if active { palette.accent } else { COLOR_BORDER };

    iced::widget::button::Style {
        background: Some(Background::Color(background)),
        text_color: COLOR_TEXT,
        border: Border {
            radius: RADIUS_CONTROL.into(),
            width: 1.0,
            color: border_color,
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

fn toggler_style(
    _theme: &Theme,
    status: iced::widget::toggler::Status,
    palette: &UiPalette,
) -> iced::widget::toggler::Style {
    let is_toggled = match status {
        iced::widget::toggler::Status::Active { is_toggled } => is_toggled,
        iced::widget::toggler::Status::Hovered { is_toggled } => is_toggled,
        iced::widget::toggler::Status::Disabled { is_toggled } => is_toggled,
    };

    let (background, border_color) = if is_toggled {
        (Background::Color(palette.accent), palette.accent_border)
    } else {
        (
            Background::Color(Color::from_rgb(0.40, 0.40, 0.42)),
            Color::from_rgb(0.50, 0.50, 0.52),
        )
    };

    iced::widget::toggler::Style {
        background,
        background_border_width: 1.0,
        background_border_color: border_color,
        foreground: Background::Color(Color::from_rgb(0.92, 0.92, 0.93)),
        foreground_border_width: 1.0,
        foreground_border_color: Color::from_rgb(0.60, 0.60, 0.62),
        text_color: Some(COLOR_TEXT),
        border_radius: None,
        padding_ratio: 0.3,
    }
}

fn menu_style(_theme: &Theme, palette: &UiPalette) -> iced::widget::overlay::menu::Style {
    iced::widget::overlay::menu::Style {
        background: iced::Background::Color(Color::from_rgb(0.12, 0.12, 0.13)),
        border: iced::Border {
            radius: RADIUS_INPUT.into(),
            width: 1.0,
            color: COLOR_BORDER_SUBTLE,
        },
        text_color: COLOR_TEXT,
        selected_text_color: COLOR_TEXT,
        selected_background: iced::Background::Color(palette.accent_soft),
        shadow: iced::Shadow::default(),
    }
}

fn pick_list_style(
    _theme: &Theme,
    status: iced::widget::pick_list::Status,
    palette: &UiPalette,
) -> iced::widget::pick_list::Style {
    let border_color = match status {
        iced::widget::pick_list::Status::Active => COLOR_BORDER,
        iced::widget::pick_list::Status::Hovered => Color::from_rgb(0.35, 0.35, 0.37),
        iced::widget::pick_list::Status::Opened { .. } => palette.accent,
    };

    iced::widget::pick_list::Style {
        placeholder_color: COLOR_DIM,
        text_color: COLOR_TEXT,
        background: iced::Background::Color(Color::from_rgb(0.10, 0.10, 0.11)),
        border: iced::Border {
            radius: RADIUS_INPUT.into(),
            width: 1.0,
            color: border_color,
        },
        handle_color: palette.accent,
    }
}

fn ghost_button_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
) -> iced::widget::button::Style {
    let background = match status {
        iced::widget::button::Status::Hovered => Color::from_rgb(0.18, 0.18, 0.19),
        iced::widget::button::Status::Pressed => Color::from_rgb(0.22, 0.22, 0.23),
        iced::widget::button::Status::Disabled | iced::widget::button::Status::Active => {
            Color::TRANSPARENT
        }
    };

    iced::widget::button::Style {
        background: Some(Background::Color(background)),
        text_color: COLOR_TEXT,
        border: Border {
            radius: RADIUS_CONTROL.into(),
            width: 1.0,
            color: COLOR_BORDER_SUBTLE,
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

fn plain_button_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
) -> iced::widget::button::Style {
    let text_color = match status {
        iced::widget::button::Status::Disabled => COLOR_DIM,
        _ => COLOR_TEXT,
    };

    iced::widget::button::Style {
        background: Some(Background::Color(Color::TRANSPARENT)),
        text_color,
        border: Border::default(),
        shadow: Shadow::default(),
        ..Default::default()
    }
}

fn list_button_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
    palette: &UiPalette,
) -> iced::widget::button::Style {
    let background = match status {
        iced::widget::button::Status::Hovered => palette.accent_soft,
        iced::widget::button::Status::Pressed => COLOR_ROW_ACTIVE,
        iced::widget::button::Status::Disabled | iced::widget::button::Status::Active => {
            Color::TRANSPARENT
        }
    };

    iced::widget::button::Style {
        background: Some(Background::Color(background)),
        text_color: COLOR_TEXT,
        border: Border::default(),
        shadow: Shadow::default(),
        ..Default::default()
    }
}

fn menu_item_button_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
) -> iced::widget::button::Style {
    let background = match status {
        iced::widget::button::Status::Hovered => COLOR_ROW_ACTIVE,
        iced::widget::button::Status::Pressed => Color::from_rgb(0.20, 0.20, 0.21),
        iced::widget::button::Status::Disabled | iced::widget::button::Status::Active => {
            Color::TRANSPARENT
        }
    };

    iced::widget::button::Style {
        background: Some(Background::Color(background)),
        text_color: COLOR_TEXT,
        border: Border {
            radius: RADIUS_ROW.into(),
            width: 0.0,
            color: Color::TRANSPARENT,
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

fn menu_item_button_variant_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
    alternate: bool,
) -> iced::widget::button::Style {
    if !alternate {
        return menu_item_button_style(_theme, status);
    }

    let background = match status {
        iced::widget::button::Status::Hovered => COLOR_ROW_ACTIVE,
        iced::widget::button::Status::Pressed => Color::from_rgb(0.20, 0.20, 0.21),
        iced::widget::button::Status::Disabled | iced::widget::button::Status::Active => {
            Color::TRANSPARENT
        }
    };

    iced::widget::button::Style {
        background: Some(Background::Color(background)),
        text_color: Color::from_rgb(0.68, 0.68, 0.70),
        border: Border {
            radius: RADIUS_ROW.into(),
            width: 0.0,
            color: Color::TRANSPARENT,
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

fn search_input_style(
    _theme: &Theme,
    status: iced::widget::text_input::Status,
    palette: &UiPalette,
) -> iced::widget::text_input::Style {
    let border_color = match status {
        iced::widget::text_input::Status::Focused { .. } => palette.accent,
        iced::widget::text_input::Status::Hovered => Color::from_rgb(0.35, 0.35, 0.37),
        iced::widget::text_input::Status::Active | iced::widget::text_input::Status::Disabled => {
            COLOR_BORDER
        }
    };

    iced::widget::text_input::Style {
        background: Background::Color(Color::from_rgb(0.10, 0.10, 0.11)),
        border: Border {
            radius: RADIUS_INPUT.into(),
            width: 1.0,
            color: border_color,
        },
        icon: COLOR_DIM,
        placeholder: COLOR_DIM,
        value: COLOR_TEXT,
        selection: palette.focused_selection,
    }
}

fn slider_style(
    _theme: &Theme,
    status: iced::widget::slider::Status,
    palette: &UiPalette,
) -> iced::widget::slider::Style {
    let handle_background = match status {
        iced::widget::slider::Status::Active => palette.accent,
        iced::widget::slider::Status::Hovered => palette.accent_toggle_bg_hover,
        iced::widget::slider::Status::Dragged => palette.accent_toggle_bg_pressed,
    };

    let handle_border = match status {
        iced::widget::slider::Status::Active => palette.accent,
        iced::widget::slider::Status::Hovered => palette.accent,
        iced::widget::slider::Status::Dragged => palette.accent,
    };

    iced::widget::slider::Style {
        rail: iced::widget::slider::Rail {
            backgrounds: (
                Background::Color(palette.accent),
                Background::Color(Color::from_rgb(0.36, 0.36, 0.39)),
            ),
            width: 4.0,
            border: Border {
                radius: 999.0.into(),
                width: 0.0,
                color: Color::TRANSPARENT,
            },
        },
        handle: iced::widget::slider::Handle {
            shape: iced::widget::slider::HandleShape::Circle { radius: 7.0 },
            background: Background::Color(handle_background),
            border_width: 1.0,
            border_color: handle_border,
        },
    }
}

fn panel_style(background: Color, radius: f32) -> iced::widget::container::Style {
    iced::widget::container::Style {
        background: Some(Background::Color(background)),
        text_color: Some(COLOR_TEXT),
        border: Border {
            radius: radius.into(),
            width: 1.0,
            color: COLOR_BORDER_SUBTLE,
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

fn format_time(seconds: f64) -> String {
    let seconds = seconds.max(0.0).round() as u64;
    let minutes = seconds / 60;
    let remainder = seconds % 60;
    format!("{minutes}:{remainder:02}")
}

#[derive(Debug, Clone)]
enum ContextMenu {
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

fn clamp_menu_position(position: Point, menu_width: f32, menu_height: f32) -> (f32, f32) {
    let x = position.x.clamp(12.0, WINDOW_WIDTH - menu_width - 24.0);
    let y = position.y.clamp(12.0, WINDOW_HEIGHT - menu_height - 24.0);
    (x, y)
}

fn song_context_menu_size(
    playlist_count: usize,
    has_playlist_remove_action: bool,
    has_queue_remove_action: bool,
) -> (f32, f32) {
    let base_rows =
        6 + usize::from(has_playlist_remove_action) + usize::from(has_queue_remove_action);
    let estimated_height = 28.0
        + (base_rows as f32 * 38.0)
        + (playlist_count as f32 * 38.0)
        + 38.0
        + (playlist_count as f32 * 38.0)
        + 38.0;
    (CONTEXT_MENU_WIDTH, estimated_height.min(400.0))
}

fn menu_separator<'a>() -> iced::widget::Rule<'a, Theme> {
    iced::widget::rule::horizontal(1).style(|_theme| iced::widget::rule::Style {
        color: COLOR_BORDER,
        radius: 0.0.into(),
        fill_mode: iced::widget::rule::FillMode::Full,
        snap: true,
    })
}
