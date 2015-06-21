#include "test_guff.h"

static int point_printf_cb(const void *v, void *udata) {
    point *p = (point *)v;
    char buf[256];
    int res = snprintf(buf, 256, "(%g, %g)", p->x, p->y);
    if (256 < res) { return -1; }
    printf("%s", buf);
    return res;
}

static int point_equal_cb(const void *vexp, const void *vgot, void *udata) {
    point *exp = (point *)vexp;
    point *got = (point *)vgot;
    double tol = 0.000000001;
    if (udata != NULL) { tol = *(double *)udata; }

    if (IS_EMPTY(exp->x) && !IS_EMPTY(got->x)) {
        return 0;
    } else if (fabs(exp->x - got->x) > tol) {
        return 0;
    }

    if (IS_EMPTY(exp->y) && !IS_EMPTY(got->y)) {
        return 0;
    } else if (fabs(exp->y - got->y) > tol) {
        return 0;
    }

    return 1;
}

static greatest_type_info type_point_def = {
    .equal = point_equal_cb,
    .print = point_printf_cb,
};

greatest_type_info *type_point = &type_point_def;
