#include <gst/gst.h>
#include "ui.h"

int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);

    GtkApplication* app = gtk_application_new(NULL, G_APPLICATION_HANDLES_OPEN);

    g_signal_connect(app, "activate", G_CALLBACK(app_activate_cb), NULL);
    g_signal_connect(app, "open", G_CALLBACK(app_open_cb), NULL);

    return g_application_run(G_APPLICATION(app), argc, argv);
}
