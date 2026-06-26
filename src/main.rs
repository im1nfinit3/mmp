mod app;
mod core;
mod library;
mod playback;
mod settings;
mod style;
mod system_accent;
mod ui;

fn main() {
    app::run().expect("failed to launch mmp");
}
