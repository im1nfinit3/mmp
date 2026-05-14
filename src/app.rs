//! Application state, data model, and Relm4 top-level component.
//!
//! The parent coordinator: owns LibraryHandle, Playback, QueueState,
//! and three sub-components (PlaybackBar, NavPane, ContentPane).

use std::path::PathBuf;
use std::sync::mpsc;

use gtk4::prelude::*;
use relm4::prelude::*;

use crate::library::scan;
use crate::library::{LibraryEvent, LibraryHandle};
use crate::playback::{Playback, PlaybackEvent, QueueState};
use crate::ui::content_pane::{
    ContentPane, ContentPaneMsg, ContentPaneOutput,
};
use crate::ui::nav_pane::{NavPane, NavPaneMsg, NavPaneOutput};
use crate::ui::playback_bar::{
    PlaybackBar, PlaybackBarMsg, PlaybackBarOutput,
};
use crate::ui::Page;

// ---------------------------------------------------------------------------
// Messages
// ---------------------------------------------------------------------------

#[derive(Debug, Clone)]
pub enum AppMsg {
    // -- Routed from PlaybackBar --
    PlaybackBarOutput(PlaybackBarOutput),
    // -- Routed from NavPane --
    NavPaneOutput(NavPaneOutput),
    // -- Routed from ContentPane --
    ContentPaneOutput(ContentPaneOutput),
    // -- App-level --
    Tick,
}

// ---------------------------------------------------------------------------
// App state
// ---------------------------------------------------------------------------

pub struct AppModel {
    pub library_handle: LibraryHandle,
    pub library_rx: Option<mpsc::Receiver<LibraryEvent>>,
    pub queue: QueueState,
    pub playback: Option<Playback>,
    pub playback_rx: Option<mpsc::Receiver<PlaybackEvent>>,
    pub volume: f64,
    pub muted: bool,
    /// PlaybackBar sub-component controller.
    playback_bar: relm4::Controller<PlaybackBar>,
    /// NavPane sub-component controller.
    nav_pane: relm4::Controller<NavPane>,
    /// ContentPane sub-component controller.
    content_pane: relm4::Controller<ContentPane>,
    /// Current track path (for forwarding to ContentPane).
    current_track_path: Option<PathBuf>,
}

// ---------------------------------------------------------------------------
// Relm4 component
// ---------------------------------------------------------------------------

#[relm4::component(pub)]
impl SimpleComponent for AppModel {
    type Init = ();
    type Input = AppMsg;
    type Output = ();

    view! {
        #[root]
        window = gtk4::Window {
            set_default_size: (900, 600),
            set_title: Some("My Music Player (mmp)"),

            gtk4::Box {
                set_orientation: gtk4::Orientation::Vertical,
                set_css_classes: &["app-root"],

                // Placeholder for PlaybackBar
                #[name(playback_bar_slot)]
                gtk4::Box {},

                gtk4::Separator {
                    set_orientation: gtk4::Orientation::Horizontal,
                },

                #[name(main_shell)]
                gtk4::Box {
                    set_orientation: gtk4::Orientation::Horizontal,
                    set_hexpand: true,
                    set_vexpand: true,
                    set_css_classes: &["main-shell"],

                    // Placeholder for NavPane
                    #[name(nav_pane_slot)]
                    gtk4::Box {},

                    // Placeholder for ContentPane
                    #[name(content_pane_slot)]
                    gtk4::Box {
                        set_hexpand: true,
                        set_vexpand: true,
                    },
                },
            },
        }
    }

    fn init(
        _init: Self::Init,
        root: Self::Root,
        sender: ComponentSender<Self>,
    ) -> ComponentParts<Self> {
        // Load CSS
        let css_provider = gtk4::CssProvider::new();
        css_provider.load_from_data(include_str!("ui/style.css"));
        gtk4::style_context_add_provider_for_display(
            &gtk4::gdk::Display::default().expect("no display"),
            &css_provider,
            gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION,
        );

        let widgets = view_output!();

        // -- Spawn Library actor --
        let (lib_event_tx, lib_event_rx) = mpsc::channel();
        let scan_event_tx = lib_event_tx.clone();
        let library_handle = crate::library::spawn(lib_event_tx);

        // -- Create PlaybackBar child --
        let pb = PlaybackBar::builder()
            .launch(())
            .forward(
                sender.input_sender(),
                |msg: PlaybackBarOutput| AppMsg::PlaybackBarOutput(msg),
            );
        widgets
            .playback_bar_slot
            .append(pb.widget());

        // -- Create NavPane child --
        let nav = NavPane::builder()
            .launch(())
            .forward(
                sender.input_sender(),
                |msg: NavPaneOutput| AppMsg::NavPaneOutput(msg),
            );
        widgets.nav_pane_slot.append(nav.widget());

        // -- Create ContentPane child --
        let content = ContentPane::builder()
            .launch(library_handle.clone())
            .forward(
                sender.input_sender(),
                |msg: ContentPaneOutput| AppMsg::ContentPaneOutput(msg),
            );
        widgets.content_pane_slot.append(content.widget());

        // -- Setup playback engine --
        let (playback_tx, playback_rx) = mpsc::channel();
        let mut playback = Playback::new(playback_tx);

        let model = AppModel {
            library_handle,
            library_rx: Some(lib_event_rx),
            queue: QueueState::new(),
            playback: None,
            playback_rx: None,
            volume: 0.7,
            muted: false,
            playback_bar: pb,
            nav_pane: nav,
            content_pane: content,
            current_track_path: None,
        };

        playback.set_volume(model.volume);
        let model = AppModel {
            playback: Some(playback),
            playback_rx: Some(playback_rx),
            ..model
        };

        // -- Periodic tick --
        let sender_clone = sender.clone();
        glib::timeout_add_local(std::time::Duration::from_millis(200), move || {
            sender_clone.input(AppMsg::Tick);
            glib::ControlFlow::Continue
        });

        // -- Directory scan (deferred 300ms so window renders first) --
        let lib_h = model.library_handle.clone();
        let scan_tx = scan_event_tx;
        let scan_state = std::cell::RefCell::new(Some((lib_h, scan_tx)));
        glib::timeout_add_local(std::time::Duration::from_millis(300), move || {
            if let Some((handle, tx)) = scan_state.borrow_mut().take() {
                scan::start_scan(handle, tx);
            }
            glib::ControlFlow::Break
        });

        ComponentParts { model, widgets }
    }

    fn update(&mut self, msg: Self::Input, _sender: ComponentSender<Self>) {
        match msg {
            AppMsg::Tick => {
                // -- Drain playback events (collect first to avoid borrow conflicts) --
                let playback_events: Vec<PlaybackEvent> = self
                    .playback_rx
                    .as_ref()
                    .map(|rx| {
                        let mut evts = Vec::new();
                        while let Ok(event) = rx.try_recv() {
                            evts.push(event);
                        }
                        evts
                    })
                    .unwrap_or_default();

                for event in playback_events {
                    match &event {
                        PlaybackEvent::EndOfStream => self.advance_track(),
                        PlaybackEvent::Error(err) => {
                            eprintln!("Playback error: {}", err);
                            self.advance_track();
                        }
                        PlaybackEvent::Position { .. }
                        | PlaybackEvent::Tags { .. }
                        | PlaybackEvent::StateChanged(_) => {
                            let _ = self.playback_bar.sender().send(
                                PlaybackBarMsg::PlaybackEvent(event.clone())
                            );
                        }
                    }
                }

                // -- Drain library events --
                if let Some(ref rx) = self.library_rx {
                    while let Ok(event) = rx.try_recv() {
                        match event {
                            LibraryEvent::SongsLoaded { .. }
                            | LibraryEvent::SongsAdded { .. } => {
                                let _ = self.content_pane.sender().send(
                                    ContentPaneMsg::SongsAdded,
                                );
                            }
                            LibraryEvent::PlaylistsChanged => {
                                let playlists =
                                    self.library_handle.get_playlists();
                                let _ = self.nav_pane.sender().send(
                                    NavPaneMsg::SetPlaylists(playlists.clone()),
                                );
                                let _ = self.content_pane.sender().send(
                                    ContentPaneMsg::SetPlaylists(playlists),
                                );
                            }
                            LibraryEvent::ScanStarted
                            | LibraryEvent::ScanComplete { .. }
                            | LibraryEvent::Error(_) => {}
                        }
                    }
                }
            }

            // -- PlaybackBar output --
            AppMsg::PlaybackBarOutput(msg) => match msg {
                PlaybackBarOutput::PlayPause => {
                    if let Some(ref mut pb) = self.playback {
                        if self.queue.current.is_none()
                            && !self.queue.tracks.is_empty()
                        {
                            let idx = self.queue.tracks.len() - 1;
                            self.queue.current = Some(idx);
                            pb.play_file(&self.queue.tracks[idx]);
                            self.update_current_track();
                        } else {
                            pb.toggle_pause();
                        }
                    }
                }
                PlaybackBarOutput::Previous => {
                    if let Some(current) = self.queue.current
                        && current > 0 {
                            let prev = current - 1;
                            self.queue.current = Some(prev);
                            self.play_track_at(prev);
                        }
                }
                PlaybackBarOutput::Next => {
                    self.advance_track();
                }
                PlaybackBarOutput::Seek(seconds) => {
                    if let Some(ref mut pb) = self.playback {
                        pb.seek(seconds);
                    }
                }
                PlaybackBarOutput::VolumeChanged(vol) => {
                    self.volume = vol;
                    if let Some(ref mut pb) = self.playback {
                        pb.set_volume(vol);
                    }
                    if self.muted {
                        self.muted = false;
                        if let Some(ref mut pb) = self.playback {
                            pb.set_mute(false);
                        }
                    }
                }
                PlaybackBarOutput::MuteToggled => {
                    self.muted = !self.muted;
                    if let Some(ref mut pb) = self.playback {
                        pb.set_mute(self.muted);
                    }
                }
                PlaybackBarOutput::ShuffleToggled => {
                    self.queue.toggle_shuffle();
                }
                PlaybackBarOutput::RepeatToggled => {
                    self.queue.cycle_repeat();
                }
            },

            // -- NavPane output --
            AppMsg::NavPaneOutput(msg) => match msg {
                NavPaneOutput::PageSelected(page) => {
                    let _ = self
                        .content_pane
                        .sender()
                        .send(ContentPaneMsg::SetPage(page));
                }
                NavPaneOutput::CreatePlaylist(_name) => {
                    self.show_create_playlist_dialog();
                }
                NavPaneOutput::DeletePlaylist(id) => {
                    self.library_handle.delete_playlist(id);
                }
                NavPaneOutput::RenamePlaylist(id, _name) => {
                    self.show_rename_playlist_dialog(id);
                }
            },

            // -- ContentPane output --
            AppMsg::ContentPaneOutput(msg) => match msg {
                ContentPaneOutput::PlayFromLibrary(path) => {
                    let idx = self.queue.push(path.clone());
                    if self.queue.current.is_none() {
                        self.queue.current = Some(idx);
                        self.play_track_at(idx);
                    }
                }
                ContentPaneOutput::QueueFromLibrary(path) => {
                    self.queue.push(path);
                }
                ContentPaneOutput::AddToPlaylist(playlist_id, path) => {
                    self.library_handle
                        .add_to_playlist(playlist_id, path);
                }
                ContentPaneOutput::AddToNewPlaylist(_name, path) => {
                    self.show_new_playlist_dialog(path);
                }
                ContentPaneOutput::SaveQueueAsPlaylist(_name) => {
                    self.show_save_queue_dialog();
                }
                ContentPaneOutput::NavSongsWithSearch(_search) => {
                    // Navigate to songs page and set search
                    let _ = self
                        .content_pane
                        .sender()
                        .send(ContentPaneMsg::SetPage(Page::Songs));
                    // TODO: wire search text to ContentPane
                }
            },
        }
    }
}

impl AppModel {
    fn play_track_at(&mut self, idx: usize) {
        if let Some(ref mut pb) = self.playback
            && let Some(path) = self.queue.tracks.get(idx) {
                pb.play_file(path);
                self.update_current_track();
            }
    }

    fn advance_track(&mut self) {
        if let Some(next) = self.queue.next_track() {
            self.queue.current = Some(next);
            self.play_track_at(next);
        } else {
            if let Some(ref mut pb) = self.playback {
                pb.stop();
            }
            self.queue.current = None;
            self.current_track_path = None;
            let _ = self.content_pane.sender().send(
                ContentPaneMsg::CurrentTrackPath(None),
            );
        }
    }

    fn update_current_track(&mut self) {
        let path = self
            .queue
            .current
            .and_then(|i| self.queue.tracks.get(i).cloned());
        self.current_track_path = path.clone();
        let _ = self
            .content_pane
            .sender()
            .send(ContentPaneMsg::CurrentTrackPath(path));
    }

    /// Show a dialog asking for a playlist name, then create it.
    fn show_create_playlist_dialog(&self) {
        let dialog = gtk4::Dialog::builder()
            .title("Create Playlist")
            .modal(true)
            .build();

        let entry = gtk4::Entry::new();
        entry.set_placeholder_text(Some("Playlist name"));
        dialog.content_area().append(&entry);

        dialog.add_button("Cancel", gtk4::ResponseType::Cancel);
        dialog.add_button("Create", gtk4::ResponseType::Accept);

        let lib = self.library_handle.clone();
        dialog.connect_response(move |d, resp| {
            if resp == gtk4::ResponseType::Accept {
                let name = entry.text().to_string();
                if !name.is_empty() {
                    let _ = lib.create_playlist(&name);
                }
            }
            d.close();
        });

        dialog.present();
    }

    /// Show a dialog asking for a new name for an existing playlist.
    fn show_rename_playlist_dialog(&self, playlist_id: i64) {
        let dialog = gtk4::Dialog::builder()
            .title("Rename Playlist")
            .modal(true)
            .build();

        let entry = gtk4::Entry::new();
        entry.set_placeholder_text(Some("New name"));
        dialog.content_area().append(&entry);

        dialog.add_button("Cancel", gtk4::ResponseType::Cancel);
        dialog.add_button("Rename", gtk4::ResponseType::Accept);

        let lib = self.library_handle.clone();
        dialog.connect_response(move |d, resp| {
            if resp == gtk4::ResponseType::Accept {
                let name = entry.text().to_string();
                if !name.is_empty() {
                    lib.rename_playlist(playlist_id, &name);
                }
            }
            d.close();
        });

        dialog.present();
    }

    /// Show a dialog asking for a playlist name, then create it and add
    /// the given song.
    fn show_new_playlist_dialog(&self, song_path: PathBuf) {
        let dialog = gtk4::Dialog::builder()
            .title("New Playlist")
            .modal(true)
            .build();

        let entry = gtk4::Entry::new();
        entry.set_placeholder_text(Some("Playlist name"));
        dialog.content_area().append(&entry);

        dialog.add_button("Cancel", gtk4::ResponseType::Cancel);
        dialog.add_button("Create", gtk4::ResponseType::Accept);

        let lib = self.library_handle.clone();
        dialog.connect_response(move |d, resp| {
            if resp == gtk4::ResponseType::Accept {
                let name = entry.text().to_string();
                if !name.is_empty()
                    && let Ok(id) = lib.create_playlist(&name) {
                        lib.add_to_playlist(id, song_path.clone());
                    }
            }
            d.close();
        });

        dialog.present();
    }

    /// Show a dialog asking for a playlist name, then save the current
    /// queue as a new playlist.
    fn show_save_queue_dialog(&self) {
        let dialog = gtk4::Dialog::builder()
            .title("Save Queue as Playlist")
            .modal(true)
            .build();

        let entry = gtk4::Entry::new();
        entry.set_placeholder_text(Some("Playlist name"));
        dialog.content_area().append(&entry);

        dialog.add_button("Cancel", gtk4::ResponseType::Cancel);
        dialog.add_button("Save", gtk4::ResponseType::Accept);

        let lib = self.library_handle.clone();
        let tracks = self.queue.tracks.clone();
        dialog.connect_response(move |d, resp| {
            if resp == gtk4::ResponseType::Accept {
                let name = entry.text().to_string();
                if !name.is_empty()
                    && let Ok(id) = lib.create_playlist(&name) {
                        for path in &tracks {
                            lib.add_to_playlist(id, path.clone());
                        }
                    }
            }
            d.close();
        });

        dialog.present();
    }
}
