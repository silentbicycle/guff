#include "test_guff.h"

#include "draw.h"
#include "input.h"
#include "input_internal.h"

static data_set ds;
static plot_info pi;

void free_rows(plot_info *pi);

static void setup_cb(void *data) {
    memset(&pi, 0, sizeof(pi));
    memset(&ds, 0, sizeof(ds));
}

static void teardown_cb(void *data) {
    free_rows(&pi);
    input_free(&ds);
}

#define READ_LINES_AND_INIT_PI(CFG, LINES)                              \
    do {                                                                \
        if (CFG.log_x) { pi.log_x = true; }                             \
        if (CFG.log_y) { pi.log_y = true; }                             \
        size_t count = sizeof(LINES)/sizeof(LINES[0]);                  \
        for (size_t i = 0; i < count; i++) {                            \
            char *line = LINES[i];                                      \
            ASSERT_EQ(SINK_LINE_OK,                                     \
                sink_line(&CFG, &ds, line, strlen(line), i));           \
        }                                                               \
    } while (0)                                                         \

DEF_TEST(bounds_basic) {
    config cfg;
    memset(&cfg, 0, sizeof(cfg));
    char *lines[] = {
        "1 2 3",
        "4 5 6",
        "7 8 9",
    };
    READ_LINES_AND_INIT_PI(cfg, lines);

    draw_calc_bounds(&ds, &pi);
    ASSERT_IN_RANGE(0, pi.min_x, 0.0001);
    ASSERT_IN_RANGE(2, pi.max_x, 0.0001);

    // 0, not 1 -> include axis
    ASSERT_IN_RANGE(0, pi.min_y, 0.0001);
    ASSERT_IN_RANGE(9, pi.max_y, 0.0001);

    PASS();
}

DEF_TEST(bounds_empty_cells) {
    config cfg;
    memset(&cfg, 0, sizeof(cfg));
    char *lines[] = {
        " 2 3",
        "4  6",
        "7 8 ",
    };
    READ_LINES_AND_INIT_PI(cfg, lines);

    draw_calc_bounds(&ds, &pi);
    ASSERT_IN_RANGE(0, pi.min_x, 0.0001);
    ASSERT_IN_RANGE(2, pi.max_x, 0.0001);

    // 0, not 1 -> include axis
    ASSERT_IN_RANGE(0, pi.min_y, 0.0001);
    ASSERT_IN_RANGE(8, pi.max_y, 0.0001);

    PASS();
}

DEF_TEST(bounds_negative_quadrant) {
    config cfg = {
        .x_column = true,
    };
    char *lines[] = {
        "-5 -50",
        "-3 -30",
        "-1 -10",
    };
    READ_LINES_AND_INIT_PI(cfg, lines);

    draw_calc_bounds(&ds, &pi);
    ASSERT_IN_RANGE(-5, pi.min_x, 0.0001);
    ASSERT_IN_RANGE(0, pi.max_x, 0.0001);

    ASSERT_IN_RANGE(-50, pi.min_y, 0.0001);
    ASSERT_IN_RANGE(0, pi.max_y, 0.0001);

    PASS();
}

DEF_TEST(bounds_4q) {
    config cfg = {
        .x_column = true,
    };
    char *lines[] = {
        "-50 -50",
        "-49 49",
        "0 0",
        "10 10",
    };
    READ_LINES_AND_INIT_PI(cfg, lines);

    draw_calc_bounds(&ds, &pi);
    ASSERT_IN_RANGE(-50, pi.min_x, 0.0001);
    ASSERT_IN_RANGE(10, pi.max_x, 0.0001);

    ASSERT_IN_RANGE(-50, pi.min_y, 0.0001);
    ASSERT_IN_RANGE(49, pi.max_y, 0.0001);

    PASS();
}

DEF_TEST(bounds_diff_quadrants_for_columns) {
    config cfg = {
        .x_column = true,
    };
    char *lines[] = {
        "-50 -50 50",
        "-49 -49 49",
        "-48 -48 48",
        "-47 -47 47",
    };
    READ_LINES_AND_INIT_PI(cfg, lines);

    draw_calc_bounds(&ds, &pi);
    ASSERT_IN_RANGE(-50, pi.min_x, 0.0001);
    ASSERT_IN_RANGE(-47, pi.max_x, 0.0001);

    ASSERT_IN_RANGE(-50, pi.min_y, 0.0001);
    ASSERT_IN_RANGE(50, pi.max_y, 0.0001);

    PASS();
}

DEF_TEST(bounds_too_distant_to_touch_axis) {
    config cfg = {
        .x_column = true,
    };
    char *lines[] = {
        "-5000 -5000",
        "-4900 -4900",
        "-4800 -4800",
    };
    READ_LINES_AND_INIT_PI(cfg, lines);

    draw_calc_bounds(&ds, &pi);
    ASSERT_IN_RANGE(-5000, pi.min_x, 0.0001);
    ASSERT_IN_RANGE(-4800, pi.max_x, 0.0001);

    ASSERT_IN_RANGE(-5000, pi.min_y, 0.0001);
    ASSERT_IN_RANGE(-4800, pi.max_y, 0.0001);

    PASS();
}

DEF_TEST(bounds_log_basic) {
    config cfg = {
        .log_y = true,
    };
    char *lines[] = {
        "1214",
        "358",
        "316",
        "187",
        "186",
        "93",
        "63",
        "11",
    };
    READ_LINES_AND_INIT_PI(cfg, lines);

    draw_calc_bounds(&ds, &pi);
    ASSERT_IN_RANGE(0, pi.min_x, 0.0001);
    ASSERT_IN_RANGE(7, pi.max_x, 0.0001);

    ASSERT_IN_RANGE(0, pi.min_y, 0.0001);
    ASSERT_IN_RANGE(/* log(1214) */ 7.101, pi.max_y, 0.001);
    PASS();
}

DEF_TEST(bounds_log_distant) {
    config cfg = {
        .log_y = true,
    };
    char *lines[] = {
        "1214",
        "358",
        "316",
        "187",
        "186",
    };
    READ_LINES_AND_INIT_PI(cfg, lines);

    draw_calc_bounds(&ds, &pi);
    ASSERT_IN_RANGE(0, pi.min_x, 0.0001);
    ASSERT_IN_RANGE(4, pi.max_x, 0.0001);

    ASSERT_IN_RANGE(5.2257, pi.min_y, 0.0001);
    ASSERT_IN_RANGE(7.101, pi.max_y, 0.001);
    PASS();
}

DEF_TEST(reject_x_range_of_zero) {
    config cfg;
    memset(&cfg, 0, sizeof(cfg));
    char *lines[] = {
        "-3 -2",
    };
    READ_LINES_AND_INIT_PI(cfg, lines);

    draw_calc_bounds(&ds, &pi);
    ASSERT(pi.range_x != 0);
    PASS();
}

DEF_TEST(reject_y_range_of_zero) {
    config cfg;
    memset(&cfg, 0, sizeof(cfg));
    char *lines[] = {
        "0 0",
        "0 0",
    };
    READ_LINES_AND_INIT_PI(cfg, lines);

    draw_calc_bounds(&ds, &pi);
    ASSERT(pi.range_y != 0);
    PASS();
}

SUITE(s_draw) {
    SET_SETUP(setup_cb, NULL);
    SET_TEARDOWN(teardown_cb, NULL);

    // calculating bounds / setting up plot info
    RUN_TEST(bounds_basic);
    RUN_TEST(bounds_empty_cells);
    RUN_TEST(bounds_negative_quadrant);
    RUN_TEST(bounds_4q);
    RUN_TEST(bounds_diff_quadrants_for_columns);
    RUN_TEST(bounds_too_distant_to_touch_axis);
    RUN_TEST(bounds_log_basic);
    RUN_TEST(bounds_log_distant);

    RUN_TEST(reject_x_range_of_zero);
    RUN_TEST(reject_y_range_of_zero);
}
