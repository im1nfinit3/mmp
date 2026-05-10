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

Single-binary GTK4 desktop music player. Five source modules:

| File | Role |
|------|------|
| `src/main.c` | Entry point: GStreamer init, `GtkApplication`, signal wiring |
| `src/ui.c` | Window layout, `GListStore`-backed song/queue views with `GtkSignalListItemFactory`, library population, async directory scan, `ui_update_now_playing` for lightweight track-change UI updates |
| `src/ui_callbacks.c` | Signal handlers, context menus, drag-drop, playlist dialogs |
| `src/playback.c` | GStreamer `playbin`, queue, shuffle (via `GPtrArray` unplayed pool), repeat, seek, volume |
| `src/database.c` | SQLite wrapper: `songs`, `playlists`, `playlist_songs` tables |

Headers in `include/`. The central state struct `MmpApp` is in `include/app_state.h`.

A global `mmp_app` pointer (`extern` from `ui_callbacks.h`, defined in `ui.c`) holds all
app state — widgets, playback, queue, DB handles, filters, etc.

### Key data structures

- **`app->library`** is a `GList` of `Song*`. An accompanying `GHashTable* library_by_path`
  maps `song->path → Song*` for O(1) lookup — use `g_hash_table_lookup` instead of
  scanning `app->library`.
- **`app->unplayed_pool`** is a `GPtrArray` of `GList*` playlist-node pointers for
  shuffle mode. O(1) indexed access via `g_ptr_array_index`, O(1) add/remove
  via `g_ptr_array_add` / `g_ptr_array_remove_index_fast`.
- **Song/queue lists** use `GListStore` + `GtkSingleSelection` + `GtkListView` with
  `GtkSignalListItemFactory` (`setup`/`bind` callbacks). Track-change indicator updates
  go through `ui_update_now_playing`, not full store rebuilds. Full rebuilds via
  `ui_refresh_view` only on view/filter changes.

## Code generation

`glib-compile-resources` compiles `src/mmp.gresource.xml` + `src/ui/style.css` into
`build/mmp-resources.c` at build time. This file is auto-generated — do not edit it.
If you change CSS, a rebuild picks it up automatically via the CMake custom command.

## SQLite

`src/sqlite/sqlite3-all.c` is the amalgamation entrypoint that `#include`s
`sqlite3-1.c` through `sqlite3-9.c`, compiling the entire engine as one translation unit.
Only `sqlite3-all.c` appears in CMakeLists.txt.

Database files are created at runtime in `~/.config/mmp/`:
- `library.db` — cached song metadata
- `playlists.db` — user playlists

## Audio files

Scanned from `$XDG_MUSIC_DIR` on startup. Supported formats: `.mp3`, `.flac`, `.ogg`,
`.wav`, `.m4a`.

## Style

- C23, GLib allocation (`g_new0`, `g_free`, `g_strdup`, etc.)
- Build type: `MinSizeRel` (`-Os -DNDEBUG`)
- `compile_commands.json` is generated for clangd (cache at `.cache/clangd/`)
- No tests, no linter, no CI, no pre-commit hooks
