#ifndef SHAPE_RENDERER_H
#define SHAPE_RENDERER_H

#include <gtk/gtk.h>
#include "json_parser.h"

gboolean is_shape_type(const char *type);
GtkWidget* shape_renderer_create(const WidgetConfig *config);

#endif /* SHAPE_RENDERER_H */
