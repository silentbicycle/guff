#include "test_guff.h"

#include "scale.h"

static data_set ds;
static plot_info pi;

static void setup_cb(void *data) {
    memset(&pi, 0, sizeof(pi));
    memset(&ds, 0, sizeof(ds));
}

static void teardown_cb(void *data) {
    /* free_rows(&pi); */
    /* input_free(&ds); */
}

DEF_TEST(get_transform) {
    ASSERT_EQ(TRANSFORM_NONE,   scale_get_transform(false, false));
    ASSERT_EQ(TRANSFORM_LOG_X,  scale_get_transform(true, false));
    ASSERT_EQ(TRANSFORM_LOG_Y,  scale_get_transform(false, true));
    ASSERT_EQ(TRANSFORM_LOG_XY, scale_get_transform(true, true));
    PASS();
}

DEF_TEST(transform) {
    point p = { .x = 10, .y = 100 };
    point outn, outx, outy, outxy;

    scale_transform(&p, TRANSFORM_NONE, &outn);
    scale_transform(&p, TRANSFORM_LOG_X, &outx);
    scale_transform(&p, TRANSFORM_LOG_Y, &outy);
    scale_transform(&p, TRANSFORM_LOG_XY, &outxy);

    point exp_outn = { .x = 10, .y = 100 };
    point exp_outx = { .x = log(10), .y = 100 };
    point exp_outy = { .x = 10, .y = log(100) };
    point exp_outxy = { .x = log(10), .y = log(100) };

    ASSERT_EQUAL_T(&exp_outn, &outn, type_point, NULL);
    ASSERT_EQUAL_T(&exp_outx, &outx, type_point, NULL);
    ASSERT_EQUAL_T(&exp_outy, &outy, type_point, NULL);
    ASSERT_EQUAL_T(&exp_outxy, &outxy, type_point, NULL);
    PASS();
}

static transform_t init_pi(plot_info *pi) {
    pi->range_x = pi->max_x - pi->min_x;
    pi->range_y = pi->max_y - pi->min_y;

    if (pi->w == 0) { pi->w = 72; }
    if (pi->h == 0) { pi->h = 40; }

    return scale_get_transform(pi->log_x, pi->log_y);
}

DEF_TEST(scale_basic) {
    plot_info pi = {
        .min_x = 0,
        .max_x = 100,
        .min_y = 0,
        .max_y = 100,
    };
    transform_t t = init_pi(&pi);
    point p = { .x = 0, .y = 0 };

    scaled_point sp;
    scale_point(&pi, &p, &sp, t);

#define ASSERT_SCALED_TO(SP, X, Y)                                      \
    ASSERT_EQ_FMT((X), SP.x, "%d");                                     \
    ASSERT_EQ_FMT((Y), SP.y, "%d")

    ASSERT_SCALED_TO(sp, 0, pi.h - 1);
    PASS();
}

DEF_TEST(scale_centered_origin) {
    plot_info pi = {
        .min_x = -100,
        .max_x = 100,
        .min_y = -100,
        .max_y = 100,
    };
    transform_t t = init_pi(&pi);
    point p = { .x = 0, .y = 0 };

    scaled_point sp;
    scale_point(&pi, &p, &sp, t);

    ASSERT_SCALED_TO(sp, pi.w/2 - 1, pi.h/2);
    PASS();
}

DEF_TEST(scale_example_regression) {
    plot_info pi = {
        .min_x = 1000,
        .max_x = 1040,
        .min_y = 1000,
        .max_y = 1080,

        .w = 640,
        .h = 480,
    };
    transform_t t = init_pi(&pi);
    point p0 = { .x = 1000, .y = 992 };
    point p1 = { .x = 1040, .y = 1066 };

    scaled_point sp;
    scale_point(&pi, &p0, &sp, t);
    ASSERT_SCALED_TO(sp, 0, 526);

    scale_point(&pi, &p1, &sp, t);
    ASSERT_SCALED_TO(sp, 638, 85);
    PASS();
}

DEF_TEST(scale_points) {
    plot_info pi = {
        .min_x = -100,
        .max_x = 100,
        .min_y = -100,
        .max_y = 100,
    };
    transform_t t = init_pi(&pi);
    point p0 = { .x = 10, .y = 20 };
    point p1 = { .x = 50, .y = 50 };
    point p2 = { .x = -25, .y = -10 };
    point p3 = { .x = 15, .y = 20 };
    point p4 = { .x = -7, .y = 8 };

    scaled_point sp;

    scale_point(&pi, &p0, &sp, t);
    ASSERT_SCALED_TO(sp, 38, 16);

    scale_point(&pi, &p1, &sp, t);
    ASSERT_SCALED_TO(sp, 52, 11);

    scale_point(&pi, &p2, &sp, t);
    ASSERT_SCALED_TO(sp, 26, 22);

    scale_point(&pi, &p3, &sp, t);
    ASSERT_SCALED_TO(sp, 40, 16);

    scale_point(&pi, &p4, &sp, t);
    ASSERT_SCALED_TO(sp, 33, 19);

    PASS();
}

DEF_TEST(scale_basic_log) {
    plot_info pi = {
        .min_x = 0,
        .max_x = log(100),
        .min_y = 0,
        .max_y = log(1000),

        .log_x = true,
        .log_y = true,
    };
    transform_t t = init_pi(&pi);
    point p = { .x = 50, .y = 100 };

    scaled_point sp;
    scale_point(&pi, &p, &sp, t);
    ASSERT_SCALED_TO(sp, 59, 14);

    PASS();
}

DEF_TEST(scale_points_log) {
    plot_info pi = {
        .min_x = -100,
        .max_x = 100,
        .min_y = 0,
        .max_y = log(100),
        .log_y = true,
    };
    transform_t t = init_pi(&pi);
    point p0 = { .x = 10, .y = 20 };
    point p1 = { .x = 50, .y = 50 };
    point p2 = { .x = -25, .y = 1 };
    point p3 = { .x = 15, .y = 20 };
    point p4 = { .x = -7, .y = 8 };

    scaled_point sp;

    scale_point(&pi, &p0, &sp, t);
    ASSERT_SCALED_TO(sp, 38, 14);

    scale_point(&pi, &p1, &sp, t);
    ASSERT_SCALED_TO(sp, 52, 7);

    scale_point(&pi, &p2, &sp, t);
    ASSERT_SCALED_TO(sp, 26, 39);

    scale_point(&pi, &p3, &sp, t);
    ASSERT_SCALED_TO(sp, 40, 14);

    scale_point(&pi, &p4, &sp, t);
    ASSERT_SCALED_TO(sp, 33, 22);

    PASS();
}

DEF_TEST(scale_out_of_range) {
    plot_info pi = {
        .min_x = 1000,
        .max_x = 1040,
        .min_y = 1850,
        .max_y = 1925,

        .w = 640,
        .h = 480,
    };
    transform_t t = init_pi(&pi);
    point p0 = { .x = 1000, .y = 1850 };
    point p1 = { .x = 1040, .y = 1924 };

    scaled_point sp;

    scale_point(&pi, &p0, &sp, t);
    ASSERT_SCALED_TO(sp, 0, 479);

    scale_point(&pi, &p1, &sp, t);
    ASSERT_SCALED_TO(sp, 638, 7);

    PASS();
}

SUITE(s_scale) {
    SET_SETUP(setup_cb, NULL);
    SET_TEARDOWN(teardown_cb, NULL);

    RUN_TEST(get_transform);
    RUN_TEST(transform);

    RUN_TEST(scale_basic);
    RUN_TEST(scale_centered_origin);
    RUN_TEST(scale_example_regression);
    RUN_TEST(scale_points);
    RUN_TEST(scale_basic_log);
    RUN_TEST(scale_points_log);

    RUN_TEST(scale_out_of_range);
}
