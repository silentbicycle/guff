#ifndef REGRESSION_H
#define REGRESSION_H

#include "guff.h"
#include "scale.h"

/* Linear regression. */
void regression(point *points, size_t point_count, transform_t t, double *slope, double *intercept);

#endif
