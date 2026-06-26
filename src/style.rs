//! Theme, colours, and widget style functions for the mmp GUI.

use std::sync::{LazyLock, Mutex};

use iced::widget::svg;
use iced::{Background, Border, Color, Shadow, Theme};

use crate::system_accent::UiPalette;

// ---------------------------------------------------------------------------
// Icon SVGs (embedded as byte slices)
// ---------------------------------------------------------------------------

pub const ICON_PREVIOUS: &[u8] = include_bytes!("icons/previous.svg");
pub const ICON_PLAY: &[u8] = include_bytes!("icons/play.svg");
pub const ICON_PAUSE: &[u8] = include_bytes!("icons/pause.svg");
pub const ICON_NEXT: &[u8] = include_bytes!("icons/next.svg");
pub const ICON_VOLUME_HIGH: &[u8] = include_bytes!("icons/volume-high.svg");
pub const ICON_VOLUME_MUTE: &[u8] = include_bytes!("icons/volume-mute.svg");
pub const ICON_SHUFFLE_OFF: &[u8] = include_bytes!("icons/shuffle-off.svg");
pub const ICON_SHUFFLE_ON: &[u8] = include_bytes!("icons/shuffle.svg");
pub const ICON_REPEAT_ALL: &[u8] = include_bytes!("icons/repeat-all.svg");
pub const ICON_REPEAT_ONE: &[u8] = include_bytes!("icons/repeat-one.svg");

pub static ICON_CACHE: LazyLock<Mutex<HashMap<&'static [u8], svg::Handle>>> =
    LazyLock::new(|| Mutex::new(HashMap::new()));

use std::collections::HashMap;

pub fn icon_svg<'a>(icon: &'static [u8]) -> svg::Svg<'a, Theme> {
    let handle = ICON_CACHE
        .lock()
        .expect("icon cache lock")
        .entry(icon)
        .or_insert_with(|| svg::Handle::from_memory(icon))
        .clone();

    svg(handle)
        .width(20)
        .height(20)
        .style(|_theme, _status| svg::Style {
            color: Some(COLOR_TEXT),
        })
}

// ---------------------------------------------------------------------------
// Colour palette
// ---------------------------------------------------------------------------

pub const COLOR_BG: Color = Color::from_rgb(0.10, 0.10, 0.11);
pub const COLOR_PANEL_ALT: Color = Color::from_rgb(0.08, 0.08, 0.09);
pub const COLOR_ROW_ACTIVE: Color = Color::from_rgb(0.17, 0.17, 0.18);
pub const COLOR_BORDER: Color = Color::from_rgb(0.23, 0.23, 0.24);
pub const COLOR_TEXT: Color = Color::from_rgb(0.92, 0.92, 0.93);
pub const COLOR_DIM: Color = Color::from_rgb(0.58, 0.58, 0.60);
pub const COLOR_BACKDROP: Color = Color::from_rgba(0.0, 0.0, 0.0, 0.55);
pub const COLOR_SURFACE: Color = Color::from_rgb(0.12, 0.12, 0.13);
pub const COLOR_SURFACE_SOFT: Color = Color::from_rgb(0.15, 0.15, 0.16);
pub const COLOR_BORDER_SUBTLE: Color = Color::from_rgb(0.18, 0.18, 0.20);
pub const COLOR_STATUS_BAR_BG: Color = Color::from_rgb(0.07, 0.07, 0.08);

pub const WINDOW_WIDTH: f32 = 1240.0;
pub const WINDOW_HEIGHT: f32 = 820.0;
pub const CONTEXT_MENU_WIDTH: f32 = 220.0;
pub const RADIUS_PANEL: f32 = 14.0;
pub const RADIUS_CONTROL: f32 = 10.0;
pub const RADIUS_ROW: f32 = 12.0;
pub const RADIUS_INPUT: f32 = 12.0;

// ---------------------------------------------------------------------------
// Shared panel helper
// ---------------------------------------------------------------------------

fn panel_style(background: Color, radius: f32) -> iced::widget::container::Style {
    iced::widget::container::Style {
        background: Some(Background::Color(background)),
        text_color: Some(COLOR_TEXT),
        border: Border {
            radius: radius.into(),
            width: 1.0,
            color: COLOR_BORDER_SUBTLE,
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

// ---------------------------------------------------------------------------
// Container / panel styles
// ---------------------------------------------------------------------------

pub fn app_shell_style(_theme: &Theme) -> iced::widget::container::Style {
    panel_style(COLOR_BG, 0.0)
}

pub fn header_panel_style(_theme: &Theme) -> iced::widget::container::Style {
    panel_style(COLOR_SURFACE, RADIUS_PANEL)
}

pub fn nav_panel_style(_theme: &Theme) -> iced::widget::container::Style {
    panel_style(COLOR_PANEL_ALT, RADIUS_PANEL)
}

pub fn content_panel_style(_theme: &Theme) -> iced::widget::container::Style {
    panel_style(COLOR_SURFACE, RADIUS_PANEL)
}

pub fn song_list_panel_style(_theme: &Theme) -> iced::widget::container::Style {
    panel_style(COLOR_PANEL_ALT, RADIUS_PANEL - 2.0)
}

pub fn menu_panel_style(_theme: &Theme) -> iced::widget::container::Style {
    panel_style(COLOR_SURFACE_SOFT, RADIUS_ROW)
}

pub fn modal_panel_style(_theme: &Theme) -> iced::widget::container::Style {
    panel_style(COLOR_SURFACE, RADIUS_PANEL)
}

pub fn modal_backdrop_style(_theme: &Theme) -> iced::widget::container::Style {
    iced::widget::container::Style {
        background: Some(Background::Color(COLOR_BACKDROP)),
        ..Default::default()
    }
}

pub fn song_row_style(
    _theme: &Theme,
    is_current: bool,
    palette: &UiPalette,
) -> iced::widget::container::Style {
    iced::widget::container::Style {
        background: Some(Background::Color(if is_current {
            Color::from_rgb(0.18, 0.19, 0.22)
        } else {
            Color::from_rgb(0.10, 0.10, 0.11)
        })),
        text_color: Some(COLOR_TEXT),
        border: Border {
            radius: RADIUS_ROW.into(),
            width: 1.0,
            color: if is_current {
                palette.accent_border
            } else {
                Color::from_rgba(1.0, 1.0, 1.0, 0.04)
            },
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

pub fn status_bar_panel_style(_theme: &Theme) -> iced::widget::container::Style {
    iced::widget::container::Style {
        background: Some(Background::Color(COLOR_STATUS_BAR_BG)),
        text_color: Some(COLOR_TEXT),
        border: Border {
            radius: RADIUS_PANEL.into(),
            width: 1.0,
            color: COLOR_BORDER_SUBTLE,
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

// ---------------------------------------------------------------------------
// Button styles
// ---------------------------------------------------------------------------

pub fn nav_button_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
    selected: bool,
    palette: &UiPalette,
) -> iced::widget::button::Style {
    let base_bg = if selected {
        COLOR_ROW_ACTIVE
    } else {
        Color::TRANSPARENT
    };

    let hover_bg = if selected {
        COLOR_ROW_ACTIVE
    } else {
        palette.accent_soft
    };

    let background = match status {
        iced::widget::button::Status::Hovered => hover_bg,
        iced::widget::button::Status::Pressed => COLOR_BORDER,
        iced::widget::button::Status::Disabled | iced::widget::button::Status::Active => base_bg,
    };

    iced::widget::button::Style {
        background: Some(Background::Color(background)),
        text_color: COLOR_TEXT,
        border: Border {
            radius: RADIUS_ROW.into(),
            width: 0.0,
            color: Color::TRANSPARENT,
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

pub fn control_button_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
) -> iced::widget::button::Style {
    let background = match status {
        iced::widget::button::Status::Hovered => Color::from_rgb(0.20, 0.20, 0.22),
        iced::widget::button::Status::Pressed => Color::from_rgb(0.24, 0.24, 0.26),
        iced::widget::button::Status::Disabled | iced::widget::button::Status::Active => {
            Color::from_rgb(0.16, 0.16, 0.18)
        }
    };

    iced::widget::button::Style {
        background: Some(Background::Color(background)),
        text_color: COLOR_TEXT,
        border: Border {
            radius: RADIUS_CONTROL.into(),
            width: 1.0,
            color: COLOR_BORDER_SUBTLE,
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

pub fn toggle_icon_button_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
    active: bool,
    palette: &UiPalette,
) -> iced::widget::button::Style {
    let background = match (active, status) {
        (true, iced::widget::button::Status::Hovered) => palette.accent_toggle_bg_hover,
        (true, iced::widget::button::Status::Pressed) => palette.accent_toggle_bg_pressed,
        (true, iced::widget::button::Status::Disabled)
        | (true, iced::widget::button::Status::Active) => palette.accent_toggle_bg,
        (false, iced::widget::button::Status::Hovered) => Color::from_rgb(0.20, 0.20, 0.22),
        (false, iced::widget::button::Status::Pressed) => Color::from_rgb(0.24, 0.24, 0.26),
        (false, iced::widget::button::Status::Disabled)
        | (false, iced::widget::button::Status::Active) => Color::from_rgb(0.16, 0.16, 0.18),
    };

    let border_color = if active { palette.accent } else { COLOR_BORDER };

    iced::widget::button::Style {
        background: Some(Background::Color(background)),
        text_color: COLOR_TEXT,
        border: Border {
            radius: RADIUS_CONTROL.into(),
            width: 1.0,
            color: border_color,
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

pub fn ghost_button_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
) -> iced::widget::button::Style {
    let background = match status {
        iced::widget::button::Status::Hovered => Color::from_rgb(0.18, 0.18, 0.19),
        iced::widget::button::Status::Pressed => Color::from_rgb(0.22, 0.22, 0.23),
        iced::widget::button::Status::Disabled | iced::widget::button::Status::Active => {
            Color::TRANSPARENT
        }
    };

    iced::widget::button::Style {
        background: Some(Background::Color(background)),
        text_color: COLOR_TEXT,
        border: Border {
            radius: RADIUS_CONTROL.into(),
            width: 1.0,
            color: COLOR_BORDER_SUBTLE,
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

pub fn plain_button_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
) -> iced::widget::button::Style {
    let text_color = match status {
        iced::widget::button::Status::Disabled => COLOR_DIM,
        _ => COLOR_TEXT,
    };

    iced::widget::button::Style {
        background: Some(Background::Color(Color::TRANSPARENT)),
        text_color,
        border: Border::default(),
        shadow: Shadow::default(),
        ..Default::default()
    }
}

pub fn list_button_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
    palette: &UiPalette,
) -> iced::widget::button::Style {
    let background = match status {
        iced::widget::button::Status::Hovered => palette.accent_soft,
        iced::widget::button::Status::Pressed => COLOR_ROW_ACTIVE,
        iced::widget::button::Status::Disabled | iced::widget::button::Status::Active => {
            Color::TRANSPARENT
        }
    };

    iced::widget::button::Style {
        background: Some(Background::Color(background)),
        text_color: COLOR_TEXT,
        border: Border::default(),
        shadow: Shadow::default(),
        ..Default::default()
    }
}

pub fn menu_item_button_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
) -> iced::widget::button::Style {
    let background = match status {
        iced::widget::button::Status::Hovered => COLOR_ROW_ACTIVE,
        iced::widget::button::Status::Pressed => Color::from_rgb(0.20, 0.20, 0.21),
        iced::widget::button::Status::Disabled | iced::widget::button::Status::Active => {
            Color::TRANSPARENT
        }
    };

    iced::widget::button::Style {
        background: Some(Background::Color(background)),
        text_color: COLOR_TEXT,
        border: Border {
            radius: RADIUS_ROW.into(),
            width: 0.0,
            color: Color::TRANSPARENT,
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

pub fn menu_item_button_variant_style(
    _theme: &Theme,
    status: iced::widget::button::Status,
    alternate: bool,
) -> iced::widget::button::Style {
    if !alternate {
        return menu_item_button_style(_theme, status);
    }

    let background = match status {
        iced::widget::button::Status::Hovered => COLOR_ROW_ACTIVE,
        iced::widget::button::Status::Pressed => Color::from_rgb(0.20, 0.20, 0.21),
        iced::widget::button::Status::Disabled | iced::widget::button::Status::Active => {
            Color::TRANSPARENT
        }
    };

    iced::widget::button::Style {
        background: Some(Background::Color(background)),
        text_color: Color::from_rgb(0.68, 0.68, 0.70),
        border: Border {
            radius: RADIUS_ROW.into(),
            width: 0.0,
            color: Color::TRANSPARENT,
        },
        shadow: Shadow::default(),
        ..Default::default()
    }
}

// ---------------------------------------------------------------------------
// Toggler style
// ---------------------------------------------------------------------------

pub fn toggler_style(
    _theme: &Theme,
    status: iced::widget::toggler::Status,
    palette: &UiPalette,
) -> iced::widget::toggler::Style {
    let is_toggled = match status {
        iced::widget::toggler::Status::Active { is_toggled } => is_toggled,
        iced::widget::toggler::Status::Hovered { is_toggled } => is_toggled,
        iced::widget::toggler::Status::Disabled { is_toggled } => is_toggled,
    };

    let (background, border_color) = if is_toggled {
        (Background::Color(palette.accent), palette.accent_border)
    } else {
        (
            Background::Color(Color::from_rgb(0.40, 0.40, 0.42)),
            Color::from_rgb(0.50, 0.50, 0.52),
        )
    };

    iced::widget::toggler::Style {
        background,
        background_border_width: 1.0,
        background_border_color: border_color,
        foreground: Background::Color(Color::from_rgb(0.92, 0.92, 0.93)),
        foreground_border_width: 1.0,
        foreground_border_color: Color::from_rgb(0.60, 0.60, 0.62),
        text_color: Some(COLOR_TEXT),
        border_radius: None,
        padding_ratio: 0.3,
    }
}

// ---------------------------------------------------------------------------
// Input / pick-list / slider styles
// ---------------------------------------------------------------------------

pub fn search_input_style(
    _theme: &Theme,
    status: iced::widget::text_input::Status,
    palette: &UiPalette,
) -> iced::widget::text_input::Style {
    let border_color = match status {
        iced::widget::text_input::Status::Focused { .. } => palette.accent,
        iced::widget::text_input::Status::Hovered => Color::from_rgb(0.35, 0.35, 0.37),
        iced::widget::text_input::Status::Active | iced::widget::text_input::Status::Disabled => {
            COLOR_BORDER
        }
    };

    iced::widget::text_input::Style {
        background: Background::Color(Color::from_rgb(0.10, 0.10, 0.11)),
        border: Border {
            radius: RADIUS_INPUT.into(),
            width: 1.0,
            color: border_color,
        },
        icon: COLOR_DIM,
        placeholder: COLOR_DIM,
        value: COLOR_TEXT,
        selection: palette.focused_selection,
    }
}

pub fn slider_style(
    _theme: &Theme,
    status: iced::widget::slider::Status,
    palette: &UiPalette,
) -> iced::widget::slider::Style {
    let handle_background = match status {
        iced::widget::slider::Status::Active => palette.accent,
        iced::widget::slider::Status::Hovered => palette.accent_toggle_bg_hover,
        iced::widget::slider::Status::Dragged => palette.accent_toggle_bg_pressed,
    };

    let handle_border = match status {
        iced::widget::slider::Status::Active => palette.accent,
        iced::widget::slider::Status::Hovered => palette.accent,
        iced::widget::slider::Status::Dragged => palette.accent,
    };

    iced::widget::slider::Style {
        rail: iced::widget::slider::Rail {
            backgrounds: (
                Background::Color(palette.accent),
                Background::Color(Color::from_rgb(0.36, 0.36, 0.39)),
            ),
            width: 4.0,
            border: Border {
                radius: 999.0.into(),
                width: 0.0,
                color: Color::TRANSPARENT,
            },
        },
        handle: iced::widget::slider::Handle {
            shape: iced::widget::slider::HandleShape::Circle { radius: 7.0 },
            background: Background::Color(handle_background),
            border_width: 1.0,
            border_color: handle_border,
        },
    }
}

pub fn pick_list_style(
    _theme: &Theme,
    status: iced::widget::pick_list::Status,
    palette: &UiPalette,
) -> iced::widget::pick_list::Style {
    let border_color = match status {
        iced::widget::pick_list::Status::Active => COLOR_BORDER,
        iced::widget::pick_list::Status::Hovered => Color::from_rgb(0.35, 0.35, 0.37),
        iced::widget::pick_list::Status::Opened { .. } => palette.accent,
    };

    iced::widget::pick_list::Style {
        placeholder_color: COLOR_DIM,
        text_color: COLOR_TEXT,
        background: iced::Background::Color(Color::from_rgb(0.10, 0.10, 0.11)),
        border: iced::Border {
            radius: RADIUS_INPUT.into(),
            width: 1.0,
            color: border_color,
        },
        handle_color: palette.accent,
    }
}

pub fn menu_style(_theme: &Theme, palette: &UiPalette) -> iced::widget::overlay::menu::Style {
    iced::widget::overlay::menu::Style {
        background: iced::Background::Color(Color::from_rgb(0.12, 0.12, 0.13)),
        border: iced::Border {
            radius: RADIUS_INPUT.into(),
            width: 1.0,
            color: COLOR_BORDER_SUBTLE,
        },
        text_color: COLOR_TEXT,
        selected_text_color: COLOR_TEXT,
        selected_background: iced::Background::Color(palette.accent_soft),
        shadow: iced::Shadow::default(),
    }
}
