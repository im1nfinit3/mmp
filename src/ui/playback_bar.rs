//! Playback bar — transport controls, progress bar, volume, track label.
//!
//! Relm4 sub-component.  Self-derives the current track label from
//! `PlaybackEvent::Tags` (Q15-B).  Emits `PlaybackBarOutput` for all
//! transport and volume actions.

use gtk4::prelude::*;
use relm4::prelude::*;

use crate::playback::{PlaybackEvent, PlaybackState};
use crate::ui::widgets;

// ---------------------------------------------------------------------------
// Model
// ---------------------------------------------------------------------------

pub struct PlaybackBar {
    /// "Title — Artist" label.
    current_track_label: gtk4::Label,
    /// Play / Pause button.
    play_pause_button: gtk4::Button,
    /// Shuffle toggle button.
    shuffle_button: gtk4::Button,
    /// Repeat toggle button.
    repeat_button: gtk4::Button,
    /// Mute toggle button.
    mute_button: gtk4::Button,
    /// Volume slider (0–100).
    volume_scale: gtk4::Scale,
    /// Seek / progress bar (seconds range).
    track_progress_scale: gtk4::Scale,
    /// Elapsed time label.
    elapsed_time_label: gtk4::Label,
    /// Duration label.
    duration_label: gtk4::Label,
    /// Local copy of volume (0.0–1.0).
    volume: f64,
    /// Whether audio is muted.
    muted: bool,
    /// Whether shuffle is active.
    shuffle: bool,
    /// Current repeat mode.
    repeat: crate::library::song::RepeatMode,
}

// ---------------------------------------------------------------------------
// Messages (Input)
// ---------------------------------------------------------------------------

#[derive(Debug, Clone)]
pub enum PlaybackBarMsg {
    /// Forwarded playback event from the parent.
    PlaybackEvent(PlaybackEvent),
    /// User clicked play/pause.
    PlayPauseClicked,
    /// User clicked previous.
    PreviousClicked,
    /// User clicked next.
    NextClicked,
    /// User moved the seek bar.
    Seek(f64),
    /// User changed volume.
    VolumeChanged(f64),
    /// User clicked mute.
    MuteClicked,
    /// User clicked shuffle.
    ShuffleClicked,
    /// User clicked repeat.
    RepeatClicked,
}

// ---------------------------------------------------------------------------
// Output (emitted to parent)
// ---------------------------------------------------------------------------

#[derive(Debug, Clone)]
pub enum PlaybackBarOutput {
    PlayPause,
    Previous,
    Next,
    Seek(f64),
    VolumeChanged(f64),
    MuteToggled,
    ShuffleToggled,
    RepeatToggled,
}

// ---------------------------------------------------------------------------
// Component
// ---------------------------------------------------------------------------

#[relm4::component(pub)]
impl SimpleComponent for PlaybackBar {
    type Init = ();
    type Input = PlaybackBarMsg;
    type Output = PlaybackBarOutput;

    view! {
        #[root]
        gtk4::Box {
            set_css_classes: &["playback-bar"],
            set_spacing: 12,

            // -- Left: transport controls --
            gtk4::Box {
                set_orientation: gtk4::Orientation::Horizontal,
                set_spacing: 4,
                set_valign: gtk4::Align::Center,

                gtk4::Button {
                    set_css_classes: &["playback-button"],
                    set_icon_name: "media-skip-backward-symbolic",
                    set_tooltip_text: Some("Previous track"),
                    set_valign: gtk4::Align::Center,
                    connect_clicked => PlaybackBarMsg::PreviousClicked,
                },
                #[name(play_pause_button)]
                gtk4::Button {
                    set_css_classes: &["playback-button"],
                    set_icon_name: "media-playback-start-symbolic",
                    set_tooltip_text: Some("Play"),
                    set_valign: gtk4::Align::Center,
                    connect_clicked => PlaybackBarMsg::PlayPauseClicked,
                },
                gtk4::Button {
                    set_css_classes: &["playback-button"],
                    set_icon_name: "media-skip-forward-symbolic",
                    set_tooltip_text: Some("Next track"),
                    set_valign: gtk4::Align::Center,
                    connect_clicked => PlaybackBarMsg::NextClicked,
                },
                #[name(repeat_button)]
                gtk4::Button {
                    set_css_classes: &["playback-button"],
                    set_icon_name: "media-playlist-repeat-symbolic",
                    set_tooltip_text: Some("Repeat"),
                    set_valign: gtk4::Align::Center,
                    connect_clicked => PlaybackBarMsg::RepeatClicked,
                },
                #[name(shuffle_button)]
                gtk4::Button {
                    set_css_classes: &["playback-button"],
                    set_icon_name: "media-playlist-shuffle-symbolic",
                    set_tooltip_text: Some("Shuffle"),
                    set_valign: gtk4::Align::Center,
                    connect_clicked => PlaybackBarMsg::ShuffleClicked,
                },
            },

            // -- Center: track info + progress --
            gtk4::Box {
                set_orientation: gtk4::Orientation::Vertical,
                set_hexpand: true,
                set_spacing: 4,
                set_css_classes: &["track-info"],

                #[name(current_track_label)]
                gtk4::Label {
                    set_label: "No track selected",
                    set_halign: gtk4::Align::Start,
                    set_ellipsize: gtk4::pango::EllipsizeMode::End,
                    set_css_classes: &["track-info"],
                },

                gtk4::Box {
                    set_orientation: gtk4::Orientation::Horizontal,
                    set_spacing: 8,

                    #[name(elapsed_time_label)]
                    gtk4::Label {
                        set_css_classes: &["time-label"],
                        set_label: "0:00",
                    },
                    #[name(track_progress_scale)]
                    gtk4::Scale {
                        set_hexpand: true,
                        set_draw_value: false,
                        set_range: (0.0, 1.0),
                        set_increments: (1.0, 10.0),
                    },
                    #[name(duration_label)]
                    gtk4::Label {
                        set_css_classes: &["time-label"],
                        set_label: "0:00",
                    },
                },
            },

            // -- Right: volume --
            gtk4::Box {
                set_orientation: gtk4::Orientation::Horizontal,
                set_spacing: 8,
                set_valign: gtk4::Align::Center,

                #[name(volume_scale)]
                gtk4::Scale {
                    set_css_classes: &["volume-scale"],
                    set_range: (0.0, 100.0),
                    set_draw_value: false,
                    set_value: 70.0,
                },
                #[name(mute_button)]
                gtk4::Button {
                    set_css_classes: &["volume-button"],
                    set_icon_name: "audio-volume-medium-symbolic",
                    set_tooltip_text: Some("Mute"),
                    connect_clicked => PlaybackBarMsg::MuteClicked,
                },
            },
        }
    }

    fn init(
        _init: Self::Init,
        root: Self::Root,
        sender: ComponentSender<Self>,
    ) -> ComponentParts<Self> {
        let widgets = view_output!();

        // Bold current-track label
        let attrs = gtk4::pango::AttrList::new();
        attrs.insert(gtk4::pango::AttrInt::new_weight(gtk4::pango::Weight::Bold));
        widgets.current_track_label.set_attributes(Some(&attrs));

        // Volume scale → Input
        {
            let s = sender.clone();
            widgets
                .volume_scale
                .connect_change_value(move |_, _, value| {
                    s.input(PlaybackBarMsg::VolumeChanged(value / 100.0));
                    gtk4::glib::Propagation::Proceed
                });
        }

        // Track progress scale → Input
        {
            let s = sender.clone();
            widgets
                .track_progress_scale
                .connect_change_value(move |scale, _, value| {
                    let seconds = value * scale.adjustment().upper();
                    s.input(PlaybackBarMsg::Seek(seconds));
                    gtk4::glib::Propagation::Proceed
                });
        }

        // Volume revealer: slider hidden until hover, slides out left
        {
            let revealer = gtk4::Revealer::new();
            revealer.set_transition_type(gtk4::RevealerTransitionType::SlideLeft);
            revealer.set_reveal_child(false);

            // Find the volume controls box (last child of the root)
            let volume_box = root
                .last_child()
                .and_then(|c| c.downcast::<gtk4::Box>().ok())
                .expect("volume controls box");

            volume_box.remove(&widgets.volume_scale);
            revealer.set_child(Some(&widgets.volume_scale));
            volume_box.prepend(&revealer);

            let motion = gtk4::EventControllerMotion::new();
            let r1 = revealer.clone();
            motion.connect_enter(move |_, _x, _y| {
                r1.set_reveal_child(true);
            });
            let r2 = revealer.clone();
            motion.connect_leave(move |_| {
                r2.set_reveal_child(false);
            });
            volume_box.add_controller(motion);
        }

        let model = PlaybackBar {
            current_track_label: widgets.current_track_label.clone(),
            play_pause_button: widgets.play_pause_button.clone(),
            shuffle_button: widgets.shuffle_button.clone(),
            repeat_button: widgets.repeat_button.clone(),
            mute_button: widgets.mute_button.clone(),
            volume_scale: widgets.volume_scale.clone(),
            track_progress_scale: widgets.track_progress_scale.clone(),
            elapsed_time_label: widgets.elapsed_time_label.clone(),
            duration_label: widgets.duration_label.clone(),
            volume: 0.7,
            muted: false,
            shuffle: false,
            repeat: crate::library::song::RepeatMode::Off,
        };

        ComponentParts { model, widgets }
    }

    fn update(&mut self, msg: Self::Input, sender: ComponentSender<Self>) {
        match msg {
            PlaybackBarMsg::PlayPauseClicked => {
                let _ = sender.output(PlaybackBarOutput::PlayPause);
            }
            PlaybackBarMsg::PreviousClicked => {
                let _ = sender.output(PlaybackBarOutput::Previous);
            }
            PlaybackBarMsg::NextClicked => {
                let _ = sender.output(PlaybackBarOutput::Next);
            }
            PlaybackBarMsg::Seek(secs) => {
                let _ = sender.output(PlaybackBarOutput::Seek(secs));
            }
            PlaybackBarMsg::VolumeChanged(vol) => {
                let _ = sender.output(PlaybackBarOutput::VolumeChanged(vol));
            }
            PlaybackBarMsg::MuteClicked => {
                let _ = sender.output(PlaybackBarOutput::MuteToggled);
            }
            PlaybackBarMsg::ShuffleClicked => {
                let _ = sender.output(PlaybackBarOutput::ShuffleToggled);
            }
            PlaybackBarMsg::RepeatClicked => {
                let _ = sender.output(PlaybackBarOutput::RepeatToggled);
            }
            PlaybackBarMsg::PlaybackEvent(event) => match event {
                PlaybackEvent::Tags { title, artist } => {
                    let label = match (title, artist) {
                        (Some(t), Some(a)) if !a.is_empty() && a != "Unknown Artist" => {
                            format!("{} — {}", t, a)
                        }
                        (Some(t), _) => t,
                        (None, Some(a)) => a,
                        (None, None) => "No track selected".into(),
                    };
                    self.current_track_label.set_label(&label);
                }
                PlaybackEvent::Position { elapsed, duration } => {
                    self.track_progress_scale.set_range(0.0, duration);
                    self.track_progress_scale.set_value(elapsed);
                    self.elapsed_time_label
                        .set_label(&widgets::format_time(elapsed));
                    self.duration_label
                        .set_label(&widgets::format_time(duration));
                }
                PlaybackEvent::StateChanged(state) => {
                    let icon = match state {
                        PlaybackState::Playing => "media-playback-pause-symbolic",
                        PlaybackState::Paused | PlaybackState::Stopped => {
                            "media-playback-start-symbolic"
                        }
                    };
                    self.play_pause_button.set_icon_name(icon);
                }
                PlaybackEvent::EndOfStream | PlaybackEvent::Error(_) => {}
            },
        }
    }
}
