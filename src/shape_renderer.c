#include "shape_renderer.h"
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Shape data stored as user_data on each GtkDrawingArea */
typedef struct {
    char *type;
    int width;
    int height;
    JsonObject *props;
} ShapeData;

static void shape_data_free(gpointer data) {
    ShapeData *sd = (ShapeData *)data;
    if (!sd) return;
    g_free(sd->type);
    if (sd->props) json_object_unref(sd->props);
    g_free(sd);
}

/* Parse #RRGGBB or "transparent". Returns 1 if opaque, 0 if transparent. */
static int parse_color(const char *hex, double *r, double *g, double *b) {
    if (!hex || strcmp(hex, "transparent") == 0) return 0;
    if (hex[0] != '#' || strlen(hex) < 7) return 0;
    unsigned int val;
    sscanf(hex + 1, "%06x", &val);
    *r = ((val >> 16) & 0xFF) / 255.0;
    *g = ((val >> 8)  & 0xFF) / 255.0;
    *b = ( val        & 0xFF) / 255.0;
    return 1;
}

static const char* get_str_prop(JsonObject *props, const char *key, const char *def) {
    if (!props || !json_object_has_member(props, key)) return def;
    JsonNode *node = json_object_get_member(props, key);
    if (!JSON_NODE_HOLDS_VALUE(node)) return def;
    const char *v = json_node_get_string(node);
    return v ? v : def;
}

static double get_dbl_prop(JsonObject *props, const char *key, double def) {
    if (!props || !json_object_has_member(props, key)) return def;
    JsonNode *node = json_object_get_member(props, key);
    if (!JSON_NODE_HOLDS_VALUE(node)) return def;
    return json_node_get_double(node);
}

static int get_int_prop_s(JsonObject *props, const char *key, int def) {
    if (!props || !json_object_has_member(props, key)) return def;
    JsonNode *node = json_object_get_member(props, key);
    if (!JSON_NODE_HOLDS_VALUE(node)) return def;
    return (int)json_node_get_int(node);
}

/* ── Line ────────────────────────────────────────────────── */
static void draw_line(GtkDrawingArea *area, cairo_t *cr, int w, int h, gpointer user_data) {
    ShapeData *sd = (ShapeData *)user_data;
    const char *stroke_color = get_str_prop(sd->props, "stroke_color", "#ECEFF4");
    double stroke_width = get_dbl_prop(sd->props, "stroke_width", 2.0);
    const char *direction = get_str_prop(sd->props, "direction", "horizontal");

    double r, g, b;
    if (!parse_color(stroke_color, &r, &g, &b)) return;

    cairo_set_source_rgb(cr, r, g, b);
    cairo_set_line_width(cr, stroke_width);

    double x1, y1, x2, y2;
    if (strcmp(direction, "horizontal") == 0) {
        x1 = 0; y1 = h / 2.0; x2 = w; y2 = h / 2.0;
    } else if (strcmp(direction, "vertical") == 0) {
        x1 = w / 2.0; y1 = 0; x2 = w / 2.0; y2 = h;
    } else if (strcmp(direction, "diagonal-se") == 0) {
        x1 = 0; y1 = 0; x2 = w; y2 = h;
    } else if (strcmp(direction, "diagonal-ne") == 0) {
        x1 = 0; y1 = h; x2 = w; y2 = 0;
    } else {
        x1 = 0; y1 = h / 2.0; x2 = w; y2 = h / 2.0;
    }

    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);
}

/* ── Rect ────────────────────────────────────────────────── */
static void draw_rect(GtkDrawingArea *area, cairo_t *cr, int w, int h, gpointer user_data) {
    ShapeData *sd = (ShapeData *)user_data;
    const char *fill_color = get_str_prop(sd->props, "fill_color", "transparent");
    const char *stroke_color = get_str_prop(sd->props, "stroke_color", "#ECEFF4");
    double stroke_width = get_dbl_prop(sd->props, "stroke_width", 2.0);
    double border_radius = get_dbl_prop(sd->props, "border_radius", 0.0);

    double fr, fg, fb, sr, sg, sb;
    int has_fill = parse_color(fill_color, &fr, &fg, &fb);
    int has_stroke = parse_color(stroke_color, &sr, &sg, &sb);

    double offset = stroke_width / 2.0;
    double rx = offset, ry = offset;
    double rw = w - stroke_width, rh = h - stroke_width;

    if (border_radius > 0) {
        double rad = border_radius;
        cairo_new_sub_path(cr);
        cairo_arc(cr, rx + rw - rad, ry + rad, rad, -M_PI / 2.0, 0);
        cairo_arc(cr, rx + rw - rad, ry + rh - rad, rad, 0, M_PI / 2.0);
        cairo_arc(cr, rx + rad, ry + rh - rad, rad, M_PI / 2.0, M_PI);
        cairo_arc(cr, rx + rad, ry + rad, rad, M_PI, 3.0 * M_PI / 2.0);
        cairo_close_path(cr);
    } else {
        cairo_rectangle(cr, rx, ry, rw, rh);
    }

    if (has_fill) {
        cairo_set_source_rgb(cr, fr, fg, fb);
        if (has_stroke) {
            cairo_fill_preserve(cr);
        } else {
            cairo_fill(cr);
        }
    }

    if (has_stroke && stroke_width > 0) {
        cairo_set_source_rgb(cr, sr, sg, sb);
        cairo_set_line_width(cr, stroke_width);
        cairo_stroke(cr);
    }
}

/* ── Ellipse ─────────────────────────────────────────────── */
static void draw_ellipse(GtkDrawingArea *area, cairo_t *cr, int w, int h, gpointer user_data) {
    ShapeData *sd = (ShapeData *)user_data;
    const char *fill_color = get_str_prop(sd->props, "fill_color", "transparent");
    const char *stroke_color = get_str_prop(sd->props, "stroke_color", "#ECEFF4");
    double stroke_width = get_dbl_prop(sd->props, "stroke_width", 2.0);

    double fr, fg, fb, sr, sg, sb;
    int has_fill = parse_color(fill_color, &fr, &fg, &fb);
    int has_stroke = parse_color(stroke_color, &sr, &sg, &sb);

    double cx = w / 2.0;
    double cy = h / 2.0;
    double rx = (w - stroke_width) / 2.0;
    double ry = (h - stroke_width) / 2.0;

    cairo_save(cr);
    cairo_translate(cr, cx, cy);
    cairo_scale(cr, rx, ry);
    cairo_arc(cr, 0, 0, 1.0, 0, 2.0 * M_PI);
    cairo_restore(cr);

    if (has_fill) {
        cairo_set_source_rgb(cr, fr, fg, fb);
        if (has_stroke) {
            cairo_fill_preserve(cr);
        } else {
            cairo_fill(cr);
        }
    }

    if (has_stroke && stroke_width > 0) {
        cairo_set_source_rgb(cr, sr, sg, sb);
        cairo_set_line_width(cr, stroke_width);
        cairo_stroke(cr);
    }
}

/* ── Triangle ────────────────────────────────────────────── */
static void draw_triangle(GtkDrawingArea *area, cairo_t *cr, int w, int h, gpointer user_data) {
    ShapeData *sd = (ShapeData *)user_data;
    const char *fill_color = get_str_prop(sd->props, "fill_color", "transparent");
    const char *stroke_color = get_str_prop(sd->props, "stroke_color", "#ECEFF4");
    double stroke_width = get_dbl_prop(sd->props, "stroke_width", 2.0);
    const char *direction = get_str_prop(sd->props, "direction", "up");

    double fr, fg, fb, sr, sg, sb;
    int has_fill = parse_color(fill_color, &fr, &fg, &fb);
    int has_stroke = parse_color(stroke_color, &sr, &sg, &sb);

    double px[3], py[3];
    if (strcmp(direction, "up") == 0) {
        px[0] = w / 2.0; py[0] = 0;
        px[1] = w;        py[1] = h;
        px[2] = 0;        py[2] = h;
    } else if (strcmp(direction, "down") == 0) {
        px[0] = 0;        py[0] = 0;
        px[1] = w;        py[1] = 0;
        px[2] = w / 2.0; py[2] = h;
    } else if (strcmp(direction, "left") == 0) {
        px[0] = w; py[0] = 0;
        px[1] = w; py[1] = h;
        px[2] = 0; py[2] = h / 2.0;
    } else if (strcmp(direction, "right") == 0) {
        px[0] = 0; py[0] = 0;
        px[1] = w; py[1] = h / 2.0;
        px[2] = 0; py[2] = h;
    } else {
        px[0] = w / 2.0; py[0] = 0;
        px[1] = w;        py[1] = h;
        px[2] = 0;        py[2] = h;
    }

    cairo_move_to(cr, px[0], py[0]);
    cairo_line_to(cr, px[1], py[1]);
    cairo_line_to(cr, px[2], py[2]);
    cairo_close_path(cr);

    if (has_fill) {
        cairo_set_source_rgb(cr, fr, fg, fb);
        if (has_stroke) cairo_fill_preserve(cr); else cairo_fill(cr);
    }
    if (has_stroke && stroke_width > 0) {
        cairo_set_source_rgb(cr, sr, sg, sb);
        cairo_set_line_width(cr, stroke_width);
        cairo_stroke(cr);
    }
}

/* ── Diamond ─────────────────────────────────────────────── */
static void draw_diamond(GtkDrawingArea *area, cairo_t *cr, int w, int h, gpointer user_data) {
    ShapeData *sd = (ShapeData *)user_data;
    const char *fill_color = get_str_prop(sd->props, "fill_color", "transparent");
    const char *stroke_color = get_str_prop(sd->props, "stroke_color", "#ECEFF4");
    double stroke_width = get_dbl_prop(sd->props, "stroke_width", 2.0);

    double fr, fg, fb, sr, sg, sb;
    int has_fill = parse_color(fill_color, &fr, &fg, &fb);
    int has_stroke = parse_color(stroke_color, &sr, &sg, &sb);

    cairo_move_to(cr, w / 2.0, 0);
    cairo_line_to(cr, w, h / 2.0);
    cairo_line_to(cr, w / 2.0, h);
    cairo_line_to(cr, 0, h / 2.0);
    cairo_close_path(cr);

    if (has_fill) {
        cairo_set_source_rgb(cr, fr, fg, fb);
        if (has_stroke) cairo_fill_preserve(cr); else cairo_fill(cr);
    }
    if (has_stroke && stroke_width > 0) {
        cairo_set_source_rgb(cr, sr, sg, sb);
        cairo_set_line_width(cr, stroke_width);
        cairo_stroke(cr);
    }
}

/* ── Arrow ───────────────────────────────────────────────── */
static void draw_arrow(GtkDrawingArea *area, cairo_t *cr, int w, int h, gpointer user_data) {
    ShapeData *sd = (ShapeData *)user_data;
    const char *stroke_color = get_str_prop(sd->props, "stroke_color", "#ECEFF4");
    double stroke_width = get_dbl_prop(sd->props, "stroke_width", 2.0);
    const char *direction = get_str_prop(sd->props, "direction", "right");

    double r, g, b;
    if (!parse_color(stroke_color, &r, &g, &b)) return;

    cairo_set_source_rgb(cr, r, g, b);
    cairo_set_line_width(cr, stroke_width);

    double x1, y1, x2, y2;
    if (strcmp(direction, "right") == 0) {
        x1 = 0; y1 = h / 2.0; x2 = w; y2 = h / 2.0;
    } else if (strcmp(direction, "left") == 0) {
        x1 = w; y1 = h / 2.0; x2 = 0; y2 = h / 2.0;
    } else if (strcmp(direction, "up") == 0) {
        x1 = w / 2.0; y1 = h; x2 = w / 2.0; y2 = 0;
    } else if (strcmp(direction, "down") == 0) {
        x1 = w / 2.0; y1 = 0; x2 = w / 2.0; y2 = h;
    } else {
        x1 = 0; y1 = h / 2.0; x2 = w; y2 = h / 2.0;
    }

    /* Draw the line */
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);

    /* Draw arrowhead at (x2, y2) */
    double arrow_size = fmax(8.0, stroke_width * 4.0);
    double angle = atan2(y2 - y1, x2 - x1);

    double ax1 = x2 - arrow_size * cos(angle - M_PI / 6.0);
    double ay1 = y2 - arrow_size * sin(angle - M_PI / 6.0);
    double ax2 = x2 - arrow_size * cos(angle + M_PI / 6.0);
    double ay2 = y2 - arrow_size * sin(angle + M_PI / 6.0);

    cairo_move_to(cr, x2, y2);
    cairo_line_to(cr, ax1, ay1);
    cairo_line_to(cr, ax2, ay2);
    cairo_close_path(cr);
    cairo_fill(cr);
}

/* ── Star ────────────────────────────────────────────────── */
static void draw_star(GtkDrawingArea *area, cairo_t *cr, int w, int h, gpointer user_data) {
    ShapeData *sd = (ShapeData *)user_data;
    const char *fill_color = get_str_prop(sd->props, "fill_color", "transparent");
    const char *stroke_color = get_str_prop(sd->props, "stroke_color", "#ECEFF4");
    double stroke_width = get_dbl_prop(sd->props, "stroke_width", 2.0);
    int points = get_int_prop_s(sd->props, "points", 5);

    if (points < 3) points = 3;
    if (points > 20) points = 20;

    double fr, fg, fb, sr, sg, sb;
    int has_fill = parse_color(fill_color, &fr, &fg, &fb);
    int has_stroke = parse_color(stroke_color, &sr, &sg, &sb);

    double cx = w / 2.0;
    double cy = h / 2.0;
    double R = fmin(w, h) / 2.0;
    double r_inner = R * 0.4;

    int total = points * 2;
    for (int i = 0; i < total; i++) {
        double angle = M_PI * i / points - M_PI / 2.0;
        double radius = (i % 2 == 0) ? R : r_inner;
        double vx = cx + radius * cos(angle);
        double vy = cy + radius * sin(angle);

        if (i == 0) {
            cairo_move_to(cr, vx, vy);
        } else {
            cairo_line_to(cr, vx, vy);
        }
    }
    cairo_close_path(cr);

    if (has_fill) {
        cairo_set_source_rgb(cr, fr, fg, fb);
        if (has_stroke) cairo_fill_preserve(cr); else cairo_fill(cr);
    }
    if (has_stroke && stroke_width > 0) {
        cairo_set_source_rgb(cr, sr, sg, sb);
        cairo_set_line_width(cr, stroke_width);
        cairo_stroke(cr);
    }
}

/* ── Public API ──────────────────────────────────────────── */

gboolean is_shape_type(const char *type) {
    if (!type) return FALSE;
    return (strcmp(type, "Line") == 0 ||
            strcmp(type, "Rect") == 0 ||
            strcmp(type, "Ellipse") == 0 ||
            strcmp(type, "Triangle") == 0 ||
            strcmp(type, "Diamond") == 0 ||
            strcmp(type, "Arrow") == 0 ||
            strcmp(type, "Star") == 0);
}

GtkWidget* shape_renderer_create(const WidgetConfig *config) {
    if (!config || !config->type) return NULL;

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(drawing_area), config->width);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(drawing_area), config->height);
    gtk_widget_set_size_request(drawing_area, config->width, config->height);

    /* Store shape data */
    ShapeData *sd = g_new0(ShapeData, 1);
    sd->type = g_strdup(config->type);
    sd->width = config->width;
    sd->height = config->height;
    if (config->props) {
        sd->props = json_object_ref(config->props);
    }

    /* Select draw function based on type */
    GtkDrawingAreaDrawFunc draw_func = NULL;
    if (strcmp(config->type, "Line") == 0) {
        draw_func = draw_line;
    } else if (strcmp(config->type, "Rect") == 0) {
        draw_func = draw_rect;
    } else if (strcmp(config->type, "Ellipse") == 0) {
        draw_func = draw_ellipse;
    } else if (strcmp(config->type, "Triangle") == 0) {
        draw_func = draw_triangle;
    } else if (strcmp(config->type, "Diamond") == 0) {
        draw_func = draw_diamond;
    } else if (strcmp(config->type, "Arrow") == 0) {
        draw_func = draw_arrow;
    } else if (strcmp(config->type, "Star") == 0) {
        draw_func = draw_star;
    }

    if (draw_func) {
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area),
                                       draw_func, sd, shape_data_free);
    } else {
        shape_data_free(sd);
    }

    /* Set widget name for identification */
    if (config->id) {
        gtk_widget_set_name(drawing_area, config->id);
    }

    return drawing_area;
}
