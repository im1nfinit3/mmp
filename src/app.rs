//! Application state, data model, and Relm4 top-level component.

use std::cell::RefCell;
use std::collections::HashMap;
use std::path::PathBuf;
use std::sync::mpsc;

use gtk4::prelude::*;
use relm4::prelude::*;
use relm4::RelmRemoveAllExt;

use crate::db;
use crate::playback::{self, Playback, PlaybackEvent, PlaybackState, QueueState};

// ---------------------------------------------------------------------------
// Data model
// ---------------------------------------------------------------------------

/// A single song/track in the library.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Song {
    pub path: PathBuf,
    pub title: String,
    pub artist: String,
    pub album: String,
    pub duration_str: String,
}

impl Song {
    pub fn new(path: PathBuf) -> Self {
        let filename = path
            .file_stem()
            .and_then(|s| s.to_str())
            .unwrap_or("Unknown")
            .to_string();
        Self {
            path,
            title: filename,
            artist: String::from("Unknown Artist"),
            album: String::from("Unknown Album"),
            duration_str: String::new(),
        }
    }

    pub fn label(&self) -> String {
        if self.artist.is_empty() || self.artist == "Unknown Artist" {
            self.title.clone()
        } else {
            format!("{} — {}", self.title, self.artist)
        }
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum RepeatMode {
    Off,
    All,
    One,
}

impl RepeatMode {
    pub fn next(self) -> Self {
        match self {
            Self::Off => Self::All,
            Self::All => Self::One,
            Self::One => Self::Off,
        }
    }

    pub fn label(self) -> &'static str {
        match self {
            Self::Off => "Repeat: Off",
            Self::All => "Repeat: All",
            Self::One => "Repeat: One",
        }
    }
}

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
    ScanError(String),
    Tick,
}

// ---------------------------------------------------------------------------
// App state
// ---------------------------------------------------------------------------

pub struct AppModel {
    pub library: Vec<Song>,
    pub library_by_path: HashMap<PathBuf, usize>,
    pub queue: QueueState,
    pub playback: Option<Playback>,
    pub playback_rx: Option<mpsc::Receiver<PlaybackEvent>>,
    pub volume: f64,
    pub muted: bool,
    pub library_db: Option<rusqlite::Connection>,
    pub playlists_db: Option<rusqlite::Connection>,
    pub playlists: Vec<db::Playlist>,
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
    pub fn current_song(&self) -> Option<&Song> {
        let idx = self.queue.current?;
        let path = self.queue.tracks.get(idx)?;
        self.library_by_path.get(path).map(|&lib_idx| &self.library[lib_idx])
    }

    pub fn filtered_indices(&self) -> Vec<usize> {
        self.library.iter().enumerate().filter(|(_, song)| {
            if !self.search_lowered.is_empty() {
                let tl = song.title.to_lowercase();
                let al = song.artist.to_lowercase();
                let bl = song.album.to_lowercase();
                if !tl.contains(&self.search_lowered) && !al.contains(&self.search_lowered) && !bl.contains(&self.search_lowered) {
                    return false;
                }
            }
            if let Some(ref a) = self.selected_artist {
                if song.artist != *a { return false; }
            }
            if let Some(ref a) = self.selected_album {
                if song.album != *a { return false; }
            }
            true
        }).map(|(i, _)| i).collect()
    }

    pub fn unique_artists(&self) -> Vec<String> {
        let mut set: std::collections::BTreeSet<String> = self.library.iter()
            .map(|s| s.artist.clone()).filter(|a| !a.is_empty()).collect();
        set.into_iter().collect()
    }

    pub fn unique_albums(&self) -> Vec<String> {
        let mut set: std::collections::BTreeSet<String> = self.library.iter()
            .map(|s| s.album.clone()).filter(|a| !a.is_empty()).collect();
        set.into_iter().collect()
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
            build_library_panel("Search songs");
        let (albums_page, albums_search_entry, albums_list_box) =
            build_library_panel("Search albums");
        let (artists_page, artists_search_entry, artists_list_box) =
            build_library_panel("Search artists");

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

        // -- Build model --
        let mut model = AppModel {
            library: Vec::new(),
            library_by_path: HashMap::new(),
            queue: QueueState::new(),
            playback: None,
            playback_rx: None,
            volume: 0.7,
            muted: false,
            library_db: None,
            playlists_db: None,
            playlists: Vec::new(),
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
        build_nav_row(&widgets.nav_recently_added_row, "Recently added", "songs-view");
        build_nav_row(&widgets.nav_albums_row, "Albums", "albums");
        build_nav_row(&widgets.nav_artists_row, "Artists", "artists");
        build_nav_row(&widgets.nav_songs_row, "Songs", "songs-view");
        build_nav_row(&widgets.nav_queue_row, "Queue", "queue");
        {
            let label = gtk4::Label::builder()
                .css_classes(["row-label"]).label("PLAYLISTS")
                .halign(gtk4::Align::Start).build();
            widgets.nav_playlists_header.set_child(Some(&label));
        }
        build_nav_row(&widgets.nav_settings_row, "Settings", "settings");

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

        // -- Open databases, load cached songs --
        if let Ok(conn) = db::open_library_db() {
            if let Ok(songs) = db::get_all_songs(&conn) {
                for song in songs {
                    model.library_by_path.insert(song.path.clone(), model.library.len());
                    model.library.push(song);
                }
            }
            model.library_db = Some(conn);
        }
        if let Ok(conn) = db::open_playlists_db() {
            if let Ok(pls) = db::get_playlists(&conn) {
                model.playlists = pls;
            }
            model.playlists_db = Some(conn);
        }

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

        // -- Directory scan --
        let sender_clone = sender.clone();
        std::thread::spawn(move || {
            scan_directory(sender_clone);
        });

        ComponentParts { model, widgets }
    }

    fn update(&mut self, msg: Self::Input, sender: ComponentSender<Self>) {
        match msg {
            AppMsg::Tick => {
                // Drain playback events into a Vec first to avoid borrow conflicts
                let events: Vec<PlaybackEvent> = if let Some(ref rx) = self.playback_rx {
                    let mut evts = Vec::new();
                    while let Ok(event) = rx.try_recv() {
                        evts.push(event);
                    }
                    evts
                } else {
                    Vec::new()
                };

                // Now process events (no borrow on self.playback_rx)
                for event in events {
                    match event {
                        PlaybackEvent::Tags { title, artist } => {
                            if let Some(ref conn) = self.library_db {
                                if let Some(idx) = self.queue.current {
                                    if let Some(path) = self.queue.tracks.get(idx).cloned() {
                                        if let Some(&lib_idx) = self.library_by_path.get(&path) {
                                            let song = &mut self.library[lib_idx];
                                            if let Some(t) = title { song.title = t; }
                                            if let Some(a) = artist { song.artist = a; }
                                            let _ = db::save_song(conn, song);
                                        }
                                    }
                                }
                            }
                        }
                        PlaybackEvent::EndOfStream => { self.advance_track(); }
                        PlaybackEvent::Error(err) => {
                            eprintln!("Playback error: {}", err);
                            self.advance_track();
                        }
                        PlaybackEvent::Position { .. } | PlaybackEvent::StateChanged(_) => {}
                    }
                }
                // Update progress display
                if let Some(ref pb) = self.playback {
                    if let Some((elapsed, duration)) = pb.query_position() {
                        self.track_progress_scale.set_range(0.0, duration);
                        self.track_progress_scale.set_value(elapsed);
                        self.elapsed_time_label.set_label(&format_time(elapsed));
                        self.duration_label.set_label(&format_time(duration));
                    }
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

            AppMsg::PlaybackEvent(event) => {
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

            AppMsg::NavSongs => { self.current_page = Page::Songs; }
            AppMsg::NavAlbums => { self.current_page = Page::Albums; }
            AppMsg::NavArtists => { self.current_page = Page::Artists; }
            AppMsg::NavQueue => { self.current_page = Page::Queue; }
            AppMsg::NavRecentlyAdded => {
                self.current_page = Page::RecentlyAdded;
                self.current_playlist_id = 0;
            }
            AppMsg::NavSettings => { self.current_page = Page::Settings; }
            AppMsg::NavPlaylistRow(id) => {
                self.current_playlist_id = id;
                self.current_page = Page::Songs;
                // Load playlist songs into song view
                if let Some(ref conn) = self.playlists_db {
                    if let Ok(songs) = db::get_playlist_songs(conn, id) {
                        // For now, just switch to songs view
                        // TODO: set base list to playlist songs
                    }
                }
            }
            AppMsg::SearchChanged(text) => {
                self.search_text = text.clone();
                self.search_lowered = text.to_lowercase();
            }
            AppMsg::SearchAlbumsChanged(text) => {
                self.search_albums_text = text.clone();
                self.search_albums_lowered = text.to_lowercase();
            }
            AppMsg::SearchArtistsChanged(text) => {
                self.search_artists_text = text.clone();
                self.search_artists_lowered = text.to_lowercase();
            }

            AppMsg::CreatePlaylist(_) => {}
            AppMsg::DeletePlaylist(_) => {}
            AppMsg::RenamePlaylist(_, _) => {}
            AppMsg::AddToPlaylist(_, _) => {}
            AppMsg::ScanAddSong(path) => {
                // Check if already in library
                if !self.library_by_path.contains_key(&path) {
                    let song = Song::new(path);
                    let idx = self.library.len();
                    self.library_by_path.insert(song.path.clone(), idx);
                    self.library.push(song.clone());
                    // Save to DB
                    if let Some(ref conn) = self.library_db {
                        let _ = db::save_song(conn, &song);
                    }
                }
            }
            AppMsg::ScanStarted => {}
            AppMsg::ScanComplete(count) => {
                eprintln!("Library scan complete: {} songs", count);
            }
            AppMsg::ScanError(err) => {
                eprintln!("Library scan error: {}", err);
            }
        }

        // Sync UI after every message
        self.sync_ui();
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

    fn sync_ui(&self) {
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

    fn rebuild_song_list(&self) {
        self.song_list_box.remove_all();
        let mut paths = self.displayed_song_paths.borrow_mut();
        paths.clear();
        let indices = self.filtered_indices();
        for &idx in &indices {
            let song = &self.library[idx];
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
            paths.push(song.path.clone());
            self.song_list_box.append(&row);
        }
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
        for pl in &self.playlists {
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
        for (i, path) in self.queue.tracks.iter().enumerate() {
            let label = if let Some(&lib_idx) = self.library_by_path.get(path) {
                self.library[lib_idx].label()
            } else {
                path.file_stem().and_then(|s| s.to_str()).unwrap_or("Unknown").to_string()
            };
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

fn format_time(seconds: f64) -> String {
    let total = seconds as u64;
    let mins = total / 60;
    let secs = total % 60;
    format!("{}:{:02}", mins, secs)
}

fn build_library_panel(placeholder: &str) -> (gtk4::Box, gtk4::SearchEntry, gtk4::ListBox) {
    let page_box = gtk4::Box::new(gtk4::Orientation::Vertical, 0);
    page_box.add_css_class("content-page");

    let panel_box = gtk4::Box::new(gtk4::Orientation::Vertical, 0);
    panel_box.set_vexpand(true);
    panel_box.add_css_class("library-panel");
    page_box.append(&panel_box);

    let search = gtk4::SearchEntry::new();
    search.set_placeholder_text(Some(placeholder));
    panel_box.append(&search);

    let list_box = gtk4::ListBox::new();
    list_box.add_css_class("library-list");
    list_box.add_css_class("boxed-list");

    let scrolled = gtk4::ScrolledWindow::new();
    scrolled.set_vexpand(true);
    scrolled.set_child(Some(&list_box));
    panel_box.append(&scrolled);

    (page_box, search, list_box)
}

fn build_nav_row(row: &gtk4::ListBoxRow, label_text: &str, _page_name: &str) {
    let label = gtk4::Label::builder()
        .css_classes(["row-label"]).label(label_text)
        .halign(gtk4::Align::Start).build();
    row.set_child(Some(&label));
}

fn scan_directory(sender: ComponentSender<AppModel>) {
    let music_dir = dirs::audio_dir().unwrap_or_else(|| {
        dirs::home_dir().unwrap_or_else(|| PathBuf::from(".")).join("Music")
    });

    let supported = ["mp3", "flac", "ogg", "wav", "m4a"];
    let mut count = 0usize;

    for entry in walkdir::WalkDir::new(&music_dir).follow_links(true)
        .into_iter().filter_map(|e| e.ok())
    {
        let path = entry.path();
        if !path.is_file() { continue; }
        let ext = path.extension().and_then(|e| e.to_str())
            .map(|e| e.to_lowercase()).unwrap_or_default();
        if !supported.contains(&ext.as_str()) { continue; }

        count += 1;
        let _ = sender.input(AppMsg::ScanAddSong(path.to_path_buf()));
    }

    let _ = sender.input(AppMsg::ScanComplete(count));
}
