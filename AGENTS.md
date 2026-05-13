# AGENTS.md

## Build

```bash
cmake -B build -S . && cmake --build build
```

Binary at `build/mmp`. Out-of-source build only — never edit files in `build/`.

## Dependencies

- **GTK4** (pkg-config: `gtk4`)
- **GStreamer 1.0** (pkg-config: `gstreamer-1.0`, `gstreamer-pbutils-1.0`)
- **glib-compile-resources** (from GLib2 dev package)
- GCC (C23 / `-std=gnu2x`), CMake >= 3.10

## Architecture

Single-binary GTK4 desktop music player. Three static libraries + executable:

| Target | Location | Role |
|--------|----------|------|
| `sqlite` | `lib/sqlite/` | Vendored SQLite amalgamation — only `sqlite3-all.c` compiled, rest `#include`d |
| `mmp_core` | `lib/mmp_core/` | Two GObject types: `MmpPlayback` (GStreamer URI audio + signals) and `MmpLibrary` (metadata, queue, shuffle/repeat, SQLite DB, directory scan, playback signal wiring) |
| `mmp_ui` | `lib/mmp_ui/` | GTK4 widgets, views, signal handlers, context menus, dialogs — depends on `mmp_core`, zero GStreamer |
| `mmp` | `src/main.c` | Thin shell: constructs `MmpPlayback` → `MmpLibrary` → `MmpUI`, wires library signals, runs `gtk_application_run()` |

Public headers in `include/`:

| Header | Layer | Contents |
|--------|-------|----------|
| `mmp_types.h` | shared | `Song`, `RepeatMode`, `SongFilter`, `SongFilterFunc` — pure data, no deps |
| `mmp_playback.h` | core | Opaque `MmpPlayback` GObject, `MmpPlaybackState` enum, public API — zero GTK, zero Song |
| `mmp_library.h` | core | Opaque `MmpLibrary` GObject, `Playlist`, Song lifecycle (`free_song`, `mmp_song_copy`, `MMP_TYPE_SONG`), public API |
| `mmp_ui.h` | ui | Opaque `MmpUI`, lifecycle API — depends on GTK |

An internal header `lib/mmp_ui/mmp_ui_internal.h` (not in `include/`) defines the full `MmpUI` struct and `MmpSongItem` GObject, shared by `ui.c` and `ui_callbacks.c`.

### Dependency DAG

```
sqlite ──► mmp_core (GStreamer) ──► mmp_ui (GTK) ──► mmp
```

No circular dependencies. `mmp_ui` never includes GStreamer headers.

### Cross-layer communication

`MmpPlayback` and `MmpLibrary` use **GObject signals**:

| Signal | Emitter | Payload | Consumer |
|--------|---------|---------|----------|
| `eos` | `MmpPlayback` | void | `MmpLibrary` — picks next track |
| `tag-received` | `MmpPlayback` | artist, title strings | `MmpLibrary` — updates Song metadata if missing |
| `error` | `MmpPlayback` | message string | `MmpLibrary` — logs, emits `now-playing-changed(NULL)` |
| `state-changed` | `MmpPlayback` | `MmpPlaybackState` enum | `MmpLibrary` — emits `now-playing-changed(song)` |
| `queue-changed` | `MmpLibrary` | void | `MmpUI` — rebuilds queue `GListStore` |
| `now-playing-changed` | `MmpLibrary` | `Song*` (nullable) | `MmpUI` — updates label, button, row indicators |
| `song-added` | `MmpLibrary` | `Song*` | `MmpUI` — creates `MmpSongItem`, updates views |
| `song-updated` | `MmpLibrary` | `Song*` | `MmpUI` — refreshes song row |
| `playlists-changed` | `MmpLibrary` | void | `MmpUI` — rebuilds playlist nav rows |

GTK widget signals (button clicks, range changes, etc.) pass specific typed pointers as `user_data` — no global context struct.

### Key data structures

- **`MmpLibrary.songs`** is a `GList` of `Song*`. An accompanying `GHashTable* songs_by_path` maps `song->path → Song*` for O(1) lookup — use `mmp_library_find_song()` instead of scanning.
- **`MmpLibrary.unplayed_pool`** is a `GPtrArray` of `GList*` queue-node pointers for shuffle mode. O(1) indexed access via `g_ptr_array_index`, O(1) add/remove via `g_ptr_array_add` / `g_ptr_array_remove_index_fast`.
- **Song/queue views** use `GListStore` + `GtkSingleSelection` + `GtkListView` with `GtkSignalListItemFactory` (`setup`/`bind` callbacks). Track-change indicator updates go through `ui_update_now_playing`, not full store rebuilds. Full rebuilds via `ui_refresh_view` only on view/filter changes.

## Code generation

`glib-compile-resources` compiles `src/mmp.gresource.xml` + `src/ui/style.css` into `build/mmp-resources.c` at build time. This file is auto-generated — do not edit it. If you change CSS, a rebuild picks it up automatically via the CMake custom command.

## SQLite

`lib/sqlite/sqlite3-all.c` is the amalgamation entrypoint that `#include`s `sqlite3-1.c` through `sqlite3-9.c`, compiling the entire engine as one translation unit. Only `sqlite3-all.c` appears in CMakeLists.txt.

Database files are created at runtime in `~/.config/mmp/`:
- `library.db` — cached song metadata
- `playlists.db` — user playlists

## Audio files

Scanned from `$XDG_MUSIC_DIR` on startup. Supported formats: `.mp3`, `.flac`, `.ogg`, `.wav`, `.m4a`.

## Style

- C23, GLib allocation (`g_new0`, `g_free`, `g_strdup`, etc.)
- Build type: `MinSizeRel` (`-Os -DNDEBUG`)
- `compile_commands.json` is generated for clangd (cache at `.cache/clangd/`)
- No tests, no linter, no CI, no pre-commit hooks
