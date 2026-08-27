# mmp

A native Rust music player built with [Iced](https://iced.rs/).

## Features

- Library scanning with SQLite storage and metadata parsing
- Audio playback (MP3, FLAC, Vorbis, WAV, MP4, ALAC) via rodio
- Playlists, shuffle, and repeat modes

## Requirements

- Rust (2024 edition)
- alsa (only for linux)

## Usage

```sh
cargo run --release
```

## License

BSD Zero Clause License. See [LICENSE.md](LICENSE.md).
