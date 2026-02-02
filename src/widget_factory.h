#ifndef WIDGET_FACTORY_H
#define WIDGET_FACTORY_H

#include <gtk/gtk.h>
#include "json_parser.h"

GtkWidget* widget_factory_create(const WidgetConfig *config, GError **error);

#endif /* WIDGET_FACTORY_H */
