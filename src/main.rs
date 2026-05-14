use gstreamer as gst;

mod app;
mod library;
mod playback;
mod ui;

fn main() {
    gst::init().expect("Failed to initialize GStreamer");
    let relm_app = relm4::RelmApp::new("com.mmp.Mmp");
    relm_app.run::<app::AppModel>(());
}
