use iced::Color;

const FALLBACK_ACCENT: Color = Color::from_rgb(0.27, 0.52, 0.95);
const BASE_ACTIVE_SURFACE: Color = Color::from_rgb(0.17, 0.17, 0.18);
const BASE_CONTROL_BG: Color = Color::from_rgb(0.16, 0.16, 0.18);
const BASE_CONTROL_BG_HOVER: Color = Color::from_rgb(0.20, 0.20, 0.22);
const BASE_CONTROL_BG_PRESSED: Color = Color::from_rgb(0.24, 0.24, 0.26);

#[derive(Debug, Clone, Copy)]
pub struct UiPalette {
    pub accent: Color,
    pub accent_soft: Color,
    pub accent_border: Color,
    pub accent_toggle_bg: Color,
    pub accent_toggle_bg_hover: Color,
    pub accent_toggle_bg_pressed: Color,
    pub focused_selection: Color,
}

pub fn fallback_accent() -> Color {
    FALLBACK_ACCENT
}

pub fn build_palette(accent: Color) -> UiPalette {
    let accent = normalize_color(accent).unwrap_or(FALLBACK_ACCENT);

    UiPalette {
        accent,
        accent_soft: mix(BASE_ACTIVE_SURFACE, accent, 0.22),
        accent_border: Color { a: 0.45, ..accent },
        accent_toggle_bg: mix(BASE_CONTROL_BG, accent, 0.28),
        accent_toggle_bg_hover: mix(BASE_CONTROL_BG_HOVER, accent, 0.34),
        accent_toggle_bg_pressed: mix(BASE_CONTROL_BG_PRESSED, accent, 0.30),
        focused_selection: accent,
    }
}

fn normalize_color(color: Color) -> Option<Color> {
    [color.r, color.g, color.b]
        .into_iter()
        .all(f32::is_finite)
        .then(|| Color {
            r: color.r.clamp(0.0, 1.0),
            g: color.g.clamp(0.0, 1.0),
            b: color.b.clamp(0.0, 1.0),
            a: 1.0,
        })
}

fn mix(base: Color, tint: Color, amount: f32) -> Color {
    let amount = amount.clamp(0.0, 1.0);
    Color {
        r: base.r + ((tint.r - base.r) * amount),
        g: base.g + ((tint.g - base.g) * amount),
        b: base.b + ((tint.b - base.b) * amount),
        a: 1.0,
    }
}

pub fn load_startup_palette() -> UiPalette {
    build_palette(load_startup_accent().unwrap_or_else(fallback_accent))
}

pub fn load_startup_accent() -> Option<Color> {
    startup_accent()
}

#[cfg(target_os = "linux")]
fn startup_accent() -> Option<Color> {
    let runtime = tokio::runtime::Builder::new_current_thread()
        .enable_all()
        .build()
        .ok()?;

    runtime.block_on(async {
        tokio::time::timeout(std::time::Duration::from_millis(200), load_system_accent())
            .await
            .ok()
            .flatten()
    })
}

#[cfg(any(target_os = "macos", target_os = "windows"))]
fn startup_accent() -> Option<Color> {
    platform_accent_sync()
}

#[cfg(not(any(target_os = "linux", target_os = "macos", target_os = "windows")))]
fn startup_accent() -> Option<Color> {
    None
}

#[cfg(target_os = "linux")]
pub async fn load_system_accent() -> Option<Color> {
    use ashpd::desktop::settings::Settings;

    let settings = Settings::new().await.ok()?;
    let color = settings.accent_color().await.ok()?;

    normalize_color(Color::from_rgb(
        color.red() as f32,
        color.green() as f32,
        color.blue() as f32,
    ))
}

#[cfg(target_os = "macos")]
fn platform_accent_sync() -> Option<Color> {
    use objc2_app_kit::{NSColor, NSColorSpace};

    let accent = NSColor::controlAccentColor();
    let color = accent.colorUsingColorSpace(&NSColorSpace::sRGBColorSpace())?;

    normalize_color(Color::from_rgb(
        color.redComponent() as f32,
        color.greenComponent() as f32,
        color.blueComponent() as f32,
    ))
}

#[cfg(target_os = "windows")]
fn platform_accent_sync() -> Option<Color> {
    use windows::UI::ViewManagement::{UIColorType, UISettings};

    let settings = UISettings::new().ok()?;
    let color = settings.GetColorValue(UIColorType::Accent).ok()?;

    normalize_color(Color::from_rgb(
        f32::from(color.R) / 255.0,
        f32::from(color.G) / 255.0,
        f32::from(color.B) / 255.0,
    ))
}

#[cfg(test)]
mod tests {
    use super::{UiPalette, build_palette, fallback_accent, mix, normalize_color};
    use iced::Color;

    fn approx_eq(left: Color, right: Color) {
        assert!(
            (left.r - right.r).abs() < 0.0001,
            "r: {left:?} != {right:?}"
        );
        assert!(
            (left.g - right.g).abs() < 0.0001,
            "g: {left:?} != {right:?}"
        );
        assert!(
            (left.b - right.b).abs() < 0.0001,
            "b: {left:?} != {right:?}"
        );
        assert!(
            (left.a - right.a).abs() < 0.0001,
            "a: {left:?} != {right:?}"
        );
    }

    #[test]
    fn fallback_accent_matches_current_blue() {
        approx_eq(fallback_accent(), Color::from_rgb(0.27, 0.52, 0.95));
    }

    #[test]
    fn mix_interpolates_colors() {
        approx_eq(
            mix(
                Color::from_rgb(0.0, 0.0, 0.0),
                Color::from_rgb(1.0, 0.5, 0.25),
                0.5,
            ),
            Color::from_rgb(0.5, 0.25, 0.125),
        );
    }

    #[test]
    fn normalize_color_clamps_channels_and_forces_opaque() {
        approx_eq(
            normalize_color(Color {
                r: -0.3,
                g: 0.5,
                b: 1.6,
                a: 0.2,
            })
            .expect("normalized color"),
            Color::from_rgb(0.0, 0.5, 1.0),
        );
    }

    #[test]
    fn normalize_color_rejects_non_finite_values() {
        assert!(
            normalize_color(Color {
                r: f32::NAN,
                g: 0.5,
                b: 0.5,
                a: 1.0,
            })
            .is_none()
        );
        assert!(
            normalize_color(Color {
                r: 0.5,
                g: f32::INFINITY,
                b: 0.5,
                a: 1.0,
            })
            .is_none()
        );
    }

    #[test]
    fn build_palette_preserves_accent_and_derives_variants() {
        let palette: UiPalette = build_palette(Color::from_rgb(0.8, 0.2, 0.5));

        approx_eq(palette.accent, Color::from_rgb(0.8, 0.2, 0.5));
        approx_eq(
            palette.accent_border,
            Color {
                a: 0.45,
                ..Color::from_rgb(0.8, 0.2, 0.5)
            },
        );
        assert!(palette.accent_soft.r > 0.17);
        assert!(palette.accent_toggle_bg.g > 0.16);
        approx_eq(palette.focused_selection, palette.accent);
    }
}
