//! Application state, data model, and Relm4 top-level component.

use std::cell::RefCell;
use std::path::PathBuf;
use std::sync::mpsc;

use gtk4::prelude::*;
use relm4::prelude::*;
use relm4::RelmRemoveAllExt;

use crate::library::scan;
use crate::library::song::{RepeatMode, Song};
use crate::library::{LibraryEvent, LibraryHandle};
use crate::playback::{Playback, PlaybackEvent, QueueState};
use crate::ui::widgets;

// ---------------------------------------------------------------------------
// Messages
// ---------------------------------------------------------------------------

#[derive(Debug, Clone)]
pub enum AppMsg {
    PlayPause,
    Previous,
    Next,
    Seek(f64),
    VolumeChanged(f64),
    MuteToggled,
    ShuffleToggled,
    RepeatToggled,
    PlaybackEvent(PlaybackEvent),
    PlayFromLibrary(PathBuf),
    QueueFromLibrary(PathBuf),
    ClearQueue,
    NavSongs,
    NavAlbums,
    NavArtists,
    NavQueue,
    NavRecentlyAdded,
    NavSettings,
    NavPlaylistRow(i64),
    SearchChanged(String),
    SearchAlbumsChanged(String),
    SearchArtistsChanged(String),
    CreatePlaylist(String),
    DeletePlaylist(i64),
    RenamePlaylist(i64, String),
    AddToPlaylist(i64, PathBuf),
    ScanStarted,
    ScanComplete(usize),
    ScanAddSong(PathBuf),
    BatchScan(Vec<PathBuf>),
    ScanError(String),
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
    pub current_page: Page,
    pub current_playlist_id: i64,
    pub search_text: String,
    pub search_lowered: String,
    pub search_albums_text: String,
    pub search_albums_lowered: String,
    pub search_artists_text: String,
    pub search_artists_lowered: String,
    pub selected_artist: Option<String>,
    pub selected_album: Option<String>,
    pub tick: u64,
    pub recently_added_reverse: bool,
    pub displayed_song_paths: RefCell<Vec<PathBuf>>,
    /// True when the visible list is stale and needs a rebuild.
    pub dirty_lists: bool,
    /// Songs discovered by the scanner, waiting to be added to the library.
    pub scan_pending: std::collections::VecDeque<PathBuf>,

    // Cached widget refs (cloned from view_output! or set in init)
    pub current_track_label: gtk4::Label,
    pub play_pause_button: gtk4::Button,
    pub shuffle_button: gtk4::Button,
    pub repeat_button: gtk4::Button,
    pub mute_button: gtk4::Button,
    pub volume_scale: gtk4::Scale,
    pub track_progress_scale: gtk4::Scale,
    pub elapsed_time_label: gtk4::Label,
    pub duration_label: gtk4::Label,
    pub content_stack: gtk4::Stack,
    pub song_list_box: gtk4::ListBox,
    pub albums_list_box: gtk4::ListBox,
    pub artists_list_box: gtk4::ListBox,
    pub queue_list_box: gtk4::ListBox,
    pub songs_search_entry: gtk4::SearchEntry,
    pub albums_search_entry: gtk4::SearchEntry,
    pub artists_search_entry: gtk4::SearchEntry,
    pub navigation_list: gtk4::ListBox,
    pub nav_recently_added_row: gtk4::ListBoxRow,
    pub nav_albums_row: gtk4::ListBoxRow,
    pub nav_artists_row: gtk4::ListBoxRow,
    pub nav_songs_row: gtk4::ListBoxRow,
    pub nav_queue_row: gtk4::ListBoxRow,
    pub nav_playlists_header: gtk4::ListBoxRow,
    pub nav_settings_row: gtk4::ListBoxRow,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Page {
    RecentlyAdded,
    Songs,
    Albums,
    Artists,
    Queue,
    Settings,
}

impl AppModel {
    pub fn current_song(&self) -> Option<Song> {
        let idx = self.queue.current?;
        let path = self.queue.tracks.get(idx)?;
        let songs = self.library_handle.get_all_songs();
        songs.into_iter().find(|s| s.path == *path)
    }

    pub fn filtered_songs(&self) -> Vec<Song> {
        let search = self.search_lowered.clone();
        let artist = self.selected_artist.clone();
        let album = self.selected_album.clone();
        let filter: Box<dyn Fn(&Song) -> bool + Send + 'static> = Box::new(move |song| {
            if !search.is_empty() {
                let tl = song.title.to_lowercase();
                let al = song.artist.to_lowercase();
                let bl = song.album.to_lowercase();
                if !tl.contains(&search) && !al.contains(&search) && !bl.contains(&search) {
                    return false;
                }
            }
            if let Some(ref a) = artist {
                if song.artist != *a { return false; }
            }
            if let Some(ref a) = album {
                if song.album != *a { return false; }
            }
            true
        });
        self.library_handle.get_songs(filter)
    }

    pub fn unique_artists(&self) -> Vec<String> {
        self.library_handle.get_unique_artists()
    }

    pub fn unique_albums(&self) -> Vec<String> {
        self.library_handle.get_unique_albums()
    }
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

                // ====================================================
                // Playback Bar — [controls left] [info center] [vol right]
                // ====================================================
                #[name(playback_bar)]
                gtk4::Box {
                    set_css_classes: &["playback-bar"],
                    set_spacing: 12,

                    // -- Left: transport controls --
                    #[name(controls_box)]
                    gtk4::Box {
                        set_orientation: gtk4::Orientation::Horizontal,
                        set_spacing: 4,
                        set_valign: gtk4::Align::Center,

                        #[name(prev_button)]
                        gtk4::Button {
                            set_css_classes: &["playback-button"],
                            set_icon_name: "media-skip-backward-symbolic",
                            set_tooltip_text: Some("Previous track"),
                            set_valign: gtk4::Align::Center,
                            connect_clicked => AppMsg::Previous,
                        },
                        #[name(play_pause_button)]
                        gtk4::Button {
                            set_css_classes: &["playback-button"],
                            set_icon_name: "media-playback-start-symbolic",
                            set_tooltip_text: Some("Play"),
                            set_valign: gtk4::Align::Center,
                            connect_clicked => AppMsg::PlayPause,
                        },
                        #[name(next_button)]
                        gtk4::Button {
                            set_css_classes: &["playback-button"],
                            set_icon_name: "media-skip-forward-symbolic",
                            set_tooltip_text: Some("Next track"),
                            set_valign: gtk4::Align::Center,
                            connect_clicked => AppMsg::Next,
                        },
                        #[name(repeat_button)]
                        gtk4::Button {
                            set_css_classes: &["playback-button"],
                            set_icon_name: "media-playlist-repeat-symbolic",
                            set_tooltip_text: Some("Repeat"),
                            set_valign: gtk4::Align::Center,
                            connect_clicked => AppMsg::RepeatToggled,
                        },
                        #[name(shuffle_button)]
                        gtk4::Button {
                            set_css_classes: &["playback-button"],
                            set_icon_name: "media-playlist-shuffle-symbolic",
                            set_tooltip_text: Some("Shuffle"),
                            set_valign: gtk4::Align::Center,
                            connect_clicked => AppMsg::ShuffleToggled,
                        },
                    },

                    // -- Center: track info + progress --
                    #[name(info_box)]
                    gtk4::Box {
                        set_orientation: gtk4::Orientation::Vertical,
                        set_hexpand: true,
                        set_spacing: 4,
                        set_css_classes: &["track-info"],

                        #[name(current_track_label)]
                        gtk4::Label {
                            set_label: "No track selected",
                            set_halign: gtk4::Align::Start,
                            set_ellipsize: gtk4::pango::EllipsizeMode::End,
                            set_css_classes: &["track-info"],
                        },

                        #[name(progress_box)]
                        gtk4::Box {
                            set_orientation: gtk4::Orientation::Horizontal,
                            set_spacing: 8,

                            #[name(elapsed_time_label)]
                            gtk4::Label {
                                set_css_classes: &["time-label"],
                                set_label: "0:00",
                            },
                            #[name(track_progress_scale)]
                            gtk4::Scale {
                                set_hexpand: true,
                                set_draw_value: false,
                                set_range: (0.0, 1.0),
                                set_increments: (1.0, 10.0),
                            },
                            #[name(duration_label)]
                            gtk4::Label {
                                set_css_classes: &["time-label"],
                                set_label: "0:00",
                            },
                        },
                    },

                    // -- Right: volume --
                    #[name(volume_controls)]
                    gtk4::Box {
                        set_orientation: gtk4::Orientation::Horizontal,
                        set_spacing: 8,
                        set_valign: gtk4::Align::Center,

                        #[name(volume_scale)]
                        gtk4::Scale {
                            set_css_classes: &["volume-scale"],
                            set_range: (0.0, 100.0),
                            set_draw_value: false,
                            set_value: 70.0,
                        },
                        #[name(mute_button)]
                        gtk4::Button {
                            set_css_classes: &["volume-button"],
                            set_icon_name: "audio-volume-medium-symbolic",
                            set_tooltip_text: Some("Mute"),
                            connect_clicked => AppMsg::MuteToggled,
                        },
                    },
                },

                gtk4::Separator {
                    set_orientation: gtk4::Orientation::Horizontal,
                },

                // ====================================================
                // Main shell: Nav sidebar + Content stack
                // ====================================================
                #[name(main_shell)]
                gtk4::Box {
                    set_orientation: gtk4::Orientation::Horizontal,
                    set_hexpand: true,
                    set_vexpand: true,
                    set_css_classes: &["main-shell"],

                    // -- Navigation Pane --
                    #[name(nav_pane)]
                    gtk4::Box {
                        set_orientation: gtk4::Orientation::Vertical,
                        set_css_classes: &["nav-pane"],

                        #[name(navigation_list)]
                        gtk4::ListBox {
                            set_css_classes: &["navigation-list"],
                            set_vexpand: true,

                            #[name(nav_recently_added_row)]
                            gtk4::ListBoxRow {},
                            #[name(nav_albums_row)]
                            gtk4::ListBoxRow {},
                            #[name(nav_artists_row)]
                            gtk4::ListBoxRow {},
                            #[name(nav_songs_row)]
                            gtk4::ListBoxRow {},
                            #[name(nav_queue_row)]
                            gtk4::ListBoxRow {},

                            #[name(nav_playlists_header)]
                            gtk4::ListBoxRow {
                                set_css_classes: &["nav-header"],
                                set_selectable: false,
                                set_activatable: false,
                            },

                            #[name(nav_settings_row)]
                            gtk4::ListBoxRow {},
                        },
                    },

                    // -- Content Stack --
                    #[name(content_stack)]
                    gtk4::Stack {
                        set_hexpand: true,
                        set_vexpand: true,
                        set_css_classes: &["content-stack"],
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

        // -- Create content page widgets locally --
        let (songs_page, songs_search_entry, songs_list_box) =
            widgets::build_library_panel("Search songs");
        let (albums_page, albums_search_entry, albums_list_box) =
            widgets::build_library_panel("Search albums");
        let (artists_page, artists_search_entry, artists_list_box) =
            widgets::build_library_panel("Search artists");

        let queue_list_box = gtk4::ListBox::new();
        queue_list_box.add_css_class("library-list");
        queue_list_box.add_css_class("boxed-list");
        let queue_page = {
            let page_box = gtk4::Box::new(gtk4::Orientation::Vertical, 12);
            page_box.add_css_class("content-page");
            let scrolled = gtk4::ScrolledWindow::new();
            scrolled.set_vexpand(true);
            scrolled.set_child(Some(&queue_list_box));
            page_box.append(&scrolled);
            page_box
        };

        let settings_page = {
            let page_box = gtk4::Box::new(gtk4::Orientation::Vertical, 12);
            page_box.add_css_class("content-page");
            let cb = gtk4::CheckButton::with_label("Scan music folder on startup");
            page_box.append(&cb);
            page_box
        };

        widgets.content_stack.add_titled(&songs_page, Some("songs-view"), "Songs");
        widgets.content_stack.add_titled(&albums_page, Some("albums"), "Albums");
        widgets.content_stack.add_titled(&artists_page, Some("artists"), "Artists");
        widgets.content_stack.add_titled(&queue_page, Some("queue"), "Queue");
        widgets.content_stack.add_titled(&settings_page, Some("settings"), "Settings");

        // -- Wire signals (before model consumes local variables) --
        {
            let s = sender.clone();
            songs_search_entry.connect_search_changed(move |entry| {
                s.input(AppMsg::SearchChanged(entry.text().to_string()));
            });
        }
        {
            let s = sender.clone();
            albums_search_entry.connect_search_changed(move |entry| {
                s.input(AppMsg::SearchAlbumsChanged(entry.text().to_string()));
            });
        }
        {
            let s = sender.clone();
            artists_search_entry.connect_search_changed(move |entry| {
                s.input(AppMsg::SearchArtistsChanged(entry.text().to_string()));
            });
        }
        // Song list double-click
        {
            let s = sender.clone();
            songs_list_box.connect_row_activated(move |_, _row| {
                s.input(AppMsg::Tick); // placeholder
            });
        }
        // Albums list click → filter
        {
            let s = sender.clone();
            let list = albums_list_box.clone();
            list.connect_row_activated(move |_, row| {
                if let Some(child) = row.child() {
                    if let Some(label) = child.downcast_ref::<gtk4::Label>() {
                        s.input(AppMsg::SearchChanged(label.label().to_string()));
                        s.input(AppMsg::NavSongs);
                    }
                }
            });
        }
        // Artists list click → filter
        {
            let s = sender.clone();
            let list = artists_list_box.clone();
            list.connect_row_activated(move |_, row| {
                if let Some(child) = row.child() {
                    if let Some(label) = child.downcast_ref::<gtk4::Label>() {
                        s.input(AppMsg::SearchChanged(label.label().to_string()));
                        s.input(AppMsg::NavSongs);
                    }
                }
            });
        }
        // Volume scale
        {
            let s = sender.clone();
            widgets.volume_scale.connect_change_value(move |_, _, value| {
                s.input(AppMsg::VolumeChanged(value / 100.0));
                gtk4::glib::Propagation::Proceed
            });
        }
        // Track progress scale
        {
            let s = sender.clone();
            widgets.track_progress_scale.connect_change_value(move |scale, _, value| {
                let seconds = value * scale.adjustment().upper();
                s.input(AppMsg::Seek(seconds));
                gtk4::glib::Propagation::Proceed
            });
        }

        // -- Spawn Library actor --
        let (lib_event_tx, lib_event_rx) = mpsc::channel();
        let library_handle = crate::library::spawn(lib_event_tx);

        // -- Build model --
        let mut model = AppModel {
            library_handle,
            library_rx: Some(lib_event_rx),
            queue: QueueState::new(),
            playback: None,
            playback_rx: None,
            volume: 0.7,
            muted: false,
            current_page: Page::RecentlyAdded,
            current_playlist_id: 0,
            search_text: String::new(),
            search_lowered: String::new(),
            search_albums_text: String::new(),
            search_albums_lowered: String::new(),
            search_artists_text: String::new(),
            search_artists_lowered: String::new(),
            selected_artist: None,
            selected_album: None,
            tick: 0,
            recently_added_reverse: true,
            displayed_song_paths: RefCell::new(Vec::new()),
            dirty_lists: true,
            scan_pending: std::collections::VecDeque::new(),
            current_track_label: widgets.current_track_label.clone(),
            play_pause_button: widgets.play_pause_button.clone(),
            shuffle_button: widgets.shuffle_button.clone(),
            repeat_button: widgets.repeat_button.clone(),
            mute_button: widgets.mute_button.clone(),
            volume_scale: widgets.volume_scale.clone(),
            track_progress_scale: widgets.track_progress_scale.clone(),
            elapsed_time_label: widgets.elapsed_time_label.clone(),
            duration_label: widgets.duration_label.clone(),
            content_stack: widgets.content_stack.clone(),
            song_list_box: songs_list_box,
            albums_list_box: albums_list_box,
            artists_list_box: artists_list_box,
            queue_list_box: queue_list_box,
            songs_search_entry: songs_search_entry,
            albums_search_entry: albums_search_entry,
            artists_search_entry: artists_search_entry,
            navigation_list: widgets.navigation_list.clone(),
            nav_recently_added_row: widgets.nav_recently_added_row.clone(),
            nav_albums_row: widgets.nav_albums_row.clone(),
            nav_artists_row: widgets.nav_artists_row.clone(),
            nav_songs_row: widgets.nav_songs_row.clone(),
            nav_queue_row: widgets.nav_queue_row.clone(),
            nav_playlists_header: widgets.nav_playlists_header.clone(),
            nav_settings_row: widgets.nav_settings_row.clone(),
        };

        // -- Set up navigation rows --
        widgets::build_nav_row(&widgets.nav_recently_added_row, "Recently added", "songs-view");
        widgets::build_nav_row(&widgets.nav_albums_row, "Albums", "albums");
        widgets::build_nav_row(&widgets.nav_artists_row, "Artists", "artists");
        widgets::build_nav_row(&widgets.nav_songs_row, "Songs", "songs-view");
        widgets::build_nav_row(&widgets.nav_queue_row, "Queue", "queue");
        {
            let label = gtk4::Label::builder()
                .css_classes(["row-label"]).label("PLAYLISTS")
                .halign(gtk4::Align::Start).build();
            widgets.nav_playlists_header.set_child(Some(&label));
        }
        widgets::build_nav_row(&widgets.nav_settings_row, "Settings", "settings");

        // Nav row activation signals
        for (row, msg) in [
            (&widgets.nav_recently_added_row, AppMsg::NavRecentlyAdded),
            (&widgets.nav_albums_row, AppMsg::NavAlbums),
            (&widgets.nav_artists_row, AppMsg::NavArtists),
            (&widgets.nav_songs_row, AppMsg::NavSongs),
            (&widgets.nav_queue_row, AppMsg::NavQueue),
            (&widgets.nav_settings_row, AppMsg::NavSettings),
        ] {
            let s = sender.clone();
            let m = msg;
            row.connect_activate(move |_| { s.input(m.clone()); });
        }

        // Select "Recently added" by default
        widgets.navigation_list.select_row(Some(&widgets.nav_recently_added_row));

        // -- Volume revealer: slider hidden until hover, slides out left --
        {
            let revealer = gtk4::Revealer::new();
            revealer.set_transition_type(gtk4::RevealerTransitionType::SlideLeft);
            revealer.set_reveal_child(false);

            // Reparent: remove volume_scale from volume_controls, wrap in revealer
            widgets.volume_controls.remove(&widgets.volume_scale);
            revealer.set_child(Some(&widgets.volume_scale));
            widgets.volume_controls.prepend(&revealer);

            // Motion controller on the volume_controls box
            let motion = gtk4::EventControllerMotion::new();
            let r1 = revealer.clone();
            motion.connect_enter(move |_, _x, _y| {
                r1.set_reveal_child(true);
            });
            let r2 = revealer.clone();
            motion.connect_leave(move |_| {
                r2.set_reveal_child(false);
            });
            widgets.volume_controls.add_controller(motion);
        }

        // Load current_track_label bold via Pango attributes
        {
            let attrs = gtk4::pango::AttrList::new();
            attrs.insert(gtk4::pango::AttrInt::new_weight(gtk4::pango::Weight::Bold));
            widgets.current_track_label.set_attributes(Some(&attrs));
        }

        // Library actor loads cached songs on its own thread — no DB setup needed here.

        // -- Setup playback engine --
        let (tx, rx) = mpsc::channel();
        let mut playback = Playback::new(tx);
        playback.set_volume(model.volume);
        model.playback = Some(playback);
        model.playback_rx = Some(rx);

        // -- Populate initial lists --
        model.rebuild_song_list();
        model.rebuild_albums_list();
        model.rebuild_artists_list();
        model.rebuild_playlists_nav();

        // -- Periodic tick --
        let sender_clone = sender.clone();
        glib::timeout_add_local(std::time::Duration::from_millis(200), move || {
            sender_clone.input(AppMsg::Tick);
            glib::ControlFlow::Continue
        });

        // -- Directory scan (deferred 300ms so window renders first) --
        let sender_clone = sender.clone();
        glib::timeout_add_local(std::time::Duration::from_millis(300), move || {
            let s = sender_clone.clone();
            std::thread::spawn(move || {
                scan::scan_directory(s);
            });
            glib::ControlFlow::Break // one-shot
        });

        ComponentParts { model, widgets }
    }

    fn update(&mut self, msg: Self::Input, _sender: ComponentSender<Self>) {
        match msg {
            AppMsg::Tick => {
                // Drain playback events
                let events: Vec<PlaybackEvent> = if let Some(ref rx) = self.playback_rx {
                    let mut evts = Vec::new();
                    while let Ok(event) = rx.try_recv() {
                        evts.push(event);
                    }
                    evts
                } else {
                    Vec::new()
                };

                for event in events {
                    match event {
                        PlaybackEvent::Tags { .. } => {
                            // TODO: Phase C — metadata is extracted at scan time.
                            // Runtime tag updates will be handled via LibraryCommand::UpdateSongMetadata.
                        }
                        PlaybackEvent::EndOfStream => { self.advance_track(); }
                        PlaybackEvent::Error(err) => {
                            eprintln!("Playback error: {}", err);
                            self.advance_track();
                        }
                        PlaybackEvent::Position { .. } | PlaybackEvent::StateChanged(_) => {}
                    }
                }

                // Drain library events
                if let Some(ref rx) = self.library_rx {
                    while let Ok(event) = rx.try_recv() {
                        match event {
                            LibraryEvent::SongsLoaded { .. }
                            | LibraryEvent::SongsAdded { .. } => {
                                self.dirty_lists = true;
                            }
                            LibraryEvent::PlaylistsChanged => {
                                self.rebuild_playlists_nav();
                            }
                            LibraryEvent::ScanStarted
                            | LibraryEvent::ScanComplete { .. }
                            | LibraryEvent::Error(_) => {}
                        }
                    }
                }

                // Fast: update progress/time labels always
                if let Some(ref pb) = self.playback {
                    if let Some((elapsed, duration)) = pb.query_position() {
                        self.track_progress_scale.set_range(0.0, duration);
                        self.track_progress_scale.set_value(elapsed);
                        self.elapsed_time_label.set_label(&widgets::format_time(elapsed));
                        self.duration_label.set_label(&widgets::format_time(duration));
                    }
                }
                // Fast: button states + track label
                self.sync_progress();

                // Process pending scan batch — cap at 300 per tick to yield main loop
                if !self.scan_pending.is_empty() {
                    let limit = self.scan_pending.len().min(300);
                    let batch: Vec<PathBuf> = self.scan_pending.drain(..limit).collect();
                    self.process_scan_batch(batch);
                    self.dirty_lists = true;
                }

                // Slow: rebuild lists only when content changed
                if self.dirty_lists {
                    self.sync_lists();
                }
            }

            AppMsg::PlayPause => {
                if let Some(ref mut pb) = self.playback {
                    if self.queue.current.is_none() && !self.queue.tracks.is_empty() {
                        let idx = self.queue.tracks.len() - 1;
                        self.queue.current = Some(idx);
                        pb.play_file(&self.queue.tracks[idx]);
                    } else {
                        pb.toggle_pause();
                    }
                }
            }
            AppMsg::Previous => {
                if let Some(current) = self.queue.current {
                    if current > 0 {
                        let prev = current - 1;
                        self.queue.current = Some(prev);
                        self.play_track_at(prev);
                    }
                }
            }
            AppMsg::Next => { self.advance_track(); }
            AppMsg::Seek(seconds) => {
                if let Some(ref mut pb) = self.playback { pb.seek(seconds); }
            }
            AppMsg::VolumeChanged(vol) => {
                self.volume = vol;
                if let Some(ref mut pb) = self.playback { pb.set_volume(vol); }
                if self.muted {
                    self.muted = false;
                    if let Some(ref mut pb) = self.playback { pb.set_mute(false); }
                }
            }
            AppMsg::MuteToggled => {
                self.muted = !self.muted;
                if let Some(ref mut pb) = self.playback { pb.set_mute(self.muted); }
            }
            AppMsg::ShuffleToggled => { self.queue.toggle_shuffle(); }
            AppMsg::RepeatToggled => { self.queue.cycle_repeat(); }

            AppMsg::PlaybackEvent(_event) => {
                // Handled inline above in Tick for now
            }

            AppMsg::PlayFromLibrary(path) => {
                let idx = self.queue.push(path);
                if self.queue.current.is_none() {
                    self.queue.current = Some(idx);
                    self.play_track_at(idx);
                }
            }
            AppMsg::QueueFromLibrary(path) => { self.queue.push(path); }
            AppMsg::ClearQueue => {
                if let Some(ref mut pb) = self.playback { pb.stop(); }
                self.queue.clear();
            }

            AppMsg::NavSongs => { self.current_page = Page::Songs; self.dirty_lists = true; }
            AppMsg::NavAlbums => { self.current_page = Page::Albums; self.dirty_lists = true; }
            AppMsg::NavArtists => { self.current_page = Page::Artists; self.dirty_lists = true; }
            AppMsg::NavQueue => { self.current_page = Page::Queue; self.dirty_lists = true; }
            AppMsg::NavRecentlyAdded => {
                self.current_page = Page::RecentlyAdded;
                self.current_playlist_id = 0;
                self.dirty_lists = true;
            }
            AppMsg::NavSettings => { self.current_page = Page::Settings; }
            AppMsg::NavPlaylistRow(id) => {
                self.current_playlist_id = id;
                self.current_page = Page::Songs;
                self.dirty_lists = true;
            }
            AppMsg::SearchChanged(text) => {
                self.search_text = text.clone();
                self.search_lowered = text.to_lowercase();
                self.dirty_lists = true;
            }
            AppMsg::SearchAlbumsChanged(text) => {
                self.search_albums_text = text.clone();
                self.search_albums_lowered = text.to_lowercase();
                self.dirty_lists = true;
            }
            AppMsg::SearchArtistsChanged(text) => {
                self.search_artists_text = text.clone();
                self.search_artists_lowered = text.to_lowercase();
                self.dirty_lists = true;
            }

            AppMsg::CreatePlaylist(_) => {}
            AppMsg::DeletePlaylist(_) => {}
            AppMsg::RenamePlaylist(_, _) => {}
            AppMsg::AddToPlaylist(_, _) => {}
            AppMsg::ScanAddSong(path) => {
                // Queue for batch processing in Tick — don't rebuild here
                self.scan_pending.push_back(path);
            }
            AppMsg::BatchScan(paths) => {
                // Queue for capped processing in Tick — don't block here
                self.scan_pending.extend(paths);
                self.dirty_lists = true;
            }
            AppMsg::ScanStarted => {}
            AppMsg::ScanComplete(count) => {
                eprintln!("Library scan complete: {} songs", count);
                // Flush any remaining pending songs
                if !self.scan_pending.is_empty() {
                    let batch: Vec<PathBuf> = self.scan_pending.drain(..).collect();
                    self.process_scan_batch(batch);
                }
                self.dirty_lists = true;
            }
            AppMsg::ScanError(err) => {
                eprintln!("Library scan error: {}", err);
            }
        }

            // Sync UI after every message
            // self.sync_ui();  // REMOVED — was causing O(n²) rebuilds on every message
        }
}

impl AppModel {
    fn play_track_at(&mut self, idx: usize) {
        if let Some(ref mut pb) = self.playback {
            if let Some(path) = self.queue.tracks.get(idx) {
                pb.play_file(path);
            }
        }
    }

    fn advance_track(&mut self) {
        if let Some(next) = self.queue.next_track() {
            self.queue.current = Some(next);
            self.play_track_at(next);
        } else {
            if let Some(ref mut pb) = self.playback { pb.stop(); }
            self.queue.current = None;
        }
    }

    fn process_scan_batch(&mut self, paths: Vec<PathBuf>) {
        let songs: Vec<Song> = paths.into_iter().map(Song::new).collect();
        self.library_handle.add_songs(songs);
    }

    fn sync_progress(&self) {
        // Fast path: update labels and button states only (no list rebuild)
        if let Some(song) = self.current_song() {
            self.current_track_label.set_label(&song.label());
        } else {
            self.current_track_label.set_label("No track selected");
        }

        if self.queue.shuffle {
            self.shuffle_button.set_css_classes(&["playback-button", "active-control"]);
        } else {
            self.shuffle_button.set_css_classes(&["playback-button"]);
        }
        if self.queue.repeat != RepeatMode::Off {
            self.repeat_button.set_css_classes(&["playback-button", "active-control"]);
        } else {
            self.repeat_button.set_css_classes(&["playback-button"]);
        }

        self.volume_scale.set_value(self.volume * 100.0);
        let icon = if self.muted { "audio-volume-muted-symbolic" } else { "audio-volume-medium-symbolic" };
        self.mute_button.set_icon_name(icon);
    }

    fn sync_lists(&mut self) {
        // Slow path: rebuild visible list. Only call when content changed.
        let name = match self.current_page {
            Page::RecentlyAdded | Page::Songs => "songs-view",
            Page::Albums => "albums",
            Page::Artists => "artists",
            Page::Queue => "queue",
            Page::Settings => "settings",
        };
        self.content_stack.set_visible_child_name(name);

        match self.current_page {
            Page::Songs | Page::RecentlyAdded => self.rebuild_song_list(),
            Page::Albums => self.rebuild_albums_list(),
            Page::Artists => self.rebuild_artists_list(),
            Page::Queue => self.rebuild_queue_list(),
            Page::Settings => {},
        }
        self.dirty_lists = false;
    }

    fn sync_ui(&mut self) {
        let name = match self.current_page {
            Page::RecentlyAdded | Page::Songs => "songs-view",
            Page::Albums => "albums",
            Page::Artists => "artists",
            Page::Queue => "queue",
            Page::Settings => "settings",
        };
        self.content_stack.set_visible_child_name(name);

        if let Some(song) = self.current_song() {
            self.current_track_label.set_label(&song.label());
        } else {
            self.current_track_label.set_label("No track selected");
        }

        if self.queue.shuffle {
            self.shuffle_button.set_css_classes(&["playback-button", "active-control"]);
        } else {
            self.shuffle_button.set_css_classes(&["playback-button"]);
        }
        if self.queue.repeat != RepeatMode::Off {
            self.repeat_button.set_css_classes(&["playback-button", "active-control"]);
        } else {
            self.repeat_button.set_css_classes(&["playback-button"]);
        }

        self.volume_scale.set_value(self.volume * 100.0);
        let icon = if self.muted { "audio-volume-muted-symbolic" } else { "audio-volume-medium-symbolic" };
        self.mute_button.set_icon_name(icon);

        match self.current_page {
            Page::Songs | Page::RecentlyAdded => self.rebuild_song_list(),
            Page::Albums => self.rebuild_albums_list(),
            Page::Artists => self.rebuild_artists_list(),
            Page::Queue => self.rebuild_queue_list(),
            Page::Settings => {},
        }
    }

    fn rebuild_song_list(&mut self) {
        let songs = self.filtered_songs();
        let current_row_count = {
            let mut n = 0i32;
            loop {
                if self.song_list_box.row_at_index(n).is_none() { break; }
                n += 1;
            }
            n as usize
        };

        let paths = self.displayed_song_paths.borrow();
        let can_append = current_row_count > 0
            && songs.len() > current_row_count
            && current_row_count <= paths.len()
            && songs[..current_row_count]
                .iter()
                .zip(paths.iter().take(current_row_count))
                .all(|(s, p)| s.path == *p);

        drop(paths);

        if can_append {
            let mut paths = self.displayed_song_paths.borrow_mut();
            for song in &songs[current_row_count..] {
                let row = self.build_song_row(song);
                paths.push(song.path.clone());
                self.song_list_box.append(&row);
            }
        } else {
            self.song_list_box.remove_all();
            let mut paths = self.displayed_song_paths.borrow_mut();
            paths.clear();
            for song in &songs {
                let row = self.build_song_row(song);
                paths.push(song.path.clone());
                self.song_list_box.append(&row);
            }
        }
    }

    fn build_song_row(&self, song: &Song) -> gtk4::ListBoxRow {
        let row_box = gtk4::Box::new(gtk4::Orientation::Horizontal, 12);
        row_box.add_css_class("song-row");

        let indicator = gtk4::Image::new();
        row_box.append(&indicator);

        let title_label = gtk4::Label::builder()
            .label(&song.title).halign(gtk4::Align::Start)
            .hexpand(true).ellipsize(gtk4::pango::EllipsizeMode::End).build();
        row_box.append(&title_label);

        let artist_label = gtk4::Label::builder()
            .label(&song.artist).css_classes(["dim-label"])
            .ellipsize(gtk4::pango::EllipsizeMode::Start)
            .max_width_chars(15).build();
        row_box.append(&artist_label);

        let album_label = gtk4::Label::builder()
            .label(&song.album).css_classes(["dim-label"])
            .ellipsize(gtk4::pango::EllipsizeMode::Start)
            .max_width_chars(15).build();
        row_box.append(&album_label);

        let d_label = gtk4::Label::builder()
            .label(&song.duration_str)
            .css_classes(["dim-label", "duration-label"]).build();
        row_box.append(&d_label);

        let row = gtk4::ListBoxRow::new();
        row.set_child(Some(&row_box));
        row
    }

    fn rebuild_albums_list(&self) {
        self.albums_list_box.remove_all();
        let mut seen = std::collections::BTreeSet::new();
        for album in self.unique_albums() {
            let lowered = album.to_lowercase();
            if !self.search_albums_lowered.is_empty()
                && !lowered.contains(&self.search_albums_lowered) {
                continue;
            }
            if seen.contains(&album) { continue; }
            seen.insert(album.clone());
            let row = gtk4::ListBoxRow::new();
            row.set_child(Some(&gtk4::Label::builder()
                .css_classes(["row-label"]).label(&album)
                .halign(gtk4::Align::Start).build()));
            self.albums_list_box.append(&row);
        }
    }

    fn rebuild_artists_list(&self) {
        self.artists_list_box.remove_all();
        let mut seen = std::collections::BTreeSet::new();
        for artist in self.unique_artists() {
            let lowered = artist.to_lowercase();
            if !self.search_artists_lowered.is_empty()
                && !lowered.contains(&self.search_artists_lowered) {
                continue;
            }
            if seen.contains(&artist) { continue; }
            seen.insert(artist.clone());
            let row = gtk4::ListBoxRow::new();
            row.set_child(Some(&gtk4::Label::builder()
                .css_classes(["row-label"]).label(&artist)
                .halign(gtk4::Align::Start).build()));
            self.artists_list_box.append(&row);
        }
    }

    fn rebuild_playlists_nav(&self) {
        let mut to_remove = Vec::new();
        for i in 0.. {
            let row = self.navigation_list.row_at_index(i);
            if row.is_none() { break; }
            let row = row.unwrap();
            if row.has_css_class("nav-sub-row") && row != self.nav_playlists_header {
                to_remove.push(i);
            }
        }
        for i in to_remove.into_iter().rev() {
            if let Some(row) = self.navigation_list.row_at_index(i) {
                self.navigation_list.remove(&row);
            }
        }
        let header_idx = self.nav_playlists_header.index();
        let mut pos = header_idx + 1;
        let playlists = self.library_handle.get_playlists();
        for pl in &playlists {
            let row = gtk4::ListBoxRow::new();
            row.add_css_class("nav-sub-row");
            row.set_child(Some(&gtk4::Label::builder()
                .css_classes(["row-label"]).label(&pl.name)
                .halign(gtk4::Align::Start).build()));
            self.navigation_list.insert(&row, pos);
            pos += 1;
        }
    }

    fn rebuild_queue_list(&self) {
        self.queue_list_box.remove_all();
        let all_songs = self.library_handle.get_all_songs();
        for (i, path) in self.queue.tracks.iter().enumerate() {
            let label = all_songs.iter()
                .find(|s| s.path == *path)
                .map(|s| s.label())
                .unwrap_or_else(|| {
                    path.file_stem()
                        .and_then(|s| s.to_str())
                        .unwrap_or("Unknown")
                        .to_string()
                });
            let row = gtk4::ListBoxRow::new();
            row.set_child(Some(&gtk4::Label::builder()
                .css_classes(["song-row"]).label(&label)
                .halign(gtk4::Align::Start).build()));
            if Some(i) == self.queue.current {
                row.add_css_class("current-track");
            }
            self.queue_list_box.append(&row);
        }
    }
}
