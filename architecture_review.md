# Architecture Review: `mmp_rearchitecht.md`

**Date:** May 14, 2026
**Status:** Review of user's draft architecture for the Rust port.

---

## Original Proposal (summary)

Three clean layers:

- **Library** — owns song data, metadata, SQLite caching, filter predicates.
  Playlists are just filter predicates over the library.
- **UI** — presents the interface, queries Library for `Vec<Song>`, owns no song data.
- **Playback** — audio only, exposes APIs to query current track info.

---

## What's Strong

### Separation of concerns

Library owns data + filtering, UI owns presentation, Playback owns audio. This
eliminates the current `AppModel` monolith where library state, UI state, queue
state, and playback state are all one struct. Each layer has a single
responsibility.

### Playlists as predicates

Treating a playlist as a filter function over the library is elegant:

```rust
let songs = library.filter(|song| playlist.contains(song));
```

The UI doesn't need special playlist display logic — it asks the library for a
filtered `Vec<Song>` and renders it like any other view. Playlists compose
cleanly with search and other filters.

### UI queries the library — single source of truth

The current code has `AppModel.library: Vec<Song>`, `displayed_song_paths:
RefCell<Vec<PathBuf>>`, and widget state — multiple sources of truth that drift.
The proposed approach makes the library the single authority on song data. The
UI never duplicates it, only queries it.

---

## Four Refinements

### 1. Thread boundary and ownership

The Library layer handles scanning, caching, and filtering. But scanning
requires `GstDiscoverer` for metadata extraction, which must run on the GLib
main thread (it uses GStreamer's GLib integration).

| Approach | Trade-off |
|----------|-----------|
| Library owns `GstDiscoverer`, runs on main thread | Simple, but scanning blocks UI unless aggressively batched |
| Library runs on its own thread, sends paths to a main-thread metadata worker via channel | Clean separation, more plumbing |
| **Skip GstDiscoverer during scan, extract metadata lazily on first play** | Simplest, metadata populates gradually, no blocking |

**Recommendation:** Option C for now. The directory scanner collects file paths
only (fast, no GStreamer dependency). Metadata (title, artist, album, duration)
is extracted on first play via the Playback layer and cached back to the
Library. This keeps the Library thread truly non-blocking and avoids the
thread-boundary problem entirely.

Later, a metadata worker on the main thread could pre-populate metadata
asynchronously without blocking the initial scan.

### 2. Reactivity: how does the UI know to refresh?

When the library changes (scan adds songs, metadata updates), the UI needs to
know to re-query. The document doesn't specify this mechanism.

**Proposal:** Library sends events via a channel, UI subscribes:

```rust
enum LibraryEvent {
    SongsAdded(usize),           // N new songs available
    ScanStarted,
    ScanComplete { total: usize },
    MetadataUpdated(PathBuf),
}
```

The UI component holds a `mpsc::Receiver<LibraryEvent>` (matching the existing
`mpsc::Receiver<PlaybackEvent>` pattern). On each event, the UI re-queries the
library for the current view's filtered song list and rebuilds the display.

This means the UI never polls — it's event-driven. Blank until the first event
arrives, then populated. Exactly the "display content when there is content,
display blank when there isn't" behavior.

### 3. Concrete type assignments

Mapping current types to the proposed layers:

| Current type / module | Proposed layer | Notes |
|------------------------|---------------|-------|
| `Song`, `RepeatMode` | **Library** | Pure data structs owned by the library |
| `db.rs` (SQLite schema, queries) | **Library** | Implementation detail of library persistence |
| `filtered_indices()`, filter predicates | **Library** | Library is the filter engine |
| `scan_directory()`, `walkdir` traversal | **Library** | File discovery is a library concern |
| `db::Playlist`, playlist CRUD | **Library** | Playlists are stored data |
| `QueueState`, track ordering, shuffle pool | **Playback** | Queue ordering is playback logic, not library |
| `Playback`, `PlaybackEvent`, GStreamer pipeline | **Playback** | Audio engine |
| `AppModel` widget refs, `Page`, search text, `sync_*` | **UI** | Widget handles, view state, presentation |
| `build_library_panel()`, `build_nav_row()`, `build_song_row()` | **UI** | Widget builders |

### 4. Module structure

```
src/
  main.rs              — GStreamer init, launch Relm4 app
  library/
    mod.rs             — Library struct, public query API, LibraryEvent
    song.rs            — Song, metadata types (title, artist, album, duration, year...)
    filter.rs          — FilterFn type, filter chaining, playlist-as-predicate
    db.rs              — SQLite (songs table + playlists tables)
    scan.rs            — walkdir traversal, file discovery
  playback/
    mod.rs             — PlaybackEngine, PlaybackEvent, QueueState, RepeatMode
    gst.rs             — GStreamer playbin, discoverer, bus messages
  ui/
    mod.rs             — Relm4 AppModel, AppMsg, update loop
    widgets.rs         — build_library_panel, build_nav_row, build_song_row
    style.css
```

---

## Concurrency Model

The key architectural insight: **Library runs on its own thread as a
message-passing actor.** The UI never blocks waiting for the library.

```
                    ┌─────────────────────┐
  UI thread         │  Relm4 AppModel     │
  (GTK main loop)   │  owns widgets       │
                    │  holds rx channels  │
                    └───┬─────────┬───────┘
                        │ queries │ events
                        ▼         ▲
                    ┌─────────────────────┐
  Library thread    │  Library actor      │
  (background)      │  owns DB, songs     │
                    │  runs scan          │
                    │  sends LibraryEvent │
                    └─────────────────────┘
                        │
                    ┌─────────────────────┐
  Playback          │  GStreamer          │
  (internal thread) │  pipeline           │
                    │  sends PlaybackEvent│
                    └─────────────────────┘
```

**Library actor API sketch:**

```rust
struct Library {
    tx: mpsc::Sender<LibraryCommand>,
}

enum LibraryCommand {
    GetSongs { filter: Filter, reply: oneshot::Sender<Vec<Song>> },
    StartScan,
    // ...
}

impl Library {
    fn new(event_tx: mpsc::Sender<LibraryEvent>) -> Self { /* spawn thread */ }
    fn get_songs(&self, filter: Filter) -> Vec<Song> { /* send command, await reply */ }
}
```

The `get_songs()` call blocks the UI thread briefly (a channel round-trip), but
the library is just doing a HashMap + Vec filter — microseconds even for large
libraries. The expensive operation (directory scan) happens on the library
thread and sends `LibraryEvent`s back as progress.

---

## Migration Path

This can be done incrementally without breaking the build at each step:

1. **Extract types** — move `Song`, `RepeatMode`, filter logic into `library/`
   as free functions. No behavioral change yet. `db.rs` moves to `library/db.rs`.
2. **Make Library a struct** — wrap the SQLite connection + song Vec + HashMap
   into a `Library` struct with methods. `AppModel` holds a `Library` instead
   of raw fields.
3. **Add LibraryEvent channel** — Library sends events when scan adds songs.
   UI subscribes. Still single-threaded, but the interface is ready.
4. **Move Library to its own thread** — spawn a thread, use `mpsc` + `oneshot`
   for query/response. Library actor pattern. UI never blocks on library work.
5. **Thin out AppModel** — remove library-owned fields (`library`, `library_by_path`,
   `library_db`, `playlists_db`, `playlists`), replace with a `Library` handle
   and a `Receiver<LibraryEvent>`.
6. **Move QueueState to Playback** — it's playback ordering, not library data.
   Already somewhat separated today.

Each step compiles and runs independently. No big-bang rewrite needed.
