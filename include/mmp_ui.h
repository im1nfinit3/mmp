#ifndef MMP_UI_H
#define MMP_UI_H

#include <gtk/gtk.h>

typedef struct _MmpLibrary   MmpLibrary;
typedef struct _MmpPlayback  MmpPlayback;
typedef struct _MmpUI        MmpUI;

MmpUI       *mmp_ui_new(GtkApplication *app, MmpLibrary *lib, MmpPlayback *pb);
void         mmp_ui_connect_library(MmpUI *ui, MmpLibrary *lib);
GtkWindow   *mmp_ui_get_window(MmpUI *ui);
void         mmp_ui_present_window(MmpUI *ui);
MmpLibrary   *mmp_ui_get_library(MmpUI *ui);

#endif
