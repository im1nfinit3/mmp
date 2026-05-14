//! Filter predicates for the music library.
//!
//! Playlists are treated as filter predicates: `library.get_songs(|song| playlist.contains(song.id))`.

use std::collections::BTreeSet;

use super::song::Song;

/// Type alias for a song filter function used in library queries.
pub type FilterFn = Box<dyn Fn(&Song) -> bool + Send + 'static>;

/// Filter the library by search text, artist, album, and an optional predicate.
pub fn filter_songs<'a>(
    songs: &'a [Song],
    search_lowered: &str,
    selected_artist: &Option<String>,
    selected_album: &Option<String>,
    extra_predicate: Option<&FilterFn>,
) -> Vec<&'a Song> {
    songs
        .iter()
        .filter(|song| {
            if !search_lowered.is_empty() {
                let tl = song.title.to_lowercase();
                let al = song.artist.to_lowercase();
                let bl = song.album.to_lowercase();
                if !tl.contains(search_lowered)
                    && !al.contains(search_lowered)
                    && !bl.contains(search_lowered)
                {
                    return false;
                }
            }
            if let Some(a) = selected_artist {
                if song.artist != *a {
                    return false;
                }
            }
            if let Some(a) = selected_album {
                if song.album != *a {
                    return false;
                }
            }
            if let Some(pred) = extra_predicate {
                if !pred(song) {
                    return false;
                }
            }
            true
        })
        .collect()
}

/// Extract unique artists from a song slice, sorted.
pub fn unique_artists(songs: &[Song]) -> Vec<String> {
    let set: BTreeSet<String> = songs
        .iter()
        .map(|s| s.artist.clone())
        .filter(|a| !a.is_empty())
        .collect();
    set.into_iter().collect()
}

/// Extract unique albums from a song slice, sorted.
pub fn unique_albums(songs: &[Song]) -> Vec<String> {
    let set: BTreeSet<String> = songs
        .iter()
        .map(|s| s.album.clone())
        .filter(|a| !a.is_empty())
        .collect();
    set.into_iter().collect()
}
