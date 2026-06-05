//! Queue state for linear + shuffle playback.
//!
//! Pure logic — no GStreamer dependency. Unit-tested.

use std::path::PathBuf;

use rand::Rng;

use crate::library::song::RepeatMode;

/// Queue state for linear + shuffle playback.
pub struct QueueState {
    /// Ordered list of file paths in the playback queue.
    pub tracks: Vec<PathBuf>,
    /// Index of the currently playing track (None if nothing playing).
    pub current: Option<usize>,
    /// Indices into `tracks` of unplayed songs (for shuffle mode).
    pub unplayed_pool: Vec<usize>,
    pub shuffle: bool,
    pub repeat: RepeatMode,
}

impl QueueState {
    pub fn new() -> Self {
        Self {
            tracks: Vec::new(),
            current: None,
            unplayed_pool: Vec::new(),
            shuffle: false,
            repeat: RepeatMode::Off,
        }
    }

    /// Add a track to the end of the queue.
    /// Returns the index of the newly added track.
    pub fn push(&mut self, path: PathBuf) -> usize {
        let idx = self.tracks.len();
        self.tracks.push(path);
        if self.shuffle {
            self.unplayed_pool.push(idx);
        }
        idx
    }

    /// Remove a track at the given index.
    pub fn remove(&mut self, index: usize) {
        if index >= self.tracks.len() {
            return;
        }
        self.tracks.remove(index);

        // Adjust current
        if let Some(ref mut cur) = self.current {
            if index == *cur {
                // Current track was removed — move to the next, or clear
                if *cur < self.tracks.len() {
                    // next track shifted into position, stay put
                } else if !self.tracks.is_empty() {
                    *cur = self.tracks.len() - 1;
                } else {
                    self.current = None;
                }
            } else if index < *cur {
                *cur -= 1;
            }
        }

        // Adjust unplayed_pool: remove index, shift others
        self.unplayed_pool.retain(|&i| i != index);
        for idx in &mut self.unplayed_pool {
            if *idx > index {
                *idx -= 1;
            }
        }
    }

    /// Toggle shuffle mode on/off.
    pub fn toggle_shuffle(&mut self) {
        self.shuffle = !self.shuffle;
        if self.shuffle {
            self.rebuild_unplayed_pool();
        } else {
            self.unplayed_pool.clear();
        }
    }

    /// Cycle repeat mode: Off → All → One → Off.
    pub fn cycle_repeat(&mut self) {
        self.repeat = self.repeat.next();
    }

    /// Rebuild the unplayed pool: all queue indices EXCEPT the current one.
    pub fn rebuild_unplayed_pool(&mut self) {
        self.unplayed_pool.clear();
        for i in 0..self.tracks.len() {
            if Some(i) != self.current {
                self.unplayed_pool.push(i);
            }
        }
    }

    /// Determine the next track index to play.
    /// Returns None if playback should stop.
    pub fn next_track(&mut self) -> Option<usize> {
        if self.repeat == RepeatMode::One {
            return self.current;
        }

        if self.shuffle {
            if self.unplayed_pool.is_empty() {
                if self.repeat == RepeatMode::All {
                    self.rebuild_unplayed_pool();
                    return self.next_track(); // recurse (tail-recursive, won't blow stack)
                }
                return None;
            }
            // Pick random index from unplayed pool
            let pool_idx = rand::rng().random_range(0..self.unplayed_pool.len());
            let track_idx = self.unplayed_pool.swap_remove(pool_idx);
            return Some(track_idx);
        }

        // Linear mode
        let current = self.current?;
        let next = current + 1;
        if next < self.tracks.len() {
            Some(next)
        } else if self.repeat == RepeatMode::All {
            Some(0)
        } else {
            None
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_linear_queue() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.push(PathBuf::from("c.mp3"));
        q.current = Some(0);

        assert_eq!(q.next_track(), Some(1));
        q.current = Some(1);
        assert_eq!(q.next_track(), Some(2));
        q.current = Some(2);
        assert_eq!(q.next_track(), None); // end of queue, repeat off
    }

    #[test]
    fn test_repeat_all_linear() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.repeat = RepeatMode::All;
        q.current = Some(1);
        assert_eq!(q.next_track(), Some(0)); // wraps to head
    }

    #[test]
    fn test_repeat_one() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.repeat = RepeatMode::One;
        q.current = Some(0);
        assert_eq!(q.next_track(), Some(0)); // stays
    }

    #[test]
    fn test_shuffle_exhausts_pool() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.current = Some(0);
        q.toggle_shuffle();
        // pool should have 1 item (index 1)
        assert_eq!(q.unplayed_pool.len(), 1);
        let next = q.next_track().unwrap();
        assert_eq!(next, 1);
        // pool exhausted
        assert!(q.unplayed_pool.is_empty());
        assert_eq!(q.next_track(), None);
    }

    #[test]
    fn test_shuffle_repeat_all_rebuilds_pool() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.current = Some(0);
        q.toggle_shuffle();
        q.repeat = RepeatMode::All;
        // exhaust pool with first call
        let _ = q.next_track();
        // pool should be empty after exhausting
        assert!(q.unplayed_pool.is_empty());
        // next call should rebuild and succeed
        assert!(q.next_track().is_some());
    }

    #[test]
    fn test_remove_current_track() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.push(PathBuf::from("c.mp3"));
        q.current = Some(1);
        q.remove(1);
        // current should move to next (index 1 now holds "c")
        assert_eq!(q.current, Some(1));
        assert_eq!(q.tracks[1], PathBuf::from("c.mp3"));
    }
}
