#ifndef TEST_GUFF_H
#define TEST_GUFF_H

#include "greatest.h"
#include "guff.h"

#define DEF_TEST(X) TEST X(void)

SUITE(s_draw);
SUITE(s_input);
SUITE(s_regression);
SUITE(s_scale);

extern greatest_type_info *type_point;

#endif
