mod app;
mod app_core;
mod library;
mod playback;
mod system_accent;

fn main() {
    app::run().expect("failed to launch mmp");
}
