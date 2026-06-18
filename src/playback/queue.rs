//! Queue state for linear + shuffle playback.

use std::path::PathBuf;

use rand::Rng;

use crate::library::song::RepeatMode;

/// Queue state for linear + shuffle playback.
pub struct QueueState {
    /// Ordered list of file paths in the playback queue.
    pub tracks: Vec<PathBuf>,
    /// Index of the currently playing track (None if nothing playing).
    pub current: Option<usize>,
    /// Indices into `tracks` of unplayed songs, in play order (for shuffle mode).
    /// Pre-shuffled once on toggle/rebuild, then consumed from the front.
    pub unplayed_pool: Vec<usize>,
    /// Stack of previously played track indices (most recent = back) for shuffle "previous".
    pub history: Vec<usize>,
    pub shuffle: bool,
    pub repeat: RepeatMode,
}

impl QueueState {
    pub fn new() -> Self {
        Self {
            tracks: Vec::new(),
            current: None,
            unplayed_pool: Vec::new(),
            history: Vec::new(),
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

    /// Remove all tracks from the queue, clearing playback state.
    pub fn clear(&mut self) {
        self.tracks.clear();
        self.current = None;
        self.unplayed_pool.clear();
        self.history.clear();
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

        // Adjust history: remove index, shift others
        self.history.retain(|&i| i != index);
        for idx in &mut self.history {
            if *idx > index {
                *idx -= 1;
            }
        }
    }

    /// Toggle shuffle mode on/off.
    pub fn toggle_shuffle(&mut self) {
        self.shuffle = !self.shuffle;
        if self.shuffle {
            self.history.clear();
            self.rebuild_unplayed_pool();
        } else {
            self.unplayed_pool.clear();
            self.history.clear();
        }
    }

    /// Cycle repeat mode: Off → All → One → Off.
    pub fn cycle_repeat(&mut self) {
        self.repeat = self.repeat.next();
    }

    /// Rebuild the unplayed pool: all queue indices EXCEPT the current one.
    /// When shuffle is on, the pool is randomly shuffled into play order.
    pub fn rebuild_unplayed_pool(&mut self) {
        self.unplayed_pool.clear();
        for i in 0..self.tracks.len() {
            if Some(i) != self.current {
                self.unplayed_pool.push(i);
            }
        }
        if self.shuffle {
            // Fisher-Yates shuffle — produces the ordered queue for shuffle mode
            let mut rng = rand::rng();
            for i in (1..self.unplayed_pool.len()).rev() {
                let j = rng.random_range(0..=i);
                self.unplayed_pool.swap(i, j);
            }
        }
    }

    /// Record that the current track has been played and advance the history.
    /// Should be called *before* `next_track()` when the user actively advances.
    pub fn record_current_played(&mut self) {
        if let Some(cur) = self.current {
            self.history.push(cur);
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
                    self.history.clear();
                    self.rebuild_unplayed_pool();
                    return self.next_track(); // recurse (tail-recursive, won't blow stack)
                }
                return None;
            }
            // Pop from front of pre-shuffled pool
            let track_idx = self.unplayed_pool.remove(0);
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

    /// Go back to the previously played track in shuffle mode.
    /// The current track is placed back into the unplayed pool
    /// (at a random position) since it wasn't finished.
    /// Returns None if there is no history.
    pub fn previous_track(&mut self) -> Option<usize> {
        if !self.shuffle {
            // Linear mode: just go to current - 1
            let current = self.current?;
            if current > 0 {
                return Some(current - 1);
            }
            return None;
        }

        // Shuffle mode: pop from history
        let prev = self.history.pop()?;

        // Put the current track back into the unplayed pool at a random position
        // since we're navigating away before it finished
        if let Some(current) = self.current {
            let pos = rand::rng().random_range(0..=self.unplayed_pool.len());
            self.unplayed_pool.insert(pos, current);
        }

        Some(prev)
    }

    /// Return indices in display order for the queue view.
    /// In linear mode: all tracks in order (0, 1, 2, ...).
    /// In shuffle mode: history (oldest first) + current + unplayed pool (play order).
    pub fn display_indices(&self) -> Vec<usize> {
        if self.shuffle {
            let mut indices: Vec<usize> = self.history.clone(); // oldest first
            if let Some(cur) = self.current {
                indices.push(cur);
            }
            indices.extend(&self.unplayed_pool);
            indices
        } else {
            (0..self.tracks.len()).collect()
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

    #[test]
    fn test_shuffle_history_and_previous() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.push(PathBuf::from("c.mp3"));
        q.push(PathBuf::from("d.mp3"));
        q.current = Some(0);
        q.toggle_shuffle();

        // Simulate advancing through the shuffled queue
        q.record_current_played();
        let next1 = q.next_track().unwrap();
        q.current = Some(next1);
        assert_eq!(q.history.len(), 1);
        assert_eq!(q.history[0], 0);

        q.record_current_played();
        let next2 = q.next_track().unwrap();
        q.current = Some(next2);
        assert_eq!(q.history.len(), 2);

        // Previous should go back to next1
        let prev = q.previous_track().unwrap();
        assert_eq!(prev, next1);
        // Current (next2) should be back in the pool
        assert!(!q.unplayed_pool.is_empty());
        // History should have 1 entry now
        assert_eq!(q.history.len(), 1);

        // Previous again should go back to 0
        q.current = Some(prev);
        let prev2 = q.previous_track().unwrap();
        assert_eq!(prev2, 0);
        assert!(q.history.is_empty());
    }

    #[test]
    fn test_shuffle_previous_at_beginning() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.current = Some(0);
        q.toggle_shuffle();
        // No history yet
        assert!(q.previous_track().is_none());
    }

    #[test]
    fn test_linear_previous() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.current = Some(1);
        // Linear mode: previous goes to index 0
        assert_eq!(q.previous_track(), Some(0));
        q.current = Some(0);
        assert_eq!(q.previous_track(), None); // at beginning
    }

    #[test]
    fn test_display_indices_shuffle() {
        let mut q = QueueState::new();
        q.push(PathBuf::from("a.mp3"));
        q.push(PathBuf::from("b.mp3"));
        q.push(PathBuf::from("c.mp3"));
        q.current = Some(0);
        q.toggle_shuffle();

        // Simulate playing track 0, then moving to the next shuffled track
        q.record_current_played();
        let next = q.next_track().unwrap();
        q.current = Some(next);

        // Display: [0 (history), next (current), ...remaining pool]
        let indices = q.display_indices();
        assert_eq!(indices[0], 0); // history first
        assert_eq!(indices[1], next); // current
        // Pool should follow in pre-shuffled order
        assert_eq!(indices.len(), 3);
    }
}
