#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <glib.h>
#include <json-glib/json-glib.h>

typedef struct {
    char *title;
    int width;
    int height;
    char *background_color;
} WindowConfig;

typedef struct {
    char *id;
    char *type;
    int x, y, width, height;
    JsonObject *style;
    JsonObject *props;
    JsonObject *events;
} WidgetConfig;

typedef struct {
    WindowConfig window;
    GList *widgets;  /* List of WidgetConfig* */
} LayoutConfig;

LayoutConfig* layout_config_load_from_file(const char *filename, GError **error);
void layout_config_free(LayoutConfig *config);
void widget_config_free(WidgetConfig *config);

#endif /* JSON_PARSER_H */
