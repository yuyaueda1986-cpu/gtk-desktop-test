#ifndef STYLE_MANAGER_H
#define STYLE_MANAGER_H

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

typedef struct {
    GtkCssProvider *provider;
    GString *css_buffer;
} StyleManager;

StyleManager* style_manager_new(void);
void style_manager_free(StyleManager *manager);
void style_manager_add_widget_style(StyleManager *manager, const char *widget_id, JsonObject *style);
void style_manager_add_window_style(StyleManager *manager, const char *background_color);
void style_manager_apply(StyleManager *manager);

#endif /* STYLE_MANAGER_H */
