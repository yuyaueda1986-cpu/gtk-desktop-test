#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
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

// Configuration from command-line options
typedef struct {
    int width;
    int height;
    gboolean start_fullscreen;
} Config;

static Config config = {800, 600, FALSE};

// Application state
typedef struct {
    GtkWidget *main_window;
    gboolean is_fullscreen;
} AppState;

// Forward declaration
static void show_properties_dialog(AppState *state);

// Key event handler
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    AppState *state = (AppState *)data;

    if (event->keyval == GDK_KEY_F12) {
        show_properties_dialog(state);
        return TRUE;
    }
    return FALSE;
}

// Properties dialog display
static void show_properties_dialog(AppState *state) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Properties",
        GTK_WINDOW(state->main_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_OK", GTK_RESPONSE_OK,
        "_Cancel", GTK_RESPONSE_CANCEL,
        NULL);

    // Keep dialog on top (topmost)
    gtk_window_set_keep_above(GTK_WINDOW(dialog), TRUE);

    // Radio buttons for mode selection
    GtkWidget *radio_window = gtk_radio_button_new_with_label(NULL, "Window Mode");
    GtkWidget *radio_fullscreen = gtk_radio_button_new_with_label_from_widget(
        GTK_RADIO_BUTTON(radio_window), "Fullscreen Mode");

    // Reflect current state
    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(state->is_fullscreen ? radio_fullscreen : radio_window), TRUE);

    // Add widgets to dialog
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_pack_start(GTK_BOX(content), radio_window, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(content), radio_fullscreen, FALSE, FALSE, 5);

    // Add some padding
    gtk_container_set_border_width(GTK_CONTAINER(content), 10);

    gtk_widget_show_all(dialog);

    // Response handling
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        gboolean want_fullscreen = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(radio_fullscreen));
        if (want_fullscreen && !state->is_fullscreen) {
            gtk_window_fullscreen(GTK_WINDOW(state->main_window));
            state->is_fullscreen = TRUE;
        } else if (!want_fullscreen && state->is_fullscreen) {
            gtk_window_unfullscreen(GTK_WINDOW(state->main_window));
            state->is_fullscreen = FALSE;
        }
    }
    gtk_widget_destroy(dialog);
}

// Callback for delayed fullscreen
static gboolean apply_fullscreen(gpointer user_data) {
    AppState *state = (AppState *)user_data;
    gtk_window_fullscreen(GTK_WINDOW(state->main_window));
    gtk_widget_grab_focus(state->main_window);
    return G_SOURCE_REMOVE;
}

static void print_usage(const char *prog_name) {
    g_print("Usage: %s [OPTIONS]\n\n", prog_name);
    g_print("Options:\n");
    g_print("  --geometry WxH   Set window size (e.g. --geometry 1024x768)\n");
    g_print("  --fullscreen     Start in fullscreen mode\n");
    g_print("  --help           Show this help message\n");
}

static void activate(GtkApplication *app, gpointer user_data) {
    AppState *state = (AppState *)user_data;
    GtkWidget *window;
    GtkWidget *label;

    // Create main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "GTK Desktop Test");
    gtk_window_set_default_size(GTK_WINDOW(window), config.width, config.height);

    state->main_window = window;
    state->is_fullscreen = config.start_fullscreen;

    // Create label content
    label = gtk_label_new("Press F12 to open Properties dialog");
    gtk_container_add(GTK_CONTAINER(window), label);

    // Connect key press event
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), state);

    gtk_widget_show_all(window);

    // Apply fullscreen after window is fully realized (delayed)
    if (config.start_fullscreen) {
        g_idle_add(apply_fullscreen, state);
    }
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;
    AppState state = {NULL, FALSE};

    // Parse and filter command-line arguments
    int new_argc = 1;
    char **new_argv = g_new(char *, argc + 1);
    new_argv[0] = argv[0];

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--geometry") == 0 && i + 1 < argc) {
            // Parse "WxH" format
            if (sscanf(argv[i + 1], "%dx%d", &config.width, &config.height) != 2) {
                g_printerr("Invalid geometry format: %s (use WxH, e.g. 1024x768)\n", argv[i + 1]);
                g_free(new_argv);
                return 1;
            }
            i++;  // Skip the value argument
        } else if (strcmp(argv[i], "--fullscreen") == 0) {
            config.start_fullscreen = TRUE;
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            g_free(new_argv);
            return 0;
        } else {
            // Pass unrecognized options to GTK
            new_argv[new_argc++] = argv[i];
        }
    }
    new_argv[new_argc] = NULL;

    app = gtk_application_new("com.example.gtkdesktoptest", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), &state);
    status = g_application_run(G_APPLICATION(app), new_argc, new_argv);
    g_object_unref(app);
    g_free(new_argv);

    return status;
}
