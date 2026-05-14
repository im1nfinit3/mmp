//! UI layer — Relm4 sub-components.
//!
//! Component tree:
//! ```text
//! AppModel (parent)
//! ├── PlaybackBar      — transport controls, progress, volume, track label
//! ├── NavPane          — navigation sidebar
//! └── ContentPane      — stack of library views
//! ```

/// Which page is visible in the content pane.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Page {
    RecentlyAdded,
    Songs,
    Albums,
    Artists,
    Queue,
    Settings,
}

pub mod content_pane;
pub mod nav_pane;
pub mod playback_bar;
pub mod widgets;
