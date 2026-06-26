//! Reusable widget-composing helpers used throughout the UI.

use iced::widget::{button, text};
use iced::Length;

use crate::app::Message;
use crate::style::{
    control_button_style, icon_svg, menu_item_button_style, menu_item_button_variant_style,
    toggle_icon_button_style,
};
use crate::system_accent::UiPalette;

/// A circle button with a centred SVG icon.
pub fn icon_button<'a>(icon: &'static [u8], message: Message) -> iced::widget::Button<'a, Message> {
    button(icon_svg(icon))
        .padding(10)
        .width(44)
        .height(44)
        .style(control_button_style)
        .on_press(message)
}

/// A toggle-able circle button that highlights when active.
pub fn toggle_icon_button<'a>(
    icon: &'static [u8],
    active: bool,
    message: Message,
    palette: &'a UiPalette,
) -> iced::widget::Button<'a, Message> {
    let palette = *palette;
    button(icon_svg(icon))
        .padding(10)
        .width(44)
        .height(44)
        .style(move |theme, status| toggle_icon_button_style(theme, status, active, &palette))
        .on_press(message)
}

/// A label-only menu row.
pub fn menu_item_button<'a>(
    label: &'a str,
    message: Message,
) -> iced::widget::Button<'a, Message> {
    button(text(label).size(14))
        .width(Length::Fill)
        .padding([10, 12])
        .style(menu_item_button_style)
        .on_press(message)
}

/// A menu row that may be disabled (no message → greyed out, non-interactive).
pub fn menu_item_button_with_state(
    label: String,
    message: Option<Message>,
) -> iced::Element<'static, Message> {
    let btn = button(text(label).size(14))
        .width(Length::Fill)
        .padding([10, 12])
        .style(menu_item_button_style);

    match message {
        Some(msg) => btn.on_press(msg).into(),
        None => btn.into(),
    }
}

/// A menu row with an alternative visual style.
pub fn menu_item_button_variant<'a>(
    label: &'a str,
    message: Message,
    alternate: bool,
) -> iced::widget::Button<'a, Message> {
    button(text(label).size(14))
        .width(Length::Fill)
        .padding([10, 12])
        .style(move |theme, status| menu_item_button_variant_style(theme, status, alternate))
        .on_press(message)
}
