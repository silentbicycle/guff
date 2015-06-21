#ifndef DRAW_H
#define DRAW_H

#include "guff.h"

typedef struct {
    double min_x;
    double max_x;
    double min_y;
    double max_y;

    double range_x;
    double range_y;

    bool log_x;
    bool log_y;
    size_t w;
    size_t h;

    struct counter **counters;

    char **rows;

    bool draw_x_axis;
    bool draw_y_axis;
    size_t axis_x;
    size_t axis_y;
} plot_info;

int draw(config *cfg, data_set *ds);
void draw_scale_point(plot_info *pi, point *p, size_t *out_x, size_t *out_y);
void draw_calc_bounds(data_set *ds, plot_info *pi);
void draw_calc_axis_pos(plot_info *pi);

#endif
