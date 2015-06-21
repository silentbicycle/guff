#include "scale.h"

/* Point scaling / transformations. */

void scale_point(plot_info *pi, point *p, scaled_point *out_p, transform_t t) {
    const uint8_t pad = 2;
    point tp;
    scale_transform(p, t, &tp);
    double x = tp.x;
    double y = tp.y;
    
    double cmp_pad = 0.001;
    assert(x >= pi->min_x - cmp_pad);
    assert(x <= pi->max_x + cmp_pad);

    double cell_w = pi->range_x / pi->w;
    double cell_h = pi->range_y / pi->h;

    out_p->x = (pi->w - pad) * ((x - pi->min_x + cell_w/2) / pi->range_x);

    int32_t oy = (pi->h - pad) * ((y - pi->min_y + cell_h/2) / pi->range_y);
    // flip y; 0 at bottom of plot
    out_p->y = pi->h - oy - 1;

    LOG(2, "range_x: %g, range_y: %g, cell_w: %g, cell_h: %g\n",
        pi->range_x, pi->range_y, cell_w, cell_h);
    LOG(2, "[%u, %d | %d] / [%zu, %zu]\n",
        out_p->x, oy, out_p->y, pi->w, pi->h);
}

transform_t scale_get_transform(bool log_x, bool log_y) {
    transform_t res = TRANSFORM_NONE;
    if (log_x) { res |= TRANSFORM_LOG_X; }
    if (log_y) { res |= TRANSFORM_LOG_Y; }
    return res;
}

void scale_transform(point *p, transform_t t, point *out) {
    if (t & TRANSFORM_LOG_X) {
        out->x = p->x == 0 ? 0 : log(p->x);
    } else {
        out->x = p->x;
    }
    if (t & TRANSFORM_LOG_Y) {
        out->y = p->y == 0 ? 0 : log(p->y);
    } else {
        out->y = p->y;
    }
}
