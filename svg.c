#include "svg.h"
#include "regression.h"
#include "scale.h"
#include "counter.h"

/* SVG generation. */

#define REGRESSION_LINE_WIDTH 2

static char *get_color(uint8_t column, svg_theme *theme);
static void svg_printf_header(size_t w, size_t h);
static void svg_printf_frame(size_t w, size_t h, char *fill_color, size_t border_width, char *border_color);
static void svg_printf_begin_polyline(void);
static void svg_printf_polyline_point(size_t x, size_t y);
static void svg_printf_end_polyline(char *color, size_t line_width);
static void svg_printf_circle(size_t x, size_t y, size_t point_size, char *color);
static void svg_printf_axis(plot_info *pi, svg_theme *theme);
static void svg_printf_regression_line(plot_info *pi, char *color, double slope, double intercept);
static void svg_printf_end(void);

int svg_plot(config *cfg, plot_info *pi, data_set *ds) {
    svg_theme *theme = cfg->svg_theme;
    svg_printf_header(pi->w, pi->h);
    svg_printf_frame(pi->w, pi->h, theme->bg_color, theme->border_width, theme->border_color);

    if (cfg->axis) {
        draw_calc_axis_pos(pi);
        svg_printf_axis(pi, theme);
    }

    transform_t transform = scale_get_transform(pi->log_x, pi->log_y);

    for (uint8_t c = 0; c < ds->columns; c++) {
        char *color = get_color(c, theme);
        point *column = ds->pairs[c];

        if (cfg->mode == MODE_LINE) {
            bool beginning_line = true;
            for (size_t i = 0; i < ds->rows; i++) {
                point *p = &column[i];
                if (IS_EMPTY(p->x) || IS_EMPTY(p->y)) {
                    if (!beginning_line) {
                        svg_printf_end_polyline(color, theme->line_width);
                    }
                    beginning_line = true;
                    continue;
                }

                if (beginning_line) { 
                    svg_printf_begin_polyline();
                    beginning_line = false;
                }
                scaled_point sp;
                scale_point(pi, p, &sp, transform);
                svg_printf_polyline_point(sp.x, sp.y);
            }
            svg_printf_end_polyline(color, theme->line_width);
        } else {
            for (size_t i = 0; i < ds->rows; i++) {
                point *p = &column[i];
                scaled_point sp;
                scale_point(pi, p, &sp, transform);

                if (IS_EMPTY(p->x) || IS_EMPTY(p->y)) { continue; }
                size_t point_size = SVG_DEF_POINT_SIZE;
                if (pi->counters) {
                    size_t count = counter_get(pi->counters[c], sp.x, sp.y);
                    point_size = SVG_DEF_POINT_SIZE + (cfg->log_count ? log(count) : count);
                }
                svg_printf_circle(sp.x, sp.y, point_size, color);
            }
        }

        if (cfg->regression) {
            double slope = 0;
            double intercept = 0;
            
            regression(column, ds->rows, transform, &slope, &intercept);
            svg_printf_regression_line(pi, color, slope, intercept);
        }
    }

    svg_printf_end();
    return 0;
}

static char *get_color(uint8_t column, svg_theme *theme) {
    if (column < SVG_COLOR_COUNT) {
        return theme->colors[column];
    } else {
        return theme->colors[SVG_COLOR_COUNT - 1];
    }
}

static void svg_printf_header(size_t w, size_t h) {
    printf("<svg xmlns=\"http://www.w3.org/2000/svg\" "
           "width=\"%zu\" height=\"%zu\" version=\"1.1\">\n",
        w, h);
    printf("<!-- Generator: guff %u.%u.%u -->\n",
        GUFF_VERSION_MAJOR, GUFF_VERSION_MINOR, GUFF_VERSION_PATCH);
}

static void svg_printf_frame(size_t w, size_t h, char *fill_color,
        size_t border_width, char *border_color) {
    printf("<rect x=\"0\" y=\"0\" width=\"%zu\" height=\"%zu\"\n",
        w, h);
    printf("    fill=\"%s\" stroke-width=\"%zu\" stroke=\"%s\" />\n",
        fill_color, border_width, border_color);
}

static void svg_printf_begin_polyline(void) {
    printf("<polyline points=\"\n");
}

static void svg_printf_polyline_point(size_t x, size_t y) {
    printf("    %zu,%zu\n", x, y);
}

static void svg_printf_end_polyline(char *color, size_t line_width) {
    printf("\" stroke=\"%s\" stroke-width=\"%zu\" fill=\"none\" />\n",
        color, line_width);
}

static void svg_printf_circle(size_t x, size_t y, size_t point_size, char *color) {
    printf("<circle cx=\"%zu\" cy=\"%zu\" r=\"%zu\" stroke=\"%s\" />\n",
        x, y, point_size, color);
}

static double scale_tick(size_t width, double range) {
    /* Return a size that divides the range to add roughly 5-10 ticks. */
    double rounded = pow(10, ceil(log10(range)));
    double step = rounded / (range < rounded / 2 ? 20 : 10);
    return width * (step / range);
}

static void svg_printf_axis(plot_info *pi, svg_theme *theme) {
    int tick_w = 3*theme->axis_width;

    // Y axis
    printf("<line x1=\"%zu\" y1=\"%d\" x2=\"%zu\" y2=\"%zu\" "
        "stroke=\"%s\" stroke-width=\"%u\" %s/>\n",
        pi->axis_x, 0, pi->axis_x, pi->h,
        theme->axis_color, theme->axis_width, pi->draw_y_axis ? "" : "stroke-dasharray=\"2,5\" ");

    // X axis ticks
    if (pi->draw_x_axis) {
        int y0 = pi->axis_y - tick_w;
        int y1 = pi->axis_y + tick_w;

        double xto = scale_tick(pi->w, pi->range_x);
        for (int wx = pi->axis_x + xto; wx < pi->w; wx += xto) {
            printf("<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
                "stroke=\"%s\" stroke-width=\"1\" />\n",
                wx, y0, wx, y1, theme->axis_color);
        }
        for (int wx = pi->axis_x - xto; wx > 0; wx -= xto) {
            printf("<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
                "stroke=\"%s\" stroke-width=\"1\" />\n",
                wx, y0, wx, y1, theme->axis_color);
        }
    }

    // X axis
    printf("<line x1=\"%zu\" y1=\"%zu\" x2=\"%zu\" y2=\"%zu\" "
        "stroke=\"%s\" stroke-width=\"%u\" %s/>\n",
        0L, pi->axis_y, pi->w, pi->axis_y,
        theme->axis_color, theme->axis_width, pi->draw_x_axis ? "" : "stroke-dasharray=\"2,5\" ");

    // Y axis ticks
    if (pi->draw_y_axis) {
        int x0 = pi->axis_x - tick_w;
        int x1 = pi->axis_x + tick_w;
        if (x0 > pi->w) { x0 = 0; }  // don't wrap

        double yto = scale_tick(pi->h, pi->range_y);
        for (int hy = pi->axis_y + yto; hy < pi->h; hy += yto) {
            printf("<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
                "stroke=\"%s\" stroke-width=\"1\" />\n",
                x0, hy, x1, hy, theme->axis_color);
        }
        for (int hy = pi->axis_y - yto; hy > 0; hy -= yto) {
            printf("<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
                "stroke=\"%s\" stroke-width=\"1\" />\n",
                x0, hy, x1, hy, theme->axis_color);
        }
    }
}

static void svg_printf_end(void) {
    printf("</svg>\n");
}

static void svg_printf_regression_line(plot_info *pi, char *color,
        double slope, double intercept) {

    point p0 = { .x = pi->min_x, .y = slope * pi->min_x + intercept };
    point p1 = { .x = pi->max_x, .y = slope * pi->max_x + intercept };
    LOG(2, "p0: %g * %g + %g => %g\n", slope, pi->min_x, intercept, p0.y);
    LOG(2, "p1: %g * %g + %g => %g\n", slope, pi->max_x, intercept, p1.y);

    scaled_point sp0, sp1;
    transform_t t = TRANSFORM_NONE;  // already transformed
    scale_point(pi, &p0, &sp0, t);
    scale_point(pi, &p1, &sp1, t);

    LOG(2, "p0: (%g, %g) => [%d, %d]\n", p0.x, p0.y, sp0.x, sp0.y);
    LOG(2, "p1: (%g, %g) => [%d, %d]\n", p1.x, p1.y, sp1.x, sp1.y);

    printf("<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
        "stroke=\"%s\" stroke-width=\"%u\" stroke-dasharray=\"2,5\" />\n",
        sp0.x, sp0.y, sp1.x, sp1.y, color, REGRESSION_LINE_WIDTH);
}
