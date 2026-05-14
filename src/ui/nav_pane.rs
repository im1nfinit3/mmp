//! Navigation sidebar — page selection and playlist management.
//!
//! Relm4 sub-component.  The parent pushes playlist data via
//! `NavPaneMsg::SetPlaylists`.  All playlist CRUD and page selection
//! events are emitted as `NavPaneOutput`.

use gtk4::prelude::*;
use relm4::prelude::*;

use crate::ui::Page;
use crate::library::db;

// ---------------------------------------------------------------------------
// Model
// ---------------------------------------------------------------------------

pub struct NavPane {
    /// The GTK ListBox that holds nav rows.
    navigation_list: gtk4::ListBox,
    /// Reference to the "PLAYLISTS" header row (for insertion position).
    playlists_header: gtk4::ListBoxRow,
    /// Cached playlist data, pushed by parent.
    playlists: Vec<db::Playlist>,
    /// Component sender for emitting Output.
    sender: ComponentSender<Self>,
}

// ---------------------------------------------------------------------------
// Messages (Input)
// ---------------------------------------------------------------------------

#[derive(Debug, Clone)]
pub enum NavPaneMsg {
    /// Replace the playlist sub-rows with this list.
    SetPlaylists(Vec<db::Playlist>),
    /// User clicked a navigation row.
    PageClicked(Page),
}

// ---------------------------------------------------------------------------
// Output (emitted to parent)
// ---------------------------------------------------------------------------

#[derive(Debug, Clone)]
pub enum NavPaneOutput {
    /// User clicked a page.
    PageSelected(Page),
    /// User wants to create a new playlist.
    CreatePlaylist(String),
    /// User wants to delete a playlist.
    DeletePlaylist(i64),
    /// User wants to rename a playlist.
    RenamePlaylist(i64, String),
}

// ---------------------------------------------------------------------------
// Component
// ---------------------------------------------------------------------------

#[relm4::component(pub)]
impl SimpleComponent for NavPane {
    type Init = ();
    type Input = NavPaneMsg;
    type Output = NavPaneOutput;

    view! {
        #[root]
        gtk4::Box {
            set_orientation: gtk4::Orientation::Vertical,
            set_css_classes: &["nav-pane"],

            #[name(navigation_list)]
            gtk4::ListBox {
                set_css_classes: &["navigation-list"],
                set_vexpand: true,

                // -- Static nav rows --
                #[name = "nav_recently_added"]
                gtk4::ListBoxRow {
                    connect_activate => NavPaneMsg::PageClicked(Page::RecentlyAdded),
                },
                #[name = "nav_albums"]
                gtk4::ListBoxRow {
                    connect_activate => NavPaneMsg::PageClicked(Page::Albums),
                },
                #[name = "nav_artists"]
                gtk4::ListBoxRow {
                    connect_activate => NavPaneMsg::PageClicked(Page::Artists),
                },
                #[name = "nav_songs"]
                gtk4::ListBoxRow {
                    connect_activate => NavPaneMsg::PageClicked(Page::Songs),
                },
                #[name = "nav_queue"]
                gtk4::ListBoxRow {
                    connect_activate => NavPaneMsg::PageClicked(Page::Queue),
                },

                #[name(playlists_header)]
                gtk4::ListBoxRow {
                    set_css_classes: &["nav-header"],
                    set_selectable: false,
                    set_activatable: false,
                },

                #[name = "nav_settings"]
                gtk4::ListBoxRow {
                    connect_activate => NavPaneMsg::PageClicked(Page::Settings),
                },
            },
        }
    }

    fn init(
        _init: Self::Init,
        _root: Self::Root,
        sender: ComponentSender<Self>,
    ) -> ComponentParts<Self> {
        let widgets = view_output!();

        // Label static nav rows
        label_row(&widgets.nav_recently_added, "Recently added");
        label_row(&widgets.nav_albums, "Albums");
        label_row(&widgets.nav_artists, "Artists");
        label_row(&widgets.nav_songs, "Songs");
        label_row(&widgets.nav_queue, "Queue");
        label_row(&widgets.nav_settings, "Settings");

        // "PLAYLISTS" header with right-click → Create
        {
            let label = gtk4::Label::builder()
                .css_classes(["row-label"])
                .label("PLAYLISTS")
                .halign(gtk4::Align::Start)
                .build();
            widgets.playlists_header.set_child(Some(&label));

            let s = sender.clone();
            let row = widgets.playlists_header.clone();
            let gesture = gtk4::GestureClick::new();
            gesture.set_button(3);
            gesture.connect_pressed(move |_, _n, x, y| {
                let popover = gtk4::Popover::new();
                let vbox = gtk4::Box::new(gtk4::Orientation::Vertical, 0);
                vbox.add_css_class("context-menu");

                let btn = gtk4::Button::with_label("Create playlist...");
                btn.add_css_class("context-menu-item");
                btn.set_halign(gtk4::Align::Start);
                let s = s.clone();
                let p = popover.clone();
                btn.connect_clicked(move |_| {
                    // Trigger CreatePlaylist output — parent shows dialog
                    s.output(NavPaneOutput::CreatePlaylist(String::new()))
                        .ok();
                    p.popdown();
                });
                vbox.append(&btn);

                popover.set_child(Some(&vbox));
                popover.set_parent(&row);
                let rect = gtk4::gdk::Rectangle::new(
                    x as i32, y as i32, 1, 1,
                );
                popover.set_pointing_to(Some(&rect));
                popover.popup();
            });
            widgets.playlists_header.add_controller(gesture);
        }

        // Select "Recently added" by default
        widgets
            .navigation_list
            .select_row(Some(&widgets.nav_recently_added));

        let model = NavPane {
            navigation_list: widgets.navigation_list.clone(),
            playlists_header: widgets.playlists_header.clone(),
            playlists: Vec::new(),
            sender: sender.clone(),
        };

        ComponentParts { model, widgets }
    }

    fn update(&mut self, msg: Self::Input, sender: ComponentSender<Self>) {
        match msg {
            NavPaneMsg::SetPlaylists(playlists) => {
                self.playlists = playlists;
                self.rebuild_playlist_rows();
            }
            NavPaneMsg::PageClicked(page) => {
                let _ = sender.output(NavPaneOutput::PageSelected(page));
            }
        }
    }
}

impl NavPane {
    /// Remove old playlist sub-rows and insert new ones.
    fn rebuild_playlist_rows(&self) {
        // Collect indices of existing sub-rows
        let mut to_remove = Vec::new();
        for i in 0.. {
            let row = self.navigation_list.row_at_index(i);
            if row.is_none() {
                break;
            }
            let row = row.unwrap();
            if row.has_css_class("nav-sub-row") && row != self.playlists_header {
                to_remove.push(i);
            }
        }
        for i in to_remove.into_iter().rev() {
            if let Some(row) = self.navigation_list.row_at_index(i) {
                self.navigation_list.remove(&row);
            }
        }

        // Insert new rows after the header
        let header_idx = self.playlists_header.index();
        let mut pos = header_idx + 1;
        for pl in &self.playlists {
            let row = gtk4::ListBoxRow::new();
            row.add_css_class("nav-sub-row");
            row.set_child(Some(
                &gtk4::Label::builder()
                    .css_classes(["row-label"])
                    .label(&pl.name)
                    .halign(gtk4::Align::Start)
                    .build(),
            ));

            // Right-click context menu: Rename / Delete
            let pl_id = pl.id;
            let s = self.sender.clone();
            let row_weak = row.downgrade();
            let gesture = gtk4::GestureClick::new();
            gesture.set_button(3);
            gesture.connect_pressed(move |_, _n, x, y| {
                let Some(row) = row_weak.upgrade() else { return };

                let popover = gtk4::Popover::new();
                let vbox = gtk4::Box::new(gtk4::Orientation::Vertical, 0);
                vbox.add_css_class("context-menu");

                let rename_btn = gtk4::Button::with_label("Rename");
                rename_btn.add_css_class("context-menu-item");
                rename_btn.set_halign(gtk4::Align::Start);
                let s1 = s.clone();
                let p1 = popover.clone();
                rename_btn.connect_clicked(move |_| {
                    s1.output(NavPaneOutput::RenamePlaylist(
                        pl_id,
                        String::new(),
                    )).ok();
                    p1.popdown();
                });
                vbox.append(&rename_btn);

                let delete_btn = gtk4::Button::with_label("Delete");
                delete_btn.add_css_class("context-menu-item");
                delete_btn.set_halign(gtk4::Align::Start);
                let s2 = s.clone();
                let p2 = popover.clone();
                delete_btn.connect_clicked(move |_| {
                    s2.output(NavPaneOutput::DeletePlaylist(pl_id))
                        .ok();
                    p2.popdown();
                });
                vbox.append(&delete_btn);

                popover.set_child(Some(&vbox));
                popover.set_parent(&row);
                let rect = gtk4::gdk::Rectangle::new(
                    x as i32, y as i32, 1, 1,
                );
                popover.set_pointing_to(Some(&rect));
                popover.popup();
            });
            row.add_controller(gesture);

            self.navigation_list.insert(&row, pos);
            pos += 1;
        }
    }
}

/// Helper: set a simple label on a ListBoxRow.
fn label_row(row: &gtk4::ListBoxRow, text: &str) {
    let label = gtk4::Label::builder()
        .css_classes(["row-label"])
        .label(text)
        .halign(gtk4::Align::Start)
        .build();
    row.set_child(Some(&label));
}
