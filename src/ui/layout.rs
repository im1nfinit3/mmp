//! Top-level layout and view functions for the mmp GUI.

use std::path::PathBuf;

use iced::widget::{
    button, column, container, mouse_area, opaque, row, scrollable, slider, stack, text,
    text_input, Space,
};
use iced::{
    Alignment, Element, Length, Point,
    widget::{pick_list, toggler},
};

use crate::app::{ContextMenu, Message, SongPlaylistMenuItem};
use crate::core::{ActiveModal, AppIntent, AppState, Page, PlaybackStatus, SongView};
use crate::library::song::RepeatMode;
use crate::settings::{self, SortMethod};
use crate::style::{
    self, app_shell_style, content_panel_style, ghost_button_style, header_panel_style,
    list_button_style, menu_panel_style, modal_backdrop_style, modal_panel_style,
    nav_panel_style, pick_list_style, plain_button_style, search_input_style, slider_style,
    song_list_panel_style, toggler_style, COLOR_BORDER, COLOR_DIM,
};
use crate::system_accent::UiPalette;
use crate::ui::widgets::{
    icon_button, menu_item_button, menu_item_button_variant, menu_item_button_with_state,
    toggle_icon_button,
};

// ---------------------------------------------------------------------------
// Main view
// ---------------------------------------------------------------------------

pub fn view<'a>(
    body_content: Element<'a, Message>,
    context_menu: &'a Option<ContextMenu>,
    active_modal: &'a Option<ActiveModal>,
    state: &'a AppState,
    palette: &'a UiPalette,
) -> Element<'a, Message> {
    let mut layers: Vec<Element<'a, Message>> = vec![opaque(body_content)];

    if let Some(ctx) = context_menu {
        layers.push(opaque(view_context_menu_overlay(ctx)));
    }

    if let Some(modal) = active_modal {
        layers.push(opaque(view_modal_overlay(state, modal, palette)));
    }

    stack(layers)
        .width(Length::Fill)
        .height(Length::Fill)
        .into()
}

/// Build the three-row shell: header / nav+content / status bar.
pub fn view_shell<'a>(
    state: &'a AppState,
    palette: &'a UiPalette,
    status_bar: &'a crate::app::StatusBar,
) -> Element<'a, Message> {
    mouse_area(
        container(
            column![
                view_header(state, palette),
                row![view_nav(state, palette), view_content(state, palette)]
                    .spacing(18)
                    .height(Length::Fill),
                view_status_bar(status_bar, palette),
            ]
            .spacing(8)
            .height(Length::Fill),
        )
        .padding(iced::Padding::new(22.0).bottom(12.0))
        .width(Length::Fill)
        .height(Length::Fill)
        .style(app_shell_style),
    )
    .on_move(Message::CursorMoved)
    .into()
}

// ---------------------------------------------------------------------------
// Header: transport controls + now-playing + volume
// ---------------------------------------------------------------------------

fn view_header<'a>(state: &'a AppState, palette: &'a UiPalette) -> Element<'a, Message> {
    let play_icon = match state.playback {
        PlaybackStatus::Playing => crate::style::ICON_PAUSE,
        PlaybackStatus::Paused | PlaybackStatus::Stopped => crate::style::ICON_PLAY,
    };

    let controls = row![
        icon_button(crate::style::ICON_PREVIOUS, Message::Intent(AppIntent::Previous)),
        icon_button(play_icon, Message::Intent(AppIntent::PlayPause)),
        icon_button(crate::style::ICON_NEXT, Message::Intent(AppIntent::Next)),
        toggle_icon_button(
            if state.shuffle {
                crate::style::ICON_SHUFFLE_ON
            } else {
                crate::style::ICON_SHUFFLE_OFF
            },
            state.shuffle,
            Message::Intent(AppIntent::ToggleShuffle),
            palette,
        ),
        toggle_icon_button(
            match state.repeat {
                RepeatMode::Off | RepeatMode::All => crate::style::ICON_REPEAT_ALL,
                RepeatMode::One => crate::style::ICON_REPEAT_ONE,
            },
            state.repeat != RepeatMode::Off,
            Message::Intent(AppIntent::ToggleRepeat),
            palette,
        ),
    ]
    .spacing(10)
    .width(336);

    let now_playing_info = {
        let mut col = column![].spacing(4);
        if state.current_song_title.is_empty() && state.current_song_artist.is_empty() {
            col = col.push(text(&state.current_track_label).size(20));
        } else {
            col = col.push(text(&state.current_song_title).size(20));
            if !state.current_song_artist.is_empty() || !state.current_song_album.is_empty() {
                let mut detail_parts: Vec<Element<'_, Message>> = Vec::new();
                if !state.current_song_artist.is_empty() {
                    detail_parts.push(
                        text(&state.current_song_artist).size(14).color(COLOR_DIM).into(),
                    );
                }
                if !state.current_song_album.is_empty() {
                    if !detail_parts.is_empty() {
                        detail_parts.push(text(" • ").size(14).color(COLOR_DIM).into());
                    }
                    detail_parts.push(
                        text(&state.current_song_album).size(14).color(COLOR_DIM).into(),
                    );
                }
                col = col.push(row(detail_parts).spacing(0));
            }
        }
        col
    };

    let progress = column![
        now_playing_info,
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
                crate::style::ICON_VOLUME_MUTE
            } else {
                crate::style::ICON_VOLUME_HIGH
            },
            Message::Intent(AppIntent::ToggleMute)
        )
    ]
    .spacing(12)
    .align_y(Alignment::Center)
    .width(236);

    let header = column![
        row![controls, progress, volume]
            .spacing(22)
            .align_y(Alignment::Center)
    ]
    .spacing(14);

    container(header)
        .padding([20, 22])
        .style(header_panel_style)
        .width(Length::Fill)
        .into()
}

// ---------------------------------------------------------------------------
// Navigation sidebar
// ---------------------------------------------------------------------------

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
                    move |theme, status| {
                        style::nav_button_style(theme, status, is_active, &palette)
                    }
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
                    style::nav_button_style(theme, status, state.page == Page::Settings, &palette)
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
            move |theme, status| style::nav_button_style(theme, status, selected, &palette)
        })
        .on_press(Message::Intent(AppIntent::SelectPage(page)))
        .into()
}

// ---------------------------------------------------------------------------
// Content area (page switching)
// ---------------------------------------------------------------------------

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
            view_song_list(title, "Filter playlist", &state.songs_search, &state.songs, palette)
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

// ---------------------------------------------------------------------------
// Song list
// ---------------------------------------------------------------------------

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
            move |theme| style::song_row_style(theme, song.is_current, &palette)
        }),
    )
    .on_right_press(Message::OpenSongMenu {
        path: song.path.clone(),
        queue_index: song.queue_index,
    })
    .into()
}

// ---------------------------------------------------------------------------
// Generic string list (albums / artists)
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Queue view
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Settings view
// ---------------------------------------------------------------------------

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
            row![
                text_input(
                    "Default: ~/Music (or your system audio dir)",
                    &folder_value,
                )
                .on_input(|value| Message::Intent(AppIntent::SetLibraryFolder(value)))
                .padding([10, 14])
                .width(600)
                .style({
                    let palette = *palette;
                    move |theme, status| search_input_style(theme, status, &palette)
                }),
                button(text("Rescan").size(14))
                    .padding([10, 14])
                    .style(ghost_button_style)
                    .on_press(Message::Intent(AppIntent::ForceRescan)),
            ]
            .spacing(10)
            .align_y(Alignment::Center),
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
                    move |theme| style::menu_style(theme, &palette)
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
                        let sort = SortMethod::ALL
                            .iter()
                            .find(|s| s.label() == selected)
                            .copied()
                            .unwrap_or(SortMethod::TimeAddedNewestFirst);
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
                    move |theme| style::menu_style(theme, &palette)
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

// ---------------------------------------------------------------------------
// Status bar
// ---------------------------------------------------------------------------

fn view_status_bar<'a>(
    status_bar: &'a crate::app::StatusBar,
    palette: &'a UiPalette,
) -> Element<'a, Message> {
    use crate::style::status_bar_panel_style;

    let left_text = status_bar.display_text();
    let track_count = status_bar.track_count;

    let right_label = if track_count == 1 {
        "1 song".to_string()
    } else {
        format!("{track_count} songs")
    };

    let left_color = if status_bar.has_active_notification() {
        palette.accent
    } else {
        COLOR_DIM
    };

    container(
        row![
            text(left_text).size(13).color(left_color),
            Space::new().width(Length::Fill),
            text(right_label).size(13).color(COLOR_DIM),
        ]
        .align_y(Alignment::Center),
    )
    .padding([8, 16])
    .width(Length::Fill)
    .height(34)
    .style(status_bar_panel_style)
    .into()
}

// ---------------------------------------------------------------------------
// Modal overlay
// ---------------------------------------------------------------------------

fn view_modal_overlay<'a>(
    _state: &'a AppState,
    modal: &'a ActiveModal,
    palette: &'a UiPalette,
) -> Element<'a, Message> {
    let title = match modal {
        ActiveModal::RenamePlaylist { .. } => "Rename playlist",
        ActiveModal::CreatePlaylistAndAddSong { .. } => "Create playlist and add song",
        ActiveModal::SaveQueueAsPlaylist { .. } => "Save queue as playlist",
        ActiveModal::CreatePlaylistAndAddAllFiltered { .. } => {
            "Create playlist and add all matching"
        }
    };

    let body: Element<'a, Message> = match modal {
        ActiveModal::RenamePlaylist { name, .. }
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
                        .style(crate::style::control_button_style)
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

// ---------------------------------------------------------------------------
// Context menu
// ---------------------------------------------------------------------------

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
            let (menu_width, menu_height) =
                song_context_menu_size(playlists.len(), current_playlist_id.is_some(), queue_index.is_some());

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
        menu_item_button("Play now", Message::Intent(AppIntent::PlaySong(path.clone()))),
        menu_item_button("Queue track", Message::Intent(AppIntent::QueueSong(path.clone()))),
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

fn playlist_context_menu<'a>(playlist_id: i64) -> iced::widget::Column<'a, Message> {
    column![
        menu_item_button(
            "Queue playlist",
            Message::Intent(AppIntent::QueuePlaylist(playlist_id)),
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

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

fn format_time(seconds: f64) -> String {
    let seconds = seconds.max(0.0).round() as u64;
    let minutes = seconds / 60;
    let remainder = seconds % 60;
    format!("{minutes}:{remainder:02}")
}

fn clamp_menu_position(position: Point, menu_width: f32, menu_height: f32) -> (f32, f32) {
    let x = position.x.clamp(12.0, style::WINDOW_WIDTH - menu_width - 24.0);
    let y = position.y.clamp(12.0, style::WINDOW_HEIGHT - menu_height - 24.0);
    (x, y)
}

fn song_context_menu_size(
    playlist_count: usize,
    has_playlist_remove_action: bool,
    has_queue_remove_action: bool,
) -> (f32, f32) {
    let base_rows = 6
        + usize::from(has_playlist_remove_action)
        + usize::from(has_queue_remove_action);
    let estimated_height = 28.0
        + (base_rows as f32 * 38.0)
        + (playlist_count as f32 * 38.0)
        + 38.0
        + (playlist_count as f32 * 38.0)
        + 38.0;
    (style::CONTEXT_MENU_WIDTH, estimated_height.min(400.0))
}

fn menu_separator<'a>() -> iced::widget::Rule<'a, Theme> {
    iced::widget::rule::horizontal(1).style(|_theme| iced::widget::rule::Style {
        color: COLOR_BORDER,
        radius: 0.0.into(),
        fill_mode: iced::widget::rule::FillMode::Full,
        snap: true,
    })
}

use iced::Theme;
