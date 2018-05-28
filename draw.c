#include <math.h>
#include <float.h>

#include "draw.h"
#include "scale.h"

#include "ascii.h"
#include "braille.h"
#include "svg.h"
#include "counter.h"

/* Common drawing functionality. */

static void count_points(counter *counter, plot_info *pi, data_set *ds, uint8_t column);
static bool all_empty_points(plot_info *pi);
static bool insufficient_range(plot_info *pi);

static const double MAX = 1e100;  // TODO: portable constants? DBL_MAX?
static const double MIN = -1e100;

int draw(config *cfg, data_set *ds) {
    plot_info pi;
    memset(&pi, 0, sizeof(pi));

    pi.log_x = cfg->log_x;
    pi.log_y = cfg->log_y;

    draw_calc_bounds(ds, &pi);

    if (all_empty_points(&pi)) { return 0; }
    if (insufficient_range(&pi)) { return 0; }

    pi.w = cfg->width;
    pi.h = cfg->height;

    if (cfg->mode == MODE_COUNT) {
        pi.counters = calloc(ds->columns, sizeof(counter *));
        assert(pi.counters);
        for (uint8_t c = 0; c < ds->columns; c++) {
            counter *counter = counter_init(ds->rows);
            assert(counter);
            count_points(counter, &pi, ds, c);
            pi.counters[c] = counter;
        }
    }
    
    int res = 0;

    switch (cfg->plot_type) {
    case PLOT_ASCII:
        res = ascii_plot(cfg, &pi, ds);
        break;

    case PLOT_BRAILLE:
        res = braille_plot(cfg, &pi, ds);
        break;

    case PLOT_SVG:
        res = svg_plot(cfg, &pi, ds);
        break;

    default:
        assert(false);
        break;
    }

    if (pi.counters) {
        for (uint8_t c = 0; c < ds->columns; c++) {
            counter_free(pi.counters[c]);
        }
        free(pi.counters);
    }
    return res;
}

static bool all_empty_points(plot_info *pi) {
    return (pi->min_x == MAX || pi->min_y == MAX);
}

static bool insufficient_range(plot_info *pi) {
    return (pi->range_x == 0) || (pi->range_y == 0);
}

void draw_calc_bounds(data_set *ds, plot_info *pi) {
    point min_p = { .x = MAX, .y = MAX };
    point max_p = { .x = MIN, .y = MIN };

    for (uint8_t c = 0; c < ds->columns; c++) {
        for (size_t r = 0; r < ds->rows; r++) {
            point *p = &ds->pairs[c][r];

            if (IS_EMPTY_POINT(p)) { continue; }

            if (pi->log_x && p->x <= 0) {
                fprintf(stderr, "floating point error: log(%g)\n", p->x);
                exit(1);
            }
            if (pi->log_y && p->y <= 0) {
                fprintf(stderr, "floating point error: log(%g)\n", p->y);
                exit(1);
            }

            double x = p->x;
            double y = p->y;

            if (x < min_p.x) { min_p.x = x; }
            if (x > max_p.x) { max_p.x = x; }

            if (y < min_p.y) { min_p.y = y; }
            if (y > max_p.y) { max_p.y = y; }
        }
    }

    transform_t t = scale_get_transform(pi->log_x, pi->log_y);
    point out_min_p, out_max_p;
    scale_transform(&min_p, t, &out_min_p);
    scale_transform(&max_p, t, &out_max_p);
    
    pi->min_x = out_min_p.x;
    pi->min_y = out_min_p.y;
    pi->max_x = out_max_p.x;
    pi->max_y = out_max_p.y;

    /* Override bounds that would lead to a range of zero, to avoid a
     * crash when plotting. (Found by afl.) */
    if (pi->min_x == pi->max_x) { pi->max_x = pi->min_x + 1; }
    if (pi->min_y == pi->max_y) { pi->max_y = pi->min_y + 1; }

    pi->range_x = pi->max_x - pi->min_x;
    pi->range_y = pi->max_y - pi->min_y;
    
    bool crosses_x = pi->min_x <= 0 && pi->max_x >= 0;
    bool crosses_y = pi->min_y <= 0 && pi->max_y >= 0;
    
    double cross_pad = CROSS_PAD;

    if (!crosses_x) {
        if (0 < pi->min_x && 0 > pi->min_x - pi->range_x*cross_pad) {
            pi->min_x = 0;
            pi->range_x = pi->max_x;
        } else if (0 > pi->max_x && 0 < pi->max_x + pi->range_x*cross_pad) {
            pi->max_x = 0;
            pi->range_x = -pi->min_x;
        }
    }

    if (!crosses_y) {
        if (0 < pi->min_y && 0 > pi->min_y - pi->range_y*cross_pad) {
            pi->min_y = 0;
            pi->range_y = pi->max_y;
        } else if (0 > pi->max_y && 0 < pi->max_y + pi->range_y*cross_pad) {
            pi->max_y = 0;
            pi->range_y = -pi->min_y;
        }
    }

    LOG(1, "mx %g, Mx %g, my %g, My %g\n",
        pi->min_x, pi->max_x, pi->min_y, pi->max_y);
}

void draw_calc_axis_pos(plot_info *pi) {
    pi->draw_x_axis = (0 >= pi->min_y && 0 <= pi->max_y);
    pi->draw_y_axis = (0 >= pi->min_x && 0 <= pi->max_x);

    point origin = { .x = 0, .y = 0 };
    if (!pi->draw_y_axis) {
        if (0 < pi->min_x) {
            origin.x = pi->min_x;
        } else {
            origin.x = pi->max_x;
        }
    }

    if (!pi->draw_x_axis) {
        if (0 < pi->min_y) {
            origin.y = pi->min_y;
        } else {
            origin.y = pi->max_y;
        }
    }

    scaled_point sp;
    scale_point(pi, &origin, &sp, scale_get_transform(pi->log_x, pi->log_y));
    pi->axis_x = sp.x;
    pi->axis_y = sp.y;
    LOG(1, "axis at: (%g, %g) scaled to (%d, %d)\n", origin.x, origin.y, sp.x, sp.y);
}

static void count_points(counter *counter, plot_info *pi, data_set *ds, uint8_t column) {
    point *points = ds->pairs[column];
    transform_t t = scale_get_transform(pi->log_x, pi->log_y);

    for (size_t r = 0; r < ds->rows; r++) {
        point *p = &points[r];
        if (IS_EMPTY(p->x) || IS_EMPTY(p->y)) { continue; }
        scaled_point sp;
        scale_point(pi, p, &sp, t);

        counter_increment(counter, sp.x, sp.y);
    }
}
