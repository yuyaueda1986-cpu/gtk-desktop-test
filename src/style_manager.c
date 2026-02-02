#include "style_manager.h"
#include <string.h>

StyleManager* style_manager_new(void) {
    StyleManager *manager = g_new0(StyleManager, 1);
    manager->provider = gtk_css_provider_new();
    manager->css_buffer = g_string_new("");
    return manager;
}

void style_manager_free(StyleManager *manager) {
    if (!manager) return;

    if (manager->provider) {
        g_object_unref(manager->provider);
    }
    if (manager->css_buffer) {
        g_string_free(manager->css_buffer, TRUE);
    }
    g_free(manager);
}

void style_manager_add_window_style(StyleManager *manager, const char *background_color) {
    if (!manager || !background_color) return;

    g_string_append_printf(manager->css_buffer,
        "window {\n"
        "  background-color: %s;\n"
        "}\n\n",
        background_color);
}

static void append_style_property(GString *css, const char *json_key, const char *value) {
    if (strcmp(json_key, "background_color") == 0) {
        g_string_append_printf(css, "  background: %s;\n", value);
    } else if (strcmp(json_key, "color") == 0) {
        g_string_append_printf(css, "  color: %s;\n", value);
    } else if (strcmp(json_key, "font_size") == 0) {
        /* Auto-append px if value is purely numeric */
        char *end;
        strtod(value, &end);
        if (*end == '\0') {
            g_string_append_printf(css, "  font-size: %spx;\n", value);
        } else {
            g_string_append_printf(css, "  font-size: %s;\n", value);
        }
    } else if (strcmp(json_key, "font_weight") == 0) {
        g_string_append_printf(css, "  font-weight: %s;\n", value);
    } else if (strcmp(json_key, "border_radius") == 0) {
        g_string_append_printf(css, "  border-radius: %s;\n", value);
    } else if (strcmp(json_key, "border_color") == 0) {
        g_string_append_printf(css, "  border-color: %s;\n", value);
    } else if (strcmp(json_key, "border_width") == 0) {
        g_string_append_printf(css, "  border-width: %s;\n", value);
    } else if (strcmp(json_key, "padding") == 0) {
        g_string_append_printf(css, "  padding: %s;\n", value);
    } else if (strcmp(json_key, "margin") == 0) {
        g_string_append_printf(css, "  margin: %s;\n", value);
    }
}

void style_manager_add_widget_style(StyleManager *manager, const char *widget_id, JsonObject *style) {
    if (!manager || !widget_id || !style) return;

    g_string_append_printf(manager->css_buffer, "#%s {\n", widget_id);

    GList *members = json_object_get_members(style);
    for (GList *l = members; l != NULL; l = l->next) {
        const char *key = (const char *)l->data;
        JsonNode *node = json_object_get_member(style, key);

        if (JSON_NODE_HOLDS_VALUE(node)) {
            const char *value = json_node_get_string(node);
            if (value) {
                append_style_property(manager->css_buffer, key, value);
            }
        }
    }
    g_list_free(members);

    g_string_append(manager->css_buffer, "}\n\n");
}

void style_manager_apply(StyleManager *manager) {
    if (!manager) return;

    gtk_css_provider_load_from_string(manager->provider, manager->css_buffer->str);

    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(manager->provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}
