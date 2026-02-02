#include "app.h"
#include "widget_factory.h"
#include "shape_renderer.h"
#include "style_manager.h"
#include <string.h>

/*
 * Known Limitations (WSL2/WSLg):
 *
 * Multi-monitor fullscreen issues:
 * - When running under WSLg (WSL2 GUI subsystem), the --fullscreen option
 *   may not work correctly on non-primary monitors.
 * - Symptoms include: F12 key not responding, dialogs appearing behind
 *   the fullscreen window, or focus issues.
 * - This is a limitation of WSLg's Wayland/X11 bridge, not this application.
 * - Workaround: Use fullscreen on the primary monitor only.
 */

/* Forward declarations */
static void show_properties_dialog(DashboardApp *app);
static void toggle_fullscreen(DashboardApp *app);

/* Dialog data structure */
typedef struct {
    DashboardApp *app;
    GtkWidget *dialog;
    GtkWidget *radio_fullscreen;
} DialogData;

/* OK button clicked handler */
static void on_ok_clicked(GtkButton *button, gpointer user_data) {
    DialogData *data = (DialogData *)user_data;

    gboolean want_fullscreen = gtk_check_button_get_active(
        GTK_CHECK_BUTTON(data->radio_fullscreen));
    if (want_fullscreen && !data->app->is_fullscreen) {
        gtk_window_fullscreen(GTK_WINDOW(data->app->main_window));
        data->app->is_fullscreen = TRUE;
    } else if (!want_fullscreen && data->app->is_fullscreen) {
        gtk_window_unfullscreen(GTK_WINDOW(data->app->main_window));
        data->app->is_fullscreen = FALSE;
    }

    gtk_window_destroy(GTK_WINDOW(data->dialog));
    g_free(data);
}

/* Cancel button clicked handler */
static void on_cancel_clicked(GtkButton *button, gpointer user_data) {
    DialogData *data = (DialogData *)user_data;
    gtk_window_destroy(GTK_WINDOW(data->dialog));
    g_free(data);
}

/* Properties dialog display */
static void show_properties_dialog(DashboardApp *app) {
    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Properties");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->main_window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(main_box, 15);
    gtk_widget_set_margin_end(main_box, 15);
    gtk_widget_set_margin_top(main_box, 15);
    gtk_widget_set_margin_bottom(main_box, 15);

    GtkWidget *radio_window = gtk_check_button_new_with_label("Window Mode");
    GtkWidget *radio_fullscreen = gtk_check_button_new_with_label("Fullscreen Mode");
    gtk_check_button_set_group(GTK_CHECK_BUTTON(radio_fullscreen),
                               GTK_CHECK_BUTTON(radio_window));

    gtk_check_button_set_active(
        GTK_CHECK_BUTTON(app->is_fullscreen ? radio_fullscreen : radio_window), TRUE);

    gtk_box_append(GTK_BOX(main_box), radio_window);
    gtk_box_append(GTK_BOX(main_box), radio_fullscreen);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    gtk_widget_set_margin_top(button_box, 10);

    GtkWidget *cancel_button = gtk_button_new_with_label("Cancel");
    GtkWidget *ok_button = gtk_button_new_with_label("OK");
    gtk_widget_add_css_class(ok_button, "suggested-action");

    gtk_box_append(GTK_BOX(button_box), cancel_button);
    gtk_box_append(GTK_BOX(button_box), ok_button);
    gtk_box_append(GTK_BOX(main_box), button_box);

    gtk_window_set_child(GTK_WINDOW(dialog), main_box);

    DialogData *data = g_new(DialogData, 1);
    data->app = app;
    data->dialog = dialog;
    data->radio_fullscreen = radio_fullscreen;

    g_signal_connect(ok_button, "clicked", G_CALLBACK(on_ok_clicked), data);
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(on_cancel_clicked), data);

    gtk_window_present(GTK_WINDOW(dialog));
}

/* Toggle fullscreen mode */
static void toggle_fullscreen(DashboardApp *app) {
    if (app->is_fullscreen) {
        gtk_window_unfullscreen(GTK_WINDOW(app->main_window));
        app->is_fullscreen = FALSE;
    } else {
        gtk_window_fullscreen(GTK_WINDOW(app->main_window));
        app->is_fullscreen = TRUE;
    }
}

/* Key event handler */
static gboolean on_key_pressed(GtkEventControllerKey *controller,
                               guint keyval,
                               guint keycode,
                               GdkModifierType state_flags,
                               gpointer data) {
    DashboardApp *app = (DashboardApp *)data;

    if (keyval == GDK_KEY_F12) {
        if (state_flags & GDK_CONTROL_MASK) {
            toggle_fullscreen(app);
        } else {
            show_properties_dialog(app);
        }
        return TRUE;
    }
    return FALSE;
}

/* Callback for delayed fullscreen */
static gboolean apply_fullscreen(gpointer user_data) {
    DashboardApp *app = (DashboardApp *)user_data;
    gtk_window_fullscreen(GTK_WINDOW(app->main_window));
    gtk_widget_grab_focus(app->main_window);
    return G_SOURCE_REMOVE;
}

/* Build dashboard from layout config */
static void build_dashboard(DashboardApp *app) {
    if (!app->layout) return;

    StyleManager *style_mgr = style_manager_new();

    /* Apply window background if specified */
    if (app->layout->window.background_color) {
        style_manager_add_window_style(style_mgr, app->layout->window.background_color);
    }

    /* Create GtkFixed container for absolute positioning */
    app->fixed_container = gtk_fixed_new();
    gtk_window_set_child(GTK_WINDOW(app->main_window), app->fixed_container);

    /* Create widgets from config */
    for (GList *l = app->layout->widgets; l != NULL; l = l->next) {
        WidgetConfig *wconfig = (WidgetConfig *)l->data;

        if (is_shape_type(wconfig->type)) {
            /* Shape types: render with Cairo drawing area */
            GtkWidget *shape = shape_renderer_create(wconfig);
            if (shape) {
                gtk_fixed_put(GTK_FIXED(app->fixed_container), shape, wconfig->x, wconfig->y);
            } else {
                g_warning("Failed to create shape '%s' (type: %s)",
                          wconfig->id ? wconfig->id : "(no id)",
                          wconfig->type ? wconfig->type : "(null)");
            }
        } else {
            /* Widget types: create GTK widget */
            GError *error = NULL;
            GtkWidget *widget = widget_factory_create(wconfig, &error);
            if (widget) {
                gtk_fixed_put(GTK_FIXED(app->fixed_container), widget, wconfig->x, wconfig->y);

                /* Add widget-specific CSS (not for shapes) */
                if (wconfig->style && wconfig->id) {
                    style_manager_add_widget_style(style_mgr, wconfig->id, wconfig->style);
                }
            } else {
                g_warning("Failed to create widget '%s': %s",
                          wconfig->id ? wconfig->id : "(no id)",
                          error ? error->message : "unknown error");
                g_clear_error(&error);
            }
        }
    }

    /* Apply all CSS */
    style_manager_apply(style_mgr);
    style_manager_free(style_mgr);
}

/* Activate callback */
static void on_activate(GtkApplication *gtk_app, gpointer user_data) {
    DashboardApp *app = (DashboardApp *)user_data;

    /* Create main window */
    app->main_window = gtk_application_window_new(gtk_app);

    /* Set window title from config or default */
    const char *title = "Dashboard Control Panel";
    if (app->layout && app->layout->window.title) {
        title = app->layout->window.title;
    }
    gtk_window_set_title(GTK_WINDOW(app->main_window), title);

    /* Set window size from config or default */
    int win_width = 1024, win_height = 768;
    if (app->layout) {
        if (app->layout->window.width > 0) win_width = app->layout->window.width;
        if (app->layout->window.height > 0) win_height = app->layout->window.height;
    }
    gtk_window_set_default_size(GTK_WINDOW(app->main_window), win_width, win_height);

    /* Build the dashboard UI */
    build_dashboard(app);

    /* Setup key event controller */
    GtkEventController *key_controller = gtk_event_controller_key_new();
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), app);
    gtk_widget_add_controller(app->main_window, key_controller);

    /* Show window */
    gtk_window_present(GTK_WINDOW(app->main_window));

    /* Apply fullscreen with delay */
    app->is_fullscreen = TRUE;
    g_idle_add(apply_fullscreen, app);
}

DashboardApp* dashboard_app_new(void) {
    DashboardApp *app = g_new0(DashboardApp, 1);
    return app;
}

void dashboard_app_free(DashboardApp *app) {
    if (!app) return;

    if (app->layout) {
        layout_config_free(app->layout);
    }
    g_free(app->layout_file);
    g_free(app);
}

static void print_usage(const char *prog_name) {
    g_print("Usage: %s [OPTIONS] [LAYOUT_FILE]\n\n", prog_name);
    g_print("Options:\n");
    g_print("  --help           Show this help message\n\n");
    g_print("Arguments:\n");
    g_print("  LAYOUT_FILE      JSON file defining the dashboard layout\n\n");
    g_print("Keyboard shortcuts:\n");
    g_print("  F12              Properties dialog\n");
    g_print("  Ctrl+F12         Toggle fullscreen\n");
}

int dashboard_app_run(DashboardApp *app, int argc, char **argv) {
    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (argv[i][0] != '-') {
            /* Assume it's the layout file */
            app->layout_file = g_strdup(argv[i]);
        }
    }

    /* Load layout if specified */
    if (app->layout_file) {
        GError *error = NULL;
        app->layout = layout_config_load_from_file(app->layout_file, &error);
        if (!app->layout) {
            g_printerr("Error loading layout file '%s': %s\n",
                       app->layout_file, error ? error->message : "unknown error");
            g_clear_error(&error);
            return 1;
        }
        g_print("Loaded layout from: %s\n", app->layout_file);
    } else {
        g_print("No layout file specified. Starting with empty dashboard.\n");
        g_print("Usage: %s <layout.json>\n", argv[0]);
    }

    /* Create GtkApplication */
    app->app = gtk_application_new("com.example.gtkdashboard", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app->app, "activate", G_CALLBACK(on_activate), app);

    /* Run the application */
    int status = g_application_run(G_APPLICATION(app->app), 0, NULL);

    g_object_unref(app->app);
    return status;
}
