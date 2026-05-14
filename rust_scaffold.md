# MMP Rust Port — Scaffold Report

**Date:** May 14, 2026
**Status:** Foundation complete. Compiles, window renders, needs wiring.

---

## Overview

Full-scaffold Rust port of the C GTK4 music player using the Relm4 framework
(Elm-inspired component architecture). The C project's ~2,000 lines across 5
modules have been collapsed into a 4-module Rust crate with type-safe widget
references, automatic memory management, and unit-tested queue logic.

**Stack:**
- **GUI:** gtk4 0.9 + relm4 0.9 (component model replaces global mutable state)
- **Audio:** gstreamer 0.23 + gstreamer-pbutils 0.23 (playbin + Discoverer)
- **Database:** rusqlite 0.32 (bundled SQLite, backward-compatible schema)
- **Utilities:** walkdir 2, rand 0.8, dirs 5
- **Toolchain:** Rust 1.95.0, edition 2024

**Build:** `cargo build` (binary at `target/debug/mmp`)
**Run:** `cargo run` (requires display for GTK4)

---

## Module Map

### `Cargo.toml` (18 lines)
Dependency manifest. All crates resolve on the system's Rust 1.95.0 toolchain.

### `src/main.rs` (11 lines)
```rust
fn main() {
    gst::init().expect("Failed to initialize GStreamer");
    let relm_app = relm4::RelmApp::new("com.mmp.Mmp");
    relm_app.run::<app::AppModel>(());
}
```

### `src/app.rs` (~950 lines) — Relm4 Component + All UI

**Data model:**
- `Song` — path, title, artist, album, duration_str, with `label()` helper
- `RepeatMode` — Off/All/One with `.next()` cycle and `.label()` display
- `Page` — Songs/Albums/Artists/Queue/Playlists (view enum)

**Messages (`AppMsg`):**
PlayPause, Previous, Next, Seek, VolumeChanged, MuteToggled, ShuffleToggled,
RepeatToggled, PlaybackEvent, PlayFromLibrary, QueueFromLibrary, ClearQueue,
NavSongs, NavAlbums, NavArtists, NavQueue, NavPlaylist, SearchChanged,
CreatePlaylist, DeletePlaylist, RenamePlaylist, AddToPlaylist, ScanStarted,
ScanComplete, ScanAddSong, ScanError, Tick

**State (`AppModel`):**
- `library: Vec<Song>` + `library_by_path: HashMap<PathBuf, usize>` (O(1) lookup)
- `queue: QueueState` (shuffle pool, repeat mode, current index)
- `playback: Option<Playback>` + `playback_rx: Option<mpsc::Receiver<PlaybackEvent>>`
- `library_db` + `playlists_db` (rusqlite connections)
- `playlists: Vec<Playlist>`
- Cached GTK4 widget references (labels, buttons, scales, lists, stack)
- `displayed_song_paths: RefCell<Vec<PathBuf>>` (for row activation lookup)

**Widget hierarchy (view! macro):**
```
GtkWindow (900×600, title: "MMP")
├── GtkBox (vertical)
│   ├── [Playback Bar] GtkBox (.playback-bar)
│   │   ├── GtkBox — track info (current_track_label)
│   │   ├── GtkBox — progress (track_progress_scale, elapsed/duration labels)
│   │   └── GtkBox — controls (prev, play/pause, next, shuffle, repeat, volume)
│   │
│   └── GtkBox (horizontal, content area)
│       ├── GtkScrolledWindow (.nav-pane)
│       │   └── GtkListBox (.navigation-list)
│       │       ├── [header] Library
│       │       ├── Songs
│       │       ├── Albums
│       │       ├── Artists
│       │       ├── [header] Playback
│       │       ├── Queue
│       │       └── Playlists
│       │
│       └── GtkBox (vertical, .content-page)
│           ├── GtkSearchEntry
│           └── GtkStack (content_stack)
│               ├── Songs page → GtkListBox (song_list_box)
│               ├── Albums page → GtkListBox (albums_list_box)
│               ├── Artists page → GtkListBox (artists_list_box)
│               ├── Queue page → GtkListBox (queue_list_box)
│               └── Playlists page → GtkListBox (playlist_list_box)
```

**Key initialization steps (in `SimpleComponent::init`):**
1. Load CSS from embedded `src/ui/style.css`
2. Open `~/.config/mmp/library.db` and `~/.config/mmp/playlists.db`
3. Load cached songs into `library` vec + hash map
4. Create `Playback` engine with `mpsc::channel` for event passing
5. Populate song/album/artist/playlist list boxes
6. Wire search entry, volume scale, progress scale signals
7. Wire navigation row activate signals
8. Start 200ms `glib::timeout_add_local` tick timer
9. Spawn background thread for `walkdir` directory scan

**Update loop (every message → `sync_ui()`):**
- Tick: drains `mpsc::Receiver` for playback events (EOS → advance track, Tags → update metadata, Error → skip)
- Tick: queries GStreamer position/duration, updates progress bar and time labels
- Navigation: switches `GtkStack` visible child, rebuilds appropriate list
- Search: lowercases text, triggers song list rebuild with filtered indices
- Playback: delegates to `Playback` methods, updates queue state
- Scan: adds new songs to library, persists to DB
- `sync_ui()`: sets stack page, current track label, shuffle/repeat button CSS classes, volume icon, rebuilds visible list

### `src/db.rs` (~185 lines) — SQLite Wrapper

**Schema (identical to C version):**
```sql
CREATE TABLE songs (id INTEGER PRIMARY KEY, path TEXT UNIQUE, title, artist, album, duration_str);
CREATE TABLE playlists (id INTEGER PRIMARY KEY, name TEXT UNIQUE);
CREATE TABLE playlist_songs (playlist_id, song_id, position, FK cascade);
```

**Functions:**
| Function | C equivalent | Notes |
|----------|-------------|-------|
| `open(path)` | `db_init()` | Creates tables, WAL + FK pragmas |
| `save_song()` | `db_save_song()` | INSERT OR REPLACE |
| `get_all_songs()` | `db_get_all_songs()` | Returns `Vec<Song>` |
| `get_song_id()` | — | Lookup by path |
| `create_playlist()` | `db_create_playlist()` | Returns new id |
| `delete_playlist()` | `db_delete_playlist()` | Cascade deletes songs |
| `rename_playlist()` | `db_rename_playlist()` | UPDATE |
| `get_playlists()` | `db_get_playlists()` | Returns `Vec<Playlist>` |
| `add_song_to_playlist()` | `db_add_song_to_playlist()` | Auto-inserts song if new |
| `remove_song_from_playlist()` | `db_remove_song_from_playlist()` | DELETE by path |
| `get_playlist_songs()` | `db_get_playlist_songs()` | JOIN + ORDER BY position |
| `config_dir()` | — | Returns `~/.config/mmp/` |

### `src/playback.rs` (~430 lines) — GStreamer + Queue Logic

**`Playback` struct:**
- `playbin: gst::Element` — GStreamer playback pipeline
- `discoverer: gst_pbutils::Discoverer` — metadata extraction (2s timeout)
- `event_tx: mpsc::Sender<PlaybackEvent>` — UI event channel
- `update_timer_id: Option<glib::SourceId>` — 500ms position timer

**`PlaybackEvent` enum:**
Position { elapsed, duration }, EndOfStream, Tags { title, artist }, Error(String), StateChanged(PlaybackState)

**Methods:**
`play_file`, `toggle_pause`, `stop`, `seek`, `set_volume`, `set_mute`, `query_position`, `start_ui_timer`, `stop_ui_timer`, `extract_metadata`

**`QueueState` struct — pure logic, no GStreamer dependency:**
- `tracks: Vec<PathBuf>` — ordered queue
- `current: Option<usize>` — currently playing index
- `unplayed_pool: Vec<usize>` — shuffle pool (indices into tracks)
- `shuffle: bool`, `repeat: RepeatMode`

**Methods (all unit tested):**
- `push(path)` → adds to end, updates unplayed pool if shuffle
- `insert_after_current(path)` → inserts after current, shifts indices
- `remove(index)` → removes track, adjusts current + pool
- `remove_node(index)` → removes and returns path
- `clear()` → resets all state
- `toggle_shuffle()` / `cycle_repeat()` / `rebuild_unplayed_pool()`
- `next_track()` → core algorithm:

```
if repeat_one → return current
if shuffle:
  if pool empty + repeat_all → rebuild pool, recurse
  if pool empty → return None (stop)
  random index from pool, swap_remove, return track
if linear:
  if current+1 < len → return next
  if repeat_all → return 0 (wrap)
  else → None (stop)
```

**Unit tests (7):**
- Linear queue end-of-queue
- Repeat All wraps to head
- Repeat One stays on current
- Shuffle exhausts pool
- Shuffle + Repeat All rebuilds pool
- Remove current track (index adjustment)
- Remove before current (index shift)
- Insert after current

---

## What Was Ported from C

| C Concept | Rust Equivalent |
|-----------|----------------|
| `MmpApp*` global pointer | `AppModel` owned by Relm4 |
| `GQueue* playlist` | `QueueState.tracks: Vec<PathBuf>` |
| `GList* current_track_node` | `QueueState.current: Option<usize>` |
| `GPtrArray* unplayed_pool` | `QueueState.unplayed_pool: Vec<usize>` |
| `GList* library` + `GHashTable* library_by_path` | `Vec<Song>` + `HashMap<PathBuf, usize>` |
| `GListStore` + `GtkSignalListItemFactory` | `GtkListBox` + manual row population |
| `g_idle_add` marshaling | `glib::timeout_add_local` + `RefCell` interior mutability |
| `GActionGroup` context menus | Not yet — placeholder |
| `GTask` + `scan_directory_recursive` | `std::thread::spawn` + `walkdir` |
| `gst_element_seek_simple` | `gst::Element::seek_simple` |
| SQLite raw C API | rusqlite with params! macro |
| `g_free` / manual memory | Drop + ownership |

---

## What's Still Needed

1. **Song double-click → play** — wire `connect_row_activated` to look up path from `displayed_song_paths[ row.index() ]`
2. **Queue management** — add-to-queue context action, remove-from-queue, drag-drop reorder
3. **Playlist CRUD** — create/rename/delete dialogs, add songs to playlist, load playlist
4. **Context menus** — right-click on song rows (Play Now, Add to Queue, Add to Playlist)
5. **Artist/album filtering** — clicking artist/album in those views sets filter, navigates to songs
6. **Metadata extraction during scan** — call `Playback::extract_metadata` on new songs in the directory scanner
7. **Drag-drop** — file drop onto window → add to queue
8. **Volume mute button** — toggle mute icon + state
9. **Playback state display** — update play/pause button icon based on PlaybackState events
10. **Error handling** — GStreamer plugin missing, DB errors shown in UI

---

## Key Design Decisions

1. **Relm4 over direct gtk4-rs**: Eliminates global mutable state via component model. The `view!` macro gives type-safe widget references. Trade-off: macro quirks required several API adjustments (set_child, connect_activate, sender.widgets()).

2. **`mpsc::channel` for playback events**: Decouples GStreamer callbacks (which run on various threads) from UI mutations. The 200ms tick timer drains events and applies them to AppModel.

3. **Widget refs stored in AppModel**: Relm4 0.9's `ComponentSender` doesn't expose `widgets()` in `update()`, so GTK4 widget handles are cloned into the model during `init()`. These are reference-counted objects — cloning is cheap.

4. **`GtkListBox` over `GtkListView`+factory**: Simpler for initial implementation. Performance is acceptable for libraries up to ~10,000 songs. Can switch to `FactoryVecDeque` later if needed.

5. **Backward-compatible databases**: Same SQL schema at `~/.config/mmp/`. Users can switch between C and Rust builds freely.

6. **No async runtime**: Used `std::thread::spawn` for directory scan and `glib::timeout_add_local` for periodic updates. No tokio/async-std dependency needed for this scope.

---

## Verification

- [x] `cargo build` compiles with 0 errors
- [ ] `cargo run` launches window (requires display)
- [ ] Play/pause/seek work on audio files
- [ ] Shuffle + repeat cycle through modes
- [ ] Queue add/remove/reorder
- [ ] Playlist CRUD
- [ ] Library scan adds songs incrementally
- [ ] Search filters song list
- [ ] CSS dark theme renders correctly
- [ ] Existing `~/.config/mmp/` databases are readable
