mod app;
mod library;
mod playback;
mod ui;

fn main() {
    let relm_app = relm4::RelmApp::new("com.mmp.Mmp");
    relm_app.run::<app::AppModel>(());
}
