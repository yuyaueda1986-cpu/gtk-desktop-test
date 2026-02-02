#ifndef APP_H
#define APP_H

#include <gtk/gtk.h>
#include "json_parser.h"

typedef struct {
    GtkApplication *app;
    GtkWidget *main_window;
    GtkWidget *fixed_container;
    gboolean is_fullscreen;
    LayoutConfig *layout;
    char *layout_file;
} DashboardApp;

DashboardApp* dashboard_app_new(void);
void dashboard_app_free(DashboardApp *app);
int dashboard_app_run(DashboardApp *app, int argc, char **argv);

#endif /* APP_H */
