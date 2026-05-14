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

#[derive(Debug)]
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
    NavPlaylist(i64),
    SearchChanged(String),
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
    pub selected_artist: Option<String>,
    pub selected_album: Option<String>,
    pub tick: u64,

    // Currently displayed song paths (in order shown in song_list_box)
    pub displayed_song_paths: RefCell<Vec<PathBuf>>,

    // Cached widget references (cloned from view_output!)
    pub current_track_label: gtk4::Label,
    pub play_pause_button: gtk4::Button,
    pub shuffle_button: gtk4::Button,
    pub repeat_button: gtk4::Button,
    pub volume_button: gtk4::Button,
    pub volume_scale: gtk4::Scale,
    pub track_progress_scale: gtk4::Scale,
    pub elapsed_time_label: gtk4::Label,
    pub duration_label: gtk4::Label,
    pub content_stack: gtk4::Stack,
    pub song_list_box: gtk4::ListBox,
    pub albums_list_box: gtk4::ListBox,
    pub artists_list_box: gtk4::ListBox,
    pub queue_list_box: gtk4::ListBox,
    pub playlist_list_box: gtk4::ListBox,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Page {
    Songs,
    Albums,
    Artists,
    Queue,
    Playlists,
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
            set_title: Some("MMP"),

            gtk4::Box {
                set_orientation: gtk4::Orientation::Vertical,

                // ========================================================
                // Playback Bar
                // ========================================================
                #[name(playback_bar)]
                gtk4::Box {
                    set_css_classes: &["playback-bar"],
                    set_spacing: 10,
                    set_margin_all: 4,

                    #[name(track_info_box)]
                    gtk4::Box {
                        set_orientation: gtk4::Orientation::Vertical,
                        set_halign: gtk4::Align::Start,
                        set_valign: gtk4::Align::Center,
                        set_spacing: 2,

                        #[name(current_track_label)]
                        gtk4::Label {
                            set_css_classes: &["track-info"],
                            set_label: "No track playing",
                            set_halign: gtk4::Align::Start,
                            set_ellipsize: gtk4::pango::EllipsizeMode::End,
                            set_max_width_chars: 30,
                        },
                    },

                    #[name(progress_box)]
                    gtk4::Box {
                        set_orientation: gtk4::Orientation::Vertical,
                        set_halign: gtk4::Align::Center,
                        set_valign: gtk4::Align::Center,
                        set_hexpand: true,
                        set_spacing: 2,

                        #[name(track_progress_scale)]
                        gtk4::Scale {
                            set_hexpand: true,
                            set_draw_value: false,
                            set_range: (0.0, 1.0),
                            set_increments: (1.0, 10.0),
                        },

                        gtk4::Box {
                            set_spacing: 6,
                            #[name(elapsed_time_label)]
                            gtk4::Label {
                                set_css_classes: &["time-label"],
                                set_label: "0:00",
                            },
                            gtk4::Box { set_hexpand: true },
                            #[name(duration_label)]
                            gtk4::Label {
                                set_css_classes: &["time-label", "duration-label"],
                                set_label: "0:00",
                            },
                        },
                    },

                    #[name(controls_box)]
                    gtk4::Box {
                        set_halign: gtk4::Align::End,
                        set_valign: gtk4::Align::Center,
                        set_spacing: 6,

                        #[name(prev_button)]
                        gtk4::Button {
                            set_css_classes: &["playback-button"],
                            set_label: "\u{23EE}",
                            connect_clicked => AppMsg::Previous,
                        },
                        #[name(play_pause_button)]
                        gtk4::Button {
                            set_css_classes: &["playback-button"],
                            set_label: "\u{25B6}",
                            connect_clicked => AppMsg::PlayPause,
                        },
                        #[name(next_button)]
                        gtk4::Button {
                            set_css_classes: &["playback-button"],
                            set_label: "\u{23ED}",
                            connect_clicked => AppMsg::Next,
                        },

                        gtk4::Separator {
                            set_orientation: gtk4::Orientation::Vertical,
                            set_margin_start: 6,
                            set_margin_end: 6,
                        },

                        #[name(shuffle_button)]
                        gtk4::Button {
                            set_css_classes: &["playback-button"],
                            set_label: "\u{1F500}",
                            connect_clicked => AppMsg::ShuffleToggled,
                        },
                        #[name(repeat_button)]
                        gtk4::Button {
                            set_css_classes: &["playback-button"],
                            set_label: "\u{1F501}",
                            connect_clicked => AppMsg::RepeatToggled,
                        },

                        gtk4::Separator {
                            set_orientation: gtk4::Orientation::Vertical,
                            set_margin_start: 6,
                            set_margin_end: 6,
                        },

                        #[name(volume_button)]
                        gtk4::Button {
                            set_css_classes: &["playback-button"],
                            set_label: "\u{1F50A}",
                            connect_clicked => AppMsg::MuteToggled,
                        },
                        #[name(volume_scale)]
                        gtk4::Scale {
                            set_css_classes: &["volume-scale"],
                            set_range: (0.0, 1.0),
                            set_draw_value: false,
                        },
                    },
                },

                // ========================================================
                // Content Area: Nav sidebar + Stack
                // ========================================================
                gtk4::Box {
                    set_hexpand: true,
                    set_vexpand: true,
                    set_spacing: 0,

                    #[name(nav_pane)]
                    gtk4::ScrolledWindow {
                        set_css_classes: &["nav-pane"],
                        set_hscrollbar_policy: gtk4::PolicyType::Never,

                        #[name(navigation_list)]
                        gtk4::ListBox {
                            set_css_classes: &["navigation-list"],

                            #[name(nav_header_library)]
                            gtk4::ListBoxRow {
                                set_css_classes: &["nav-header"],
                                set_selectable: false,
                                set_activatable: false,
                            },
                            #[name(nav_songs_row)]
                            gtk4::ListBoxRow {
                                set_css_classes: &["nav-sub-row"],
                            },
                            #[name(nav_albums_row)]
                            gtk4::ListBoxRow {
                                set_css_classes: &["nav-sub-row"],
                            },
                            #[name(nav_artists_row)]
                            gtk4::ListBoxRow {
                                set_css_classes: &["nav-sub-row"],
                            },

                            #[name(nav_header_playback)]
                            gtk4::ListBoxRow {
                                set_css_classes: &["nav-header"],
                                set_selectable: false,
                                set_activatable: false,
                            },
                            #[name(nav_queue_row)]
                            gtk4::ListBoxRow {
                                set_css_classes: &["nav-sub-row"],
                            },
                            #[name(nav_playlists_row)]
                            gtk4::ListBoxRow {
                                set_css_classes: &["nav-sub-row"],
                            },
                        },
                    },

                    #[name(content_box)]
                    gtk4::Box {
                        set_orientation: gtk4::Orientation::Vertical,
                        set_hexpand: true,
                        set_vexpand: true,
                        set_css_classes: &["content-page"],

                        #[name(search_entry)]
                        gtk4::SearchEntry {
                            set_placeholder_text: Some("Search library..."),
                            set_margin_bottom: 12,
                        },

                        #[name(content_stack)]
                        gtk4::Stack {
                            set_hexpand: true,
                            set_vexpand: true,

                            #[name(songs_page)]
                            gtk4::ScrolledWindow {
                                set_hscrollbar_policy: gtk4::PolicyType::Never,
                                #[name(song_list_box)]
                                gtk4::ListBox {
                                    set_css_classes: &["library-list"],
                                },
                            },

                            #[name(albums_page)]
                            gtk4::ScrolledWindow {
                                set_hscrollbar_policy: gtk4::PolicyType::Never,
                                #[name(albums_list_box)]
                                gtk4::ListBox {
                                    set_css_classes: &["library-list"],
                                },
                            },

                            #[name(artists_page)]
                            gtk4::ScrolledWindow {
                                set_hscrollbar_policy: gtk4::PolicyType::Never,
                                #[name(artists_list_box)]
                                gtk4::ListBox {
                                    set_css_classes: &["library-list"],
                                },
                            },

                            #[name(queue_page)]
                            gtk4::ScrolledWindow {
                                set_hscrollbar_policy: gtk4::PolicyType::Never,
                                #[name(queue_list_box)]
                                gtk4::ListBox {
                                    set_css_classes: &["library-list"],
                                },
                            },

                            #[name(playlists_page)]
                            gtk4::ScrolledWindow {
                                set_hscrollbar_policy: gtk4::PolicyType::Never,
                                #[name(playlist_list_box)]
                                gtk4::ListBox {
                                    set_css_classes: &["library-list"],
                                },
                            },
                        },
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

        // Set stack page names
        widgets.content_stack.add_titled(&widgets.songs_page, Some("songs"), "Songs");
        widgets.content_stack.add_titled(&widgets.albums_page, Some("albums"), "Albums");
        widgets.content_stack.add_titled(&widgets.artists_page, Some("artists"), "Artists");
        widgets.content_stack.add_titled(&widgets.queue_page, Some("queue"), "Queue");
        widgets.content_stack.add_titled(&widgets.playlists_page, Some("playlists"), "Playlists");

        // Build model with widget refs from view_output!
        let mut model = AppModel {
            library: Vec::new(),
            library_by_path: HashMap::new(),
            queue: QueueState::new(),
            playback: None,
            playback_rx: None,
            volume: 1.0,
            muted: false,
            library_db: None,
            playlists_db: None,
            playlists: Vec::new(),
            current_page: Page::Songs,
            current_playlist_id: 0,
            search_text: String::new(),
            search_lowered: String::new(),
            selected_artist: None,
            selected_album: None,
            tick: 0,
            displayed_song_paths: RefCell::new(Vec::new()),
            current_track_label: widgets.current_track_label.clone(),
            play_pause_button: widgets.play_pause_button.clone(),
            shuffle_button: widgets.shuffle_button.clone(),
            repeat_button: widgets.repeat_button.clone(),
            volume_button: widgets.volume_button.clone(),
            volume_scale: widgets.volume_scale.clone(),
            track_progress_scale: widgets.track_progress_scale.clone(),
            elapsed_time_label: widgets.elapsed_time_label.clone(),
            duration_label: widgets.duration_label.clone(),
            content_stack: widgets.content_stack.clone(),
            song_list_box: widgets.song_list_box.clone(),
            albums_list_box: widgets.albums_list_box.clone(),
            artists_list_box: widgets.artists_list_box.clone(),
            queue_list_box: widgets.queue_list_box.clone(),
            playlist_list_box: widgets.playlist_list_box.clone(),
        };

        // Open databases, load cached songs
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

        // Setup playback engine
        let (tx, rx) = mpsc::channel();
        let mut playback = Playback::new(tx);
        playback.set_volume(model.volume);
        model.playback = Some(playback);
        model.playback_rx = Some(rx);

        // Populate initial lists
        model.rebuild_song_list();
        model.rebuild_albums_list();
        model.rebuild_artists_list();
        model.rebuild_playlists_list();

        // Wire up event handlers
        let sender_clone = sender.clone();
        widgets.search_entry.connect_search_changed(move |entry| {
            sender_clone.input(AppMsg::SearchChanged(entry.text().to_string()));
        });

        let sender_clone = sender.clone();
        widgets.volume_scale.connect_change_value(move |_, _, value| {
            sender_clone.input(AppMsg::VolumeChanged(value));
            gtk4::glib::Propagation::Proceed
        });

        let sender_clone = sender.clone();
        widgets.track_progress_scale.connect_change_value(move |scale, _, value| {
            let seconds = value * scale.adjustment().upper();
            sender_clone.input(AppMsg::Seek(seconds));
            gtk4::glib::Propagation::Proceed
        });

        // Start periodic tick for playback event polling + UI updates
        let sender_clone = sender.clone();
        glib::timeout_add_local(std::time::Duration::from_millis(200), move || {
            sender_clone.input(AppMsg::Tick);
            glib::ControlFlow::Continue
        });

        // -- Set up navigation sidebar --
        // Header labels
        let lib_label = gtk4::Label::builder()
            .css_classes(["nav-header-label"]).label("Library")
            .halign(gtk4::Align::Start).margin_start(12).margin_top(12).build();
        widgets.nav_header_library.set_child(Some(&lib_label));

        let pb_label = gtk4::Label::builder()
            .css_classes(["nav-header-label"]).label("Playback")
            .halign(gtk4::Align::Start).margin_start(12).margin_top(12).build();
        widgets.nav_header_playback.set_child(Some(&pb_label));

        // Nav row labels
        for (row, text, msg) in [
            (&widgets.nav_songs_row, "Songs", AppMsg::NavSongs),
            (&widgets.nav_albums_row, "Albums", AppMsg::NavAlbums),
            (&widgets.nav_artists_row, "Artists", AppMsg::NavArtists),
            (&widgets.nav_queue_row, "Queue", AppMsg::NavQueue),
        ] {
            let label = gtk4::Label::builder()
                .css_classes(["row-label"]).label(text)
                .halign(gtk4::Align::Start).build();
            row.set_child(Some(&label));
        }
        // Playlists row
        {
            let label = gtk4::Label::builder()
                .css_classes(["row-label"]).label("Playlists")
                .halign(gtk4::Align::Start).build();
            widgets.nav_playlists_row.set_child(Some(&label));
        }

        // Connect row activation signals
        {
            let s = sender.clone();
            widgets.nav_songs_row.connect_activate(move |_| { s.input(AppMsg::NavSongs); });
        }
        {
            let s = sender.clone();
            widgets.nav_albums_row.connect_activate(move |_| { s.input(AppMsg::NavAlbums); });
        }
        {
            let s = sender.clone();
            widgets.nav_artists_row.connect_activate(move |_| { s.input(AppMsg::NavArtists); });
        }
        {
            let s = sender.clone();
            widgets.nav_queue_row.connect_activate(move |_| { s.input(AppMsg::NavQueue); });
        }
        {
            let s = sender.clone();
            widgets.nav_playlists_row.connect_activate(move |_| { s.input(AppMsg::NavPlaylist(0)); });
        }

        // -- Song list: double-click to play --
        {
            let s = sender.clone();
            widgets.song_list_box.connect_row_activated(move |list, row| {
                let idx = row.index() as usize;
                // Access displayed paths via a simple index lookup
                // We don't have direct access to model here, so we check
                // the row's child label text and match against library
                // For now, just do nothing — we'll add a proper msg later
            });
        }

        // Start directory scan in background
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
            AppMsg::NavPlaylist(id) => {
                self.current_playlist_id = id;
                self.current_page = Page::Songs;
            }
            AppMsg::SearchChanged(text) => {
                self.search_text = text.clone();
                self.search_lowered = text.to_lowercase();
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

    fn rebuild_song_list(&self) {
        self.song_list_box.remove_all();
        let mut paths = self.displayed_song_paths.borrow_mut();
        paths.clear();
        for idx in self.filtered_indices() {
            let song = &self.library[idx];
            let row = gtk4::ListBoxRow::new();
            row.set_child(Some(&gtk4::Label::builder()
                .css_classes(["song-row"]).label(&song.label())
                .halign(gtk4::Align::Start).build()));
            paths.push(song.path.clone());
            self.song_list_box.append(&row);
        }
    }

    fn rebuild_albums_list(&self) {
        self.albums_list_box.remove_all();
        for album in self.unique_albums() {
            let row = gtk4::ListBoxRow::new();
            row.set_child(Some(&gtk4::Label::builder()
                .css_classes(["song-row"]).label(&album)
                .halign(gtk4::Align::Start).build()));
            self.albums_list_box.append(&row);
        }
    }

    fn rebuild_artists_list(&self) {
        self.artists_list_box.remove_all();
        for artist in self.unique_artists() {
            let row = gtk4::ListBoxRow::new();
            row.set_child(Some(&gtk4::Label::builder()
                .css_classes(["song-row"]).label(&artist)
                .halign(gtk4::Align::Start).build()));
            self.artists_list_box.append(&row);
        }
    }

    fn rebuild_playlists_list(&self) {
        self.playlist_list_box.remove_all();
        for pl in &self.playlists {
            let row = gtk4::ListBoxRow::new();
            row.set_child(Some(&gtk4::Label::builder()
                .css_classes(["song-row"]).label(&pl.name)
                .halign(gtk4::Align::Start).build()));
            self.playlist_list_box.append(&row);
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
            self.queue_list_box.append(&row);
        }
    }

    fn sync_ui(&self) {
        let name = match self.current_page {
            Page::Songs => "songs", Page::Albums => "albums",
            Page::Artists => "artists", Page::Queue => "queue",
            Page::Playlists => "playlists",
        };
        self.content_stack.set_visible_child_name(name);

        if let Some(song) = self.current_song() {
            self.current_track_label.set_label(&song.label());
        } else {
            self.current_track_label.set_label("No track playing");
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

        self.volume_scale.set_value(self.volume);
        self.volume_button.set_label(if self.muted { "\u{1F507}" } else { "\u{1F50A}" });

        match self.current_page {
            Page::Songs => self.rebuild_song_list(),
            Page::Albums => self.rebuild_albums_list(),
            Page::Artists => self.rebuild_artists_list(),
            Page::Queue => self.rebuild_queue_list(),
            Page::Playlists => self.rebuild_playlists_list(),
        }
    }
}

fn format_time(seconds: f64) -> String {
    let total = seconds as u64;
    let mins = total / 60;
    let secs = total % 60;
    format!("{}:{:02}", mins, secs)
}

// ---------------------------------------------------------------------------
// Directory scanner
// ---------------------------------------------------------------------------

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
