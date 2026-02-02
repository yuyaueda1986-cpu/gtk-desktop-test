#include "widget_factory.h"
#include <string.h>
#include <pango/pango.h>

static const char* get_string_prop(JsonObject *props, const char *key, const char *default_val) {
    if (!props || !json_object_has_member(props, key)) return default_val;
    JsonNode *node = json_object_get_member(props, key);
    if (!JSON_NODE_HOLDS_VALUE(node)) return default_val;
    const char *val = json_node_get_string(node);
    return val ? val : default_val;
}

static double get_double_prop(JsonObject *props, const char *key, double default_val) {
    if (!props || !json_object_has_member(props, key)) return default_val;
    JsonNode *node = json_object_get_member(props, key);
    if (!JSON_NODE_HOLDS_VALUE(node)) return default_val;
    return json_node_get_double(node);
}

static int get_int_prop(JsonObject *props, const char *key, int default_val) {
    if (!props || !json_object_has_member(props, key)) return default_val;
    JsonNode *node = json_object_get_member(props, key);
    if (!JSON_NODE_HOLDS_VALUE(node)) return default_val;
    return (int)json_node_get_int(node);
}

static gboolean get_bool_prop(JsonObject *props, const char *key, gboolean default_val) {
    if (!props || !json_object_has_member(props, key)) return default_val;
    JsonNode *node = json_object_get_member(props, key);
    if (!JSON_NODE_HOLDS_VALUE(node)) return default_val;
    return json_node_get_boolean(node);
}

/* Button: props.label, props.icon_name (empty string = no icon) */
static GtkWidget* create_button(const WidgetConfig *config) {
    const char *label = get_string_prop(config->props, "label", "Button");
    const char *icon_name = get_string_prop(config->props, "icon_name", "");

    GtkWidget *button;
    if (icon_name && strlen(icon_name) > 0) {
        button = gtk_button_new_from_icon_name(icon_name);
        if (label && strlen(label) > 0) {
            gtk_button_set_label(GTK_BUTTON(button), label);
        }
    } else {
        button = gtk_button_new_with_label(label);
    }

    return button;
}

/* Label: props.label, props.font_size */
static GtkWidget* create_label(const WidgetConfig *config) {
    const char *text = get_string_prop(config->props, "label", "Label");
    int font_size = get_int_prop(config->props, "font_size", 0);

    GtkWidget *label = gtk_label_new(text);

    /* Apply font_size via Pango attributes if specified */
    if (font_size > 0) {
        PangoAttrList *attrs = pango_attr_list_new();
        PangoAttribute *attr = pango_attr_size_new_absolute(font_size * PANGO_SCALE);
        pango_attr_list_insert(attrs, attr);
        gtk_label_set_attributes(GTK_LABEL(label), attrs);
        pango_attr_list_unref(attrs);
    }

    return label;
}

/* Entry: props.placeholder, props.text */
static GtkWidget* create_entry(const WidgetConfig *config) {
    GtkWidget *entry = gtk_entry_new();

    const char *text = get_string_prop(config->props, "text", NULL);
    if (text && strlen(text) > 0) {
        GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
        gtk_entry_buffer_set_text(buffer, text, -1);
    }

    const char *placeholder = get_string_prop(config->props, "placeholder", NULL);
    if (placeholder) {
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), placeholder);
    }

    return entry;
}

/* Checkbox: props.label, props.checked */
static GtkWidget* create_check_button(const WidgetConfig *config) {
    const char *label = get_string_prop(config->props, "label", "Checkbox");
    gboolean checked = get_bool_prop(config->props, "checked", FALSE);

    GtkWidget *check = gtk_check_button_new_with_label(label);
    gtk_check_button_set_active(GTK_CHECK_BUTTON(check), checked);

    return check;
}

/* Switch: props.label, props.active */
static GtkWidget* create_switch(const WidgetConfig *config) {
    gboolean active = get_bool_prop(config->props, "active", FALSE);
    const char *label_text = get_string_prop(config->props, "label", NULL);

    GtkWidget *sw = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(sw), active);

    /* If label is specified, wrap in a box with label */
    if (label_text && strlen(label_text) > 0) {
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        GtkWidget *label = gtk_label_new(label_text);
        gtk_box_append(GTK_BOX(box), label);
        gtk_box_append(GTK_BOX(box), sw);
        return box;
    }

    return sw;
}

/* Combo: props.items (comma-separated string), props.active_index */
static GtkWidget* create_combo_box_text(const WidgetConfig *config) {
    GtkWidget *combo = gtk_combo_box_text_new();

    if (config->props && json_object_has_member(config->props, "items")) {
        JsonNode *items_node = json_object_get_member(config->props, "items");

        if (JSON_NODE_HOLDS_VALUE(items_node)) {
            /* Spec format: comma-separated string */
            const char *items_str = json_node_get_string(items_node);
            if (items_str) {
                char **parts = g_strsplit(items_str, ",", -1);
                for (int i = 0; parts[i] != NULL; i++) {
                    char *trimmed = g_strstrip(g_strdup(parts[i]));
                    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), trimmed);
                    g_free(trimmed);
                }
                g_strfreev(parts);
            }
        } else if (JSON_NODE_HOLDS_ARRAY(items_node)) {
            /* Fallback: JSON array format */
            JsonArray *items = json_node_get_array(items_node);
            guint n = json_array_get_length(items);
            for (guint i = 0; i < n; i++) {
                const char *item = json_array_get_string_element(items, i);
                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), item);
            }
        }
    }

    int active_index = get_int_prop(config->props, "active_index", 0);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), active_index);

    return combo;
}

/* Slider: props.min, props.max, props.value, props.step */
static GtkWidget* create_scale(const WidgetConfig *config) {
    double min = get_double_prop(config->props, "min", 0.0);
    double max = get_double_prop(config->props, "max", 100.0);
    double step = get_double_prop(config->props, "step", 1.0);
    double value = get_double_prop(config->props, "value", 50.0);

    GtkWidget *scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, min, max, step);
    gtk_range_set_value(GTK_RANGE(scale), value);
    gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);

    return scale;
}

/* Spin: props.min, props.max, props.value, props.step */
static GtkWidget* create_spin_button(const WidgetConfig *config) {
    double min = get_double_prop(config->props, "min", 0.0);
    double max = get_double_prop(config->props, "max", 100.0);
    double step = get_double_prop(config->props, "step", 1.0);
    double value = get_double_prop(config->props, "value", 0.0);

    GtkWidget *spin = gtk_spin_button_new_with_range(min, max, step);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), value);

    return spin;
}

/* Image: props.file_path, props.alt_text */
static GtkWidget* create_image(const WidgetConfig *config) {
    const char *file_path = get_string_prop(config->props, "file_path", "");
    const char *alt_text = get_string_prop(config->props, "alt_text", "Image");

    if (file_path && strlen(file_path) > 0) {
        GtkWidget *image = gtk_image_new_from_file(file_path);
        return image;
    }

    /* Fallback: show alt_text as a label */
    GtkWidget *label = gtk_label_new(alt_text);
    return label;
}

/* Progress: props.value (0.0-1.0), props.show_text */
static GtkWidget* create_progress_bar(const WidgetConfig *config) {
    double value = get_double_prop(config->props, "value", 0.0);
    gboolean show_text = get_bool_prop(config->props, "show_text", FALSE);

    GtkWidget *progress = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), value);

    if (show_text) {
        char *text = g_strdup_printf("%.0f%%", value * 100.0);
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), text);
        gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress), TRUE);
        g_free(text);
    }

    return progress;
}

/* Separator: props.orientation */
static GtkWidget* create_separator(const WidgetConfig *config) {
    const char *orient = get_string_prop(config->props, "orientation", "horizontal");
    GtkOrientation orientation = (strcmp(orient, "vertical") == 0)
        ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;

    return gtk_separator_new(orientation);
}

GtkWidget* widget_factory_create(const WidgetConfig *config, GError **error) {
    if (!config || !config->type) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,
                    "Widget config or type is NULL");
        return NULL;
    }

    GtkWidget *widget = NULL;

    if (strcmp(config->type, "Button") == 0) {
        widget = create_button(config);
    } else if (strcmp(config->type, "Label") == 0) {
        widget = create_label(config);
    } else if (strcmp(config->type, "Entry") == 0) {
        widget = create_entry(config);
    } else if (strcmp(config->type, "Checkbox") == 0) {
        widget = create_check_button(config);
    } else if (strcmp(config->type, "Switch") == 0) {
        widget = create_switch(config);
    } else if (strcmp(config->type, "Combo") == 0) {
        widget = create_combo_box_text(config);
    } else if (strcmp(config->type, "Slider") == 0) {
        widget = create_scale(config);
    } else if (strcmp(config->type, "Spin") == 0) {
        widget = create_spin_button(config);
    } else if (strcmp(config->type, "Image") == 0) {
        widget = create_image(config);
    } else if (strcmp(config->type, "Progress") == 0) {
        widget = create_progress_bar(config);
    } else if (strcmp(config->type, "Separator") == 0) {
        widget = create_separator(config);
    } else {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                    "Unknown widget type: %s", config->type);
        return NULL;
    }

    /* Set widget name for CSS styling */
    if (config->id) {
        gtk_widget_set_name(widget, config->id);
    }

    /* Set size request */
    gtk_widget_set_size_request(widget, config->width, config->height);

    return widget;
}
