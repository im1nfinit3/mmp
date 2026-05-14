//! Widget builders for the media player UI.
//!
//! Phase A: standalone widget constructors extracted from `app.rs`.

use gtk4::prelude::*;

/// Format seconds as `m:ss` or `mm:ss`.
pub fn format_time(seconds: f64) -> String {
    let total = seconds as u64;
    let mins = total / 60;
    let secs = total % 60;
    format!("{}:{:02}", mins, secs)
}

/// Build a page with a search entry and a list box.
pub fn build_library_panel(placeholder: &str) -> (gtk4::Box, gtk4::SearchEntry, gtk4::ListBox) {
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

/// Build a navigation row with a label.
pub fn build_nav_row(row: &gtk4::ListBoxRow, label_text: &str, _page_name: &str) {
    let label = gtk4::Label::builder()
        .css_classes(["row-label"])
        .label(label_text)
        .halign(gtk4::Align::Start)
        .build();
    row.set_child(Some(&label));
}
