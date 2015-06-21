#include "test_guff.h"

#include "regression.h"
#include <math.h>

static void setup_cb(void *data) {
}

static void teardown_cb(void *data) {
}

DEF_TEST(input_empty) {
    double slope = 0;
    double intercept = 0;
    regression(NULL, 0, TRANSFORM_NONE, &slope, &intercept);

    ASSERT(IS_EMPTY(slope));
    ASSERT(IS_EMPTY(intercept));

    PASS();
}

DEF_TEST(input_simple) {
    double slope = 0;
    double intercept = 0;
    
    point points[] = {
        {.x = 0, .y = 10},
        {.x = 1, .y = 15},
        {.x = 2, .y = 20},
        {.x = 3, .y = 25},
        {.x = 4, .y = 30},
    };
    regression(points, 5, TRANSFORM_NONE, &slope, &intercept);

    ASSERT_IN_RANGE(5, slope, 0.001);
    ASSERT_IN_RANGE(10, intercept, 0.001);

    PASS();
}

DEF_TEST(input_off_axis) {
    double slope = 0;
    double intercept = 0;
    
    point points[] = {
        {.x = 1000, .y = 1000},
        {.x = 1010, .y = 1010},
        {.x = 1020, .y = 1020},
        {.x = 1030, .y = 1035},
        {.x = 1040, .y = 1080},
    };
    regression(points, 5, TRANSFORM_NONE, &slope, &intercept);

    ASSERT_IN_RANGE(1.85, slope, 0.001);
    ASSERT_IN_RANGE(-858, intercept, 0.001);

    PASS();
}

DEF_TEST(input_xy) {
    double slope = 0;
    double intercept = 0;
    
    point points[] = {
        {.x = -3, .y = -2},
        {.x = -2, .y = -1},
        {.x = 0, .y = 1},
        {.x = 3, .y = 3},
        {.x = 9, .y = 9},
    };
    regression(points, 5, TRANSFORM_NONE, &slope, &intercept);

    ASSERT_IN_RANGE(0.901, slope, 0.001);
    ASSERT_IN_RANGE(0.738197, intercept, 0.001);

    PASS();
}

DEF_TEST(input_log_x) {
    double slope = 0;
    double intercept = 0;
    
    point points[] = {
        {.x = exp(0), .y = 0 + 50 },
        {.x = exp(1), .y = 1 + 50 },
        {.x = exp(2), .y = 2 + 50 },
        {.x = exp(3), .y = 3 + 50 },
        {.x = exp(4), .y = 4 + 50 },
    };
    regression(points, 5, TRANSFORM_LOG_X, &slope, &intercept);

    ASSERT(!isnan(slope));
    ASSERT(!isnan(intercept));
    ASSERT_IN_RANGE(1, slope, 0.001);
    ASSERT_IN_RANGE(50, intercept, 0.001);

    PASS();
}

DEF_TEST(input_log_y_no_intercept) {
    double slope = 0;
    double intercept = 0;
    
    point points[] = {
        {.x = 0, .y = exp(0)},
        {.x = 1, .y = exp(1)},
        {.x = 2, .y = exp(2)},
        {.x = 3, .y = exp(3)},
        {.x = 4, .y = exp(4)},
    };
    regression(points, 5, TRANSFORM_LOG_Y, &slope, &intercept);

    ASSERT(!isnan(slope));
    ASSERT(!isnan(intercept));
    ASSERT_IN_RANGE(1, slope, 0.001);
    ASSERT_IN_RANGE(0, intercept, 0.001);

    PASS();
}

DEF_TEST(input_log_y_intercept) {
    double slope = 0;
    double intercept = 0;
    
    point points[] = {
        {.x = 0, .y = exp(0) + 10 },
        {.x = 1, .y = exp(1) + 10 },
        {.x = 2, .y = exp(2) + 10 },
        {.x = 3, .y = exp(3) + 10 },
        {.x = 4, .y = exp(4) + 10 },
    };
    regression(points, 5, TRANSFORM_LOG_Y, &slope, &intercept);

    ASSERT(!isnan(slope));
    ASSERT(!isnan(intercept));
    ASSERT_IN_RANGE(0.440159, slope, 0.001);
    ASSERT_IN_RANGE(2.19348, intercept, 0.001);

    PASS();
}

SUITE(s_regression) {
    SET_SETUP(setup_cb, NULL);
    SET_TEARDOWN(teardown_cb, NULL);

    RUN_TEST(input_empty);
    RUN_TEST(input_simple);
    RUN_TEST(input_off_axis);
    RUN_TEST(input_xy);
    RUN_TEST(input_log_x);
    RUN_TEST(input_log_y_no_intercept);
    RUN_TEST(input_log_y_intercept);
}
