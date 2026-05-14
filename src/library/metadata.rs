use std::collections::BTreeSet;

use gstreamer as gst;
use gstreamer_pbutils::prelude::*;
use gstreamer_pbutils::{DiscovererInfo, DiscovererStreamInfo};

use super::song::Song;

#[derive(Default)]
struct TagFields {
    title: Option<String>,
    artist: Option<String>,
    album: Option<String>,
}

impl TagFields {
    fn apply_tag_list(&mut self, tags: &gst::TagList) {
        if self.title.is_none()
            && let Some(title) = first_tag_value::<gst::tags::Title>(tags)
        {
            self.title = Some(title);
        }

        if self.artist.is_none() 
            && let Some(artist) = first_tag_value::<gst::tags::Artist>(tags)
        {
            self.artist = Some(artist);
        }

        if self.album.is_none()
            && let Some(album) = first_tag_value::<gst::tags::Album>(tags)
        {
            self.album = Some(album);
        }
    }

    fn apply_stream_info(&mut self, stream: &impl IsA<DiscovererStreamInfo>) {
        if let Some(tags) = stream.tags() {
            self.apply_tag_list(&tags);
        }
    }

    fn apply_to_song(self, song: &mut Song) {
        if let Some(title) = self.title {
            song.title = title;
        }
        if let Some(artist) = self.artist {
            song.artist = artist;
        }
        if let Some(album) = self.album {
            song.album = album;
        }
    }

    fn has_missing_fields(&self) -> bool {
        self.title.is_none() || self.artist.is_none() || self.album.is_none()
    }
}

pub fn apply_discoverer_tags(song: &mut Song, info: &DiscovererInfo) {
    let mut fields = TagFields::default();

    if let Some(stream) = info.stream_info() {
        fields.apply_stream_info(&stream);
    }

    for stream in info.stream_list() {
        fields.apply_stream_info(&stream);
    }

    for audio in info.audio_streams() {
        fields.apply_stream_info(&audio);
    }

    for container in info.container_streams() {
        fields.apply_stream_info(&container);
        if let Some(tags) = container.tags() {
            fields.apply_tag_list(&tags);
        }
    }

    if fields.has_missing_fields() {
        // DiscovererInfo::tags() is deprecated since GStreamer 1.20, but it
        // still provides merged tags for files where stream/container tags only
        // expose container-private data such as QuickTime atoms.
        #[allow(deprecated)]
        if let Some(tags) = info.tags() {
            fields.apply_tag_list(&tags);
        }
    }

    fields.apply_to_song(song);

    if std::env::var_os("MMP_DEBUG_METADATA").is_some() && !song.has_complete_metadata() {
        eprintln!(
            "Incomplete metadata for {}: title={:?}, artist={:?}, album={:?}, duration={:?}, tags={:?}",
            song.path.display(),
            song.title,
            song.artist,
            song.album,
            song.duration_str,
            collect_tag_names(info)
        );
    }
}

fn first_tag_value<'a, T>(tags: &'a gst::TagList) -> Option<String>
where
    T: gst::tags::Tag<'a, TagType = &'a str>,
{
    tags.get::<T>()
        .map(|value| value.get().trim().to_string())
        .filter(|value| !value.is_empty())
}

fn collect_tag_names(info: &DiscovererInfo) -> Vec<String> {
    let mut names = BTreeSet::new();

    if let Some(stream) = info.stream_info() {
        collect_stream_tag_names(&mut names, &stream);
    }

    for stream in info.stream_list() {
        collect_stream_tag_names(&mut names, &stream);
    }

    for audio in info.audio_streams() {
        collect_stream_tag_names(&mut names, &audio);
    }

    for container in info.container_streams() {
        collect_stream_tag_names(&mut names, &container);
        if let Some(tags) = container.tags() {
            collect_tag_list_names(&mut names, &tags);
        }
    }

    names.into_iter().collect()
}

fn collect_stream_tag_names(names: &mut BTreeSet<String>, stream: &impl IsA<DiscovererStreamInfo>) {
    if let Some(tags) = stream.tags() {
        collect_tag_list_names(names, &tags);
    }
}

fn collect_tag_list_names(names: &mut BTreeSet<String>, tags: &gst::TagList) {
    for idx in 0..tags.n_tags() {
        if let Some(name) = tags.nth_tag_name(idx) {
            names.insert(name.to_string());
        }
    }
}
