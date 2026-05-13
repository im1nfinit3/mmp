#include <gst/gst.h>
#include "mmp_playback.h"
#include "mmp_library.h"
#include "mmp_ui.h"

static MmpUI *g_ui = NULL;

static void on_activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;
    if (g_ui) {
        mmp_ui_present_window(g_ui);
        return;
    }

    MmpPlayback *pb  = mmp_playback_new();
    MmpLibrary  *lib = mmp_library_new(pb);
    g_ui = mmp_ui_new(app, lib, pb);
    mmp_ui_connect_library(g_ui, lib);

    mmp_library_load_cached(lib);
    const char *music_dir = g_get_user_special_dir(G_USER_DIRECTORY_MUSIC);
    mmp_library_scan_async(lib, music_dir);
    g_free((gpointer)music_dir);
}

static void on_open(GtkApplication *app, GFile **files, int n_files,
                    const char *hint, gpointer user_data)
{
    (void)hint; (void)user_data;
    on_activate(app, user_data);
    if (g_ui)
        mmp_library_open_files(mmp_ui_get_library(g_ui), files, n_files);
}

int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);
    GtkApplication *app = gtk_application_new(NULL, G_APPLICATION_HANDLES_OPEN);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    g_signal_connect(app, "open",     G_CALLBACK(on_open),     NULL);
    return g_application_run(G_APPLICATION(app), argc, argv);
}
