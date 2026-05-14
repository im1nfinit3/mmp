# The new architechture

## Library
 1. Handles everything related to the music library
    - Metadata (Song-  title, artist, album, year, duration... etc.)
    - Song should be a struct containing the metadata and path for the file.
    - Caching the song store (sqlite database)
    - Filtering the songs via a predicate function returning a vector of <Song>
 2. Playlists:
    - Backed by an sqlite db like in the current system
    - Should work exactly like a filter predicate from public api pov
    - Effectively something like 
        ```rust
        let songs = library::filter::from_all(|song| { if song.name in playlist.songs return true; });
        ```
 3. Provides an api to query the library for filtered lists of songs

## UI layer
 1. Presents the user interface
 2. Queries Library for lists of songs returned as vector of <Song>

## Playback system
 1. Just bothers with playback
 2. Provides apis that let for example ui get the name of the current track

