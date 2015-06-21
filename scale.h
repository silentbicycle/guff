#ifndef SCALE_H
#define SCALE_H

#include "guff.h"
#include "draw.h"

typedef struct {
    int32_t x;
    int32_t y;
} scaled_point;

typedef enum {
    TRANSFORM_NONE = 0,
    TRANSFORM_LOG_X = 1,
    TRANSFORM_LOG_Y = 2,
    TRANSFORM_LOG_XY = 3,
} transform_t;

void scale_point(plot_info *pi, point *p, scaled_point *out_p, transform_t t);

transform_t scale_get_transform(bool log_x, bool log_y);
void scale_transform(point *p, transform_t t, point *out);

#endif
