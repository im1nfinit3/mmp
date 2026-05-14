//! Content pane — stack of library views (Songs, Albums, Artists, Queue, …).
//!
//! Relm4 sub-component.  Holds a `LibraryHandle` clone for read-only queries
//! (Q13-sub-C1).  Write mutations (playlist adds, etc.) are emitted as
//! `ContentPaneOutput`.

use std::cell::RefCell;
use std::path::PathBuf;
use std::rc::Rc;

use gtk4::prelude::*;
use relm4::prelude::*;
use relm4::RelmRemoveAllExt;

use crate::ui::Page;
use crate::library::db::{self};
use crate::library::song::Song;
use crate::library::LibraryHandle;
use crate::playback::PlaybackEvent;

// ---------------------------------------------------------------------------
// Model
// ---------------------------------------------------------------------------

pub struct ContentPane {
    /// Clone of the Library actor handle for read-only queries.
    library_handle: LibraryHandle,
    /// Currently visible page.
    current_page: Page,
    /// Currently selected playlist id (0 = none).
    current_playlist_id: i64,
    /// Search text for the songs page.
    search_text: String,
    /// Lowercased search text.
    search_lowered: String,
    /// Search text for the albums page.
    search_albums_text: String,
    search_albums_lowered: String,
    /// Search text for the artists page.
    search_artists_text: String,
    search_artists_lowered: String,
    /// Selected artist filter.
    selected_artist: Option<String>,
    /// Selected album filter.
    selected_album: Option<String>,
    /// Whether the visible list needs a rebuild.
    dirty_lists: bool,
    /// Paths currently displayed in the song list (shared with signal handler).
    displayed_song_paths: Rc<RefCell<Vec<PathBuf>>>,
    /// Path of the currently-playing track (for CSS highlighting).
    current_track_path: Option<PathBuf>,
    /// Cached playlist data (for context menus).
    playlists: Vec<db::Playlist>,
    /// Component sender (for emitting Output from context menus).
    sender: ComponentSender<Self>,

    // -- Widget refs --
    content_stack: gtk4::Stack,
    song_list_box: gtk4::ListBox,
    albums_list_box: gtk4::ListBox,
    artists_list_box: gtk4::ListBox,
    queue_list_box: gtk4::ListBox,
    songs_search_entry: gtk4::SearchEntry,
    albums_search_entry: gtk4::SearchEntry,
    artists_search_entry: gtk4::SearchEntry,
}

// ---------------------------------------------------------------------------
// Messages (Input)
// ---------------------------------------------------------------------------

#[derive(Debug, Clone)]
pub enum ContentPaneMsg {
    /// Switch to a different page.
    SetPage(Page),
    /// Update cached playlist data.
    SetPlaylists(Vec<db::Playlist>),
    /// New songs were added to the library.
    SongsAdded,
    /// Cached songs loaded from DB.
    SongsLoaded,
    /// The currently-playing track changed.
    CurrentTrackPath(Option<PathBuf>),
    /// Forwarded playback event (for current-track highlighting).
    PlaybackEvent(PlaybackEvent),
}

// ---------------------------------------------------------------------------
// Output (emitted to parent)
// ---------------------------------------------------------------------------

#[derive(Debug, Clone)]
pub enum ContentPaneOutput {
    /// Play a song from the library.
    PlayFromLibrary(PathBuf),
    /// Queue a song from the library.
    QueueFromLibrary(PathBuf),
    /// Add a song to an existing playlist.
    AddToPlaylist(i64, PathBuf),
    /// Add a song to a new playlist (name, path).
    AddToNewPlaylist(String, PathBuf),
    /// Save the current queue as a playlist.
    SaveQueueAsPlaylist(String),
    /// Navigate to the songs view with a search filter.
    NavSongsWithSearch(String),
}

// ---------------------------------------------------------------------------
// Component
// ---------------------------------------------------------------------------

#[relm4::component(pub)]
impl SimpleComponent for ContentPane {
    type Init = LibraryHandle;
    type Input = ContentPaneMsg;
    type Output = ContentPaneOutput;

    view! {
        #[root]
        #[name(content_stack)]
        gtk4::Stack {
            set_hexpand: true,
            set_vexpand: true,
            set_css_classes: &["content-stack"],
        }
    }

    fn init(
        library_handle: Self::Init,
        _root: Self::Root,
        sender: ComponentSender<Self>,
    ) -> ComponentParts<Self> {
        let widgets = view_output!();
        let content_stack = widgets.content_stack.clone();

        // -- Build sub-pages locally --
        let (songs_page, songs_search_entry, songs_list_box) =
            crate::ui::widgets::build_library_panel("Search songs");
        let (albums_page, albums_search_entry, albums_list_box) =
            crate::ui::widgets::build_library_panel("Search albums");
        let (artists_page, artists_search_entry, artists_list_box) =
            crate::ui::widgets::build_library_panel("Search artists");

        let queue_list_box = gtk4::ListBox::new();
        queue_list_box.add_css_class("library-list");
        queue_list_box.add_css_class("boxed-list");

        // Queue right-click → "Save queue as playlist"
        {
            let s = sender.clone();
            let gesture = gtk4::GestureClick::new();
            gesture.set_button(3);
            let qlb = queue_list_box.clone();
            gesture.connect_pressed(move |_gesture, _n, x, y| {
                let popover = gtk4::Popover::new();
                let vbox = gtk4::Box::new(gtk4::Orientation::Vertical, 0);
                vbox.add_css_class("context-menu");

                let btn = gtk4::Button::with_label("Save queue as playlist...");
                btn.add_css_class("context-menu-item");
                btn.set_halign(gtk4::Align::Start);
                let s = s.clone();
                let p = popover.clone();
                btn.connect_clicked(move |_| {
                    s.output(ContentPaneOutput::SaveQueueAsPlaylist(
                        String::new(),
                    )).ok();
                    p.popdown();
                });
                vbox.append(&btn);

                popover.set_child(Some(&vbox));
                popover.set_parent(&qlb);
                let rect = gtk4::gdk::Rectangle::new(
                    x as i32, y as i32, 1, 1,
                );
                popover.set_pointing_to(Some(&rect));
                popover.popup();
            });
            queue_list_box.add_controller(gesture);
        }
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

        content_stack.add_titled(&songs_page, Some("songs-view"), "Songs");
        content_stack.add_titled(&albums_page, Some("albums"), "Albums");
        content_stack.add_titled(&artists_page, Some("artists"), "Artists");
        content_stack.add_titled(&queue_page, Some("queue"), "Queue");
        content_stack.add_titled(&settings_page, Some("settings"), "Settings");

        // -- Search signals --
        {
            let s = sender.clone();
            songs_search_entry.connect_search_changed(move |_entry| {
                // Search is handled internally on rebuild
                s.input(ContentPaneMsg::SongsAdded); // trigger rebuild
            });
        }
        // TODO: wire albums/artists search properly in context menu phase

        // -- Song double-click → play --
        let displayed = Rc::new(RefCell::new(Vec::<PathBuf>::new()));
        {
            let s = sender.clone();
            let paths = Rc::clone(&displayed);
            songs_list_box.connect_row_activated(move |_list, row| {
                let idx = row.index() as usize;
                let p = paths.borrow();
                if let Some(path) = p.get(idx) {
                    s.output(ContentPaneOutput::PlayFromLibrary(
                        path.clone(),
                    ))
                    .ok();
                }
            });
        }

        // -- Albums/Artists click → filter songs --
        {
            let s = sender.clone();
            albums_list_box.connect_row_activated(move |_, row| {
                if let Some(child) = row.child()
                    && let Some(label) = child.downcast_ref::<gtk4::Label>() {
                        s.output(ContentPaneOutput::NavSongsWithSearch(
                            label.label().to_string(),
                        ))
                        .ok();
                    }
            });
        }
        {
            let s = sender.clone();
            artists_list_box.connect_row_activated(move |_, row| {
                if let Some(child) = row.child()
                    && let Some(label) = child.downcast_ref::<gtk4::Label>() {
                        s.output(ContentPaneOutput::NavSongsWithSearch(
                            label.label().to_string(),
                        ))
                        .ok();
                    }
            });
        }

        let model = ContentPane {
            library_handle,
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
            dirty_lists: true,
            displayed_song_paths: displayed,
            current_track_path: None,
            playlists: Vec::new(),
            sender,
            content_stack,
            song_list_box: songs_list_box,
            albums_list_box,
            artists_list_box,
            queue_list_box,
            songs_search_entry,
            albums_search_entry,
            artists_search_entry,
        };

        ComponentParts { model, widgets }
    }

    fn update(&mut self, msg: Self::Input, _sender: ComponentSender<Self>) {
        match msg {
            ContentPaneMsg::SetPage(page) => {
                self.current_page = page;
                self.current_playlist_id = 0;
                self.dirty_lists = true;
                self.sync_lists();
            }
            ContentPaneMsg::SetPlaylists(playlists) => {
                self.playlists = playlists;
            }
            ContentPaneMsg::SongsAdded | ContentPaneMsg::SongsLoaded => {
                self.dirty_lists = true;
                self.sync_lists();
            }
            ContentPaneMsg::CurrentTrackPath(path) => {
                self.current_track_path = path;
                self.dirty_lists = true;
                self.sync_lists();
            }
            ContentPaneMsg::PlaybackEvent(_event) => {
                // Handled for current-track highlighting if needed
            }
        }
    }
}

impl ContentPane {
    fn sync_lists(&mut self) {
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
            Page::Settings => {}
        }
        self.dirty_lists = false;
    }

    fn filtered_songs(&self) -> Vec<Song> {
        let search = self.search_lowered.clone();
        let artist = self.selected_artist.clone();
        let album = self.selected_album.clone();
        let filter: Box<dyn Fn(&Song) -> bool + Send + 'static> =
            Box::new(move |song| {
                if !search.is_empty() {
                    let tl = song.title.to_lowercase();
                    let al = song.artist.to_lowercase();
                    let bl = song.album.to_lowercase();
                    if !tl.contains(&search)
                        && !al.contains(&search)
                        && !bl.contains(&search)
                    {
                        return false;
                    }
                }
                if let Some(ref a) = artist
                    && song.artist != *a {
                        return false;
                    }
                if let Some(ref a) = album
                    && song.album != *a {
                        return false;
                    }
                true
            });
        self.library_handle.get_songs(filter)
    }

    fn rebuild_song_list(&mut self) {
        let songs = self.filtered_songs();
        self.song_list_box.remove_all();
        let mut paths = self.displayed_song_paths.borrow_mut();
        paths.clear();
        for song in &songs {
            let row = self.build_song_row(song);
            paths.push(song.path.clone());
            self.song_list_box.append(&row);
        }
    }

    fn build_song_row(&self, song: &Song) -> gtk4::ListBoxRow {
        let row_box = gtk4::Box::new(gtk4::Orientation::Horizontal, 12);
        row_box.add_css_class("song-row");

        let indicator = gtk4::Image::new();
        row_box.append(&indicator);

        let title_label = gtk4::Label::builder()
            .label(&song.title)
            .halign(gtk4::Align::Start)
            .hexpand(true)
            .ellipsize(gtk4::pango::EllipsizeMode::End)
            .build();
        row_box.append(&title_label);

        let artist_label = gtk4::Label::builder()
            .label(&song.artist)
            .css_classes(["dim-label"])
            .ellipsize(gtk4::pango::EllipsizeMode::Start)
            .max_width_chars(15)
            .build();
        row_box.append(&artist_label);

        let album_label = gtk4::Label::builder()
            .label(&song.album)
            .css_classes(["dim-label"])
            .ellipsize(gtk4::pango::EllipsizeMode::Start)
            .max_width_chars(15)
            .build();
        row_box.append(&album_label);

        let d_label = gtk4::Label::builder()
            .label(&song.duration_str)
            .css_classes(["dim-label", "duration-label"])
            .build();
        row_box.append(&d_label);

        let row = gtk4::ListBoxRow::new();
        row.set_child(Some(&row_box));

        // -- Right-click context menu --
        let path = song.path.clone();
        let playlists = self.playlists.clone();
        let lib = self.library_handle.clone();
        let sender = self.sender.clone();
        let row_weak = row.downgrade();
        let row_box_weak = row_box.downgrade();

        let gesture = gtk4::GestureClick::new();
        gesture.set_button(3); // right button
        gesture.connect_pressed(move |_gesture, _n_press, x, y| {
            let Some(_row) = row_weak.upgrade() else { return };
            let Some(row_box) = row_box_weak.upgrade() else { return };

            // Check which playlists already contain this song
            let in_playlists: Rc<std::collections::HashSet<i64>> =
                Rc::new(
                    lib.get_playlists()
                        .iter()
                        .filter_map(|pl| {
                            let songs = lib.get_playlist_songs(pl.id);
                            if songs.iter().any(|s| s.path == path) {
                                Some(pl.id)
                            } else {
                                None
                            }
                        })
                        .collect(),
                );

            let popover = gtk4::Popover::new();
            let vbox = gtk4::Box::new(gtk4::Orientation::Vertical, 0);
            vbox.add_css_class("context-menu");

            // "Add to playlist" header
            let header = gtk4::Label::new(Some("Add to playlist"));
            header.add_css_class("context-menu-header");
            header.set_margin_start(8);
            header.set_margin_end(8);
            header.set_margin_top(4);
            header.set_margin_bottom(4);
            vbox.append(&header);

            // Playlist items
            for pl in &playlists {
                let label = if in_playlists.contains(&pl.id) {
                    format!("✓ {} (already added)", pl.name)
                } else {
                    pl.name.clone()
                };
                let btn = gtk4::Button::with_label(&label);
                btn.add_css_class("context-menu-item");
                btn.set_halign(gtk4::Align::Start);
                if in_playlists.contains(&pl.id) {
                    btn.add_css_class("dim-label");
                }
                let path_c = path.clone();
                let pl_id = pl.id;
                let sender = sender.clone();
                let in_pl = Rc::clone(&in_playlists);
                let p = popover.clone();
                btn.connect_clicked(move |_| {
                    if !in_pl.contains(&pl_id) {
                        sender.output(ContentPaneOutput::AddToPlaylist(
                            pl_id,
                            path_c.clone(),
                        )).ok();
                    }
                    p.popdown();
                });
                vbox.append(&btn);
            }

            // Separator + "New playlist..."
            let sep = gtk4::Separator::new(gtk4::Orientation::Horizontal);
            vbox.append(&sep);

            let new_btn = gtk4::Button::with_label("New playlist...");
            new_btn.add_css_class("context-menu-item");
            new_btn.set_halign(gtk4::Align::Start);
            let path_c = path.clone();
            let sender = sender.clone();
            let p = popover.clone();
            new_btn.connect_clicked(move |_| {
                sender.output(ContentPaneOutput::AddToNewPlaylist(
                    String::new(),
                    path_c.clone(),
                )).ok();
                p.popdown();
            });
            vbox.append(&new_btn);

            popover.set_child(Some(&vbox));
            popover.set_parent(&row_box);
            let rect = gtk4::gdk::Rectangle::new(
                x as i32, y as i32, 1, 1,
            );
            popover.set_pointing_to(Some(&rect));
            popover.popup();
        });
        row.add_controller(gesture);

        // Highlight currently-playing track
        if let Some(ref current) = self.current_track_path
            && song.path == *current {
                row.add_css_class("current-track");
            }

        row
    }

    fn rebuild_albums_list(&self) {
        self.albums_list_box.remove_all();
        let albums = self.library_handle.get_unique_albums();
        let mut seen = std::collections::BTreeSet::new();
        for album in albums {
            let lowered = album.to_lowercase();
            if !self.search_albums_lowered.is_empty()
                && !lowered.contains(&self.search_albums_lowered)
            {
                continue;
            }
            if seen.contains(&album) {
                continue;
            }
            seen.insert(album.clone());
            let row = gtk4::ListBoxRow::new();
            row.set_child(Some(
                &gtk4::Label::builder()
                    .css_classes(["row-label"])
                    .label(&album)
                    .halign(gtk4::Align::Start)
                    .build(),
            ));
            self.albums_list_box.append(&row);
        }
    }

    fn rebuild_artists_list(&self) {
        self.artists_list_box.remove_all();
        let artists = self.library_handle.get_unique_artists();
        let mut seen = std::collections::BTreeSet::new();
        for artist in artists {
            let lowered = artist.to_lowercase();
            if !self.search_artists_lowered.is_empty()
                && !lowered.contains(&self.search_artists_lowered)
            {
                continue;
            }
            if seen.contains(&artist) {
                continue;
            }
            seen.insert(artist.clone());
            let row = gtk4::ListBoxRow::new();
            row.set_child(Some(
                &gtk4::Label::builder()
                    .css_classes(["row-label"])
                    .label(&artist)
                    .halign(gtk4::Align::Start)
                    .build(),
            ));
            self.artists_list_box.append(&row);
        }
    }

    fn rebuild_queue_list(&self) {
        // Queue state is owned by the parent; the content pane doesn't
        // have direct access.  For now this is a placeholder.
        self.queue_list_box.remove_all();
    }
}
