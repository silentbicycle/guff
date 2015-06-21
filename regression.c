#include "regression.h"
#include "scale.h"

/* Linear regression. */

// lr:{m:{(+/x)%1.0*#x};mx:m@x;my:m@y;dx:x-mx;n:+/dx*y-my;d:+/dx^2;s:n%d;(s;my-s*mx)}

static bool calc_means(point *points, size_t point_count, transform_t t, double *mx, double *my);
static double calc_num(point *points, size_t point_count, transform_t t, double mx, double my);
static double calc_den(point *points, size_t point_count, transform_t t, double mx);

void regression(point *points, size_t point_count, transform_t t,
        double *slope, double *intercept) {
    assert(slope);
    assert(intercept);

    double mx = 0;
    double my = 0;
    if (!calc_means(points, point_count, t, &mx, &my)) {
        *slope = EMPTY_VALUE;
        *intercept = EMPTY_VALUE;
        return;
    }

    double num = calc_num(points, point_count, t, mx, my);
    double den = calc_den(points, point_count, t, mx);
    *slope = num / den;
    *intercept = my - *slope * mx;

    LOG(1, "-- slope: %g, intercept: %g\n", *slope, *intercept);
}

static bool calc_means(point *points, size_t point_count, transform_t t, double *mx, double *my) {
    double tx = 0;
    double ty = 0;
    size_t cx = 0;
    size_t cy = 0;
    for (size_t i = 0; i < point_count; i++) {
        point *p = &points[i];
        if (IS_EMPTY_POINT(p)) { continue; }
        cx++;
        cy++;
        point tp;
        scale_transform(p, t, &tp);
        tx += tp.x;
        ty += tp.y;
    }

    if (cx == 0) { return false; }
    *mx = tx / cx;
    *my = ty / cy;
    return true;
}

static double calc_num(point *points, size_t point_count, transform_t t, double mx, double my) {
    double sum = 0;
    for (size_t i = 0; i < point_count; i++) {
        point *p = &points[i];
        point tp;
        scale_transform(p, t, &tp);
        sum += (tp.x - mx) * (tp.y - my);
    }
    return sum;
}

static double calc_den(point *points, size_t point_count, transform_t t, double mx) {
    double sum = 0;
    for (size_t i = 0; i < point_count; i++) {
        point *p = &points[i];
        point tp;
        scale_transform(p, t, &tp);
        sum += (tp.x - mx) * (tp.x - mx);
    }
    return sum;
}
