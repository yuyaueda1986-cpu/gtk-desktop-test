#include "json_parser.h"
#include <string.h>

static char* get_string_member_or_null(JsonObject *obj, const char *member) {
    if (!json_object_has_member(obj, member)) return NULL;

    JsonNode *node = json_object_get_member(obj, member);
    if (!JSON_NODE_HOLDS_VALUE(node)) return NULL;

    const char *str = json_node_get_string(node);
    return str ? g_strdup(str) : NULL;
}

static int get_int_member_or_default(JsonObject *obj, const char *member, int default_val) {
    if (!json_object_has_member(obj, member)) return default_val;

    JsonNode *node = json_object_get_member(obj, member);
    if (!JSON_NODE_HOLDS_VALUE(node)) return default_val;

    return (int)json_node_get_int(node);
}

static WidgetConfig* parse_widget(JsonObject *widget_obj) {
    WidgetConfig *config = g_new0(WidgetConfig, 1);

    config->id = get_string_member_or_null(widget_obj, "id");
    config->type = get_string_member_or_null(widget_obj, "type");

    if (json_object_has_member(widget_obj, "geometry")) {
        JsonObject *geom = json_object_get_object_member(widget_obj, "geometry");
        config->x = get_int_member_or_default(geom, "x", 0);
        config->y = get_int_member_or_default(geom, "y", 0);
        config->width = get_int_member_or_default(geom, "width", 100);
        config->height = get_int_member_or_default(geom, "height", 30);
    }

    if (json_object_has_member(widget_obj, "style")) {
        config->style = json_object_ref(json_object_get_object_member(widget_obj, "style"));
    }

    if (json_object_has_member(widget_obj, "props")) {
        config->props = json_object_ref(json_object_get_object_member(widget_obj, "props"));
    }

    if (json_object_has_member(widget_obj, "events")) {
        config->events = json_object_ref(json_object_get_object_member(widget_obj, "events"));
    }

    return config;
}

void widget_config_free(WidgetConfig *config) {
    if (!config) return;

    g_free(config->id);
    g_free(config->type);
    if (config->style) json_object_unref(config->style);
    if (config->props) json_object_unref(config->props);
    if (config->events) json_object_unref(config->events);
    g_free(config);
}

LayoutConfig* layout_config_load_from_file(const char *filename, GError **error) {
    JsonParser *parser = json_parser_new();

    if (!json_parser_load_from_file(parser, filename, error)) {
        g_object_unref(parser);
        return NULL;
    }

    JsonNode *root = json_parser_get_root(parser);
    if (!JSON_NODE_HOLDS_OBJECT(root)) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
                    "Root element must be an object");
        g_object_unref(parser);
        return NULL;
    }

    JsonObject *root_obj = json_node_get_object(root);
    LayoutConfig *config = g_new0(LayoutConfig, 1);

    /* Parse window config */
    if (json_object_has_member(root_obj, "window")) {
        JsonObject *window_obj = json_object_get_object_member(root_obj, "window");
        config->window.title = get_string_member_or_null(window_obj, "title");
        config->window.width = get_int_member_or_default(window_obj, "width", 1920);
        config->window.height = get_int_member_or_default(window_obj, "height", 1080);
        config->window.background_color = get_string_member_or_null(window_obj, "background_color");
    }

    /* Parse widgets array */
    if (json_object_has_member(root_obj, "widgets")) {
        JsonArray *widgets_array = json_object_get_array_member(root_obj, "widgets");
        guint n_widgets = json_array_get_length(widgets_array);

        for (guint i = 0; i < n_widgets; i++) {
            JsonObject *widget_obj = json_array_get_object_element(widgets_array, i);
            WidgetConfig *widget = parse_widget(widget_obj);
            config->widgets = g_list_append(config->widgets, widget);
        }
    }

    g_object_unref(parser);
    return config;
}

void layout_config_free(LayoutConfig *config) {
    if (!config) return;

    g_free(config->window.title);
    g_free(config->window.background_color);

    g_list_free_full(config->widgets, (GDestroyNotify)widget_config_free);
    g_free(config);
}
