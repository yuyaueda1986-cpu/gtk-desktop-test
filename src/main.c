#include "app.h"

int main(int argc, char **argv) {
    DashboardApp *app = dashboard_app_new();
    int status = dashboard_app_run(app, argc, argv);
    dashboard_app_free(app);
    return status;
}
