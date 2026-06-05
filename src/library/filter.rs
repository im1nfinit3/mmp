//! Helpers for deriving aggregate library views.

use std::collections::BTreeSet;

use super::song::Song;

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
