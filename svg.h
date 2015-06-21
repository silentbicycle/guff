#ifndef SVG_H
#define SVG_H

#include "guff.h"
#include "draw.h"

#define SVG_COLOR_COUNT 9
#define SVG_DEF_POINT_SIZE 2

typedef struct svg_theme {
    char *bg_color;
    char *border_color;
    char *axis_color;
    char *colors[SVG_COLOR_COUNT];
    uint8_t line_width;
    uint8_t axis_width;
    uint8_t border_width;
} svg_theme;

int svg_plot(config *cfg, plot_info *pi, data_set *ds);

#endif
