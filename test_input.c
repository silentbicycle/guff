#include "test_guff.h"

#include "input.h"
#include "input_internal.h"
#include <math.h>

static data_set ds;
static config empty_cfg;

static void setup_cb(void *data) {
    memset(&ds, 0, sizeof(ds));
}

static void teardown_cb(void *data) {
    input_free(&ds);
}

DEF_TEST(input_empty) {
    init_pairs(&ds);
    char empty[] = "\n";
    ASSERT_EQ(SINK_LINE_EMPTY, sink_line(&empty_cfg, &ds, NULL, 0, 0));
    ASSERT_EQ(SINK_LINE_EMPTY, sink_line(&empty_cfg, &ds, empty, 1, 0));
    PASS();
}

DEF_TEST(input_comment) {
    init_pairs(&ds);
    char c1[] = "#a comment";
    char c2[] = "// a comment";
    ASSERT_EQ(SINK_LINE_COMMENT, sink_line(&empty_cfg, &ds, c1, strlen(c1), 0));
    ASSERT_EQ(SINK_LINE_COMMENT, sink_line(&empty_cfg, &ds, c2, strlen(c2), 0));
    PASS();
}

DEF_TEST(input_single) {
    init_pairs(&ds);
    ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, "23", 2, 0));
    ASSERT_EQ(1, ds.columns);

    point exp = { .x = 0, .y = 23 };
    ASSERT_EQUAL_T(&exp, &ds.pairs[0][0], type_point, NULL);
    
    PASS();
}

DEF_TEST(input_pair) {
    init_pairs(&ds);
    ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, "23 24", 5, 0));

    ASSERT_EQ(2, ds.columns);
    point exp0 = { .x = 0, .y = 23 };
    point exp1 = { .x = 0, .y = 24 };
    ASSERT_EQUAL_T(&exp0, &ds.pairs[0][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp1, &ds.pairs[1][0], type_point, NULL);
    
    PASS();
}

DEF_TEST(input_floats) {
    init_pairs(&ds);
    ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, "99.9 999.999", 13, 0));

    ASSERT_EQ(2, ds.columns);
    point exp0 = { .x = 0, .y = 99.9 };
    point exp1 = { .x = 0, .y = 999.999 };
    ASSERT_EQUAL_T(&exp0, &ds.pairs[0][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp1, &ds.pairs[1][0], type_point, NULL);
    
    PASS();
}

DEF_TEST(input_csv) {
    init_pairs(&ds);
    ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, "23,24", 15, 0));

    ASSERT_EQ(2, ds.columns);
    ASSERT_EQ(1, ds.rows);
    point exp0 = { .x = 0, .y = 23 };
    point exp1 = { .x = 0, .y = 24 };
    ASSERT_EQUAL_T(&exp0, &ds.pairs[0][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp1, &ds.pairs[1][0], type_point, NULL);
    
    PASS();
}

DEF_TEST(input_tab) {
    init_pairs(&ds);

    ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, "23\t24", 5, 0));

    ASSERT_EQ(2, ds.columns);
    ASSERT_EQ(1, ds.rows);
    point exp0 = { .x = 0, .y = 23 };
    point exp1 = { .x = 0, .y = 24 };
    ASSERT_EQUAL_T(&exp0, &ds.pairs[0][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp1, &ds.pairs[1][0], type_point, NULL);
    
    PASS();
}

DEF_TEST(input_exponent) {
    init_pairs(&ds);

    ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, "-24e-7,+50e5", 16, 0));

    ASSERT_EQ(2, ds.columns);
    ASSERT_EQ(1, ds.rows);
    point exp0 = { .x = 0, .y = -24e-7 };
    point exp1 = { .x = 0, .y = 5e6 };
    ASSERT_EQUAL_T(&exp0, &ds.pairs[0][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp1, &ds.pairs[1][0], type_point, NULL);
    
    PASS();
}

DEF_TEST(input_multiline) {
    init_pairs(&ds);
    ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, "23", 2, 0));
    ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, "24", 2, 1));

    ASSERT_EQ(1, ds.columns);
    ASSERT_EQ(2, ds.rows);
    point exp0_0 = { .x = 0, .y = 23 };
    point exp1_0 = { .x = 1, .y = 24 };
    ASSERT_EQUAL_T(&exp0_0, &ds.pairs[0][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp1_0, &ds.pairs[0][1], type_point, NULL);
    
    PASS();
}

DEF_TEST(input_single_column_null) {
    init_pairs(&ds);
    char *lines[] = {
        "1278",
        "377",
        "316",
        "232",
        "_",
        "93",
        "63",
        "11",
    };
    for (size_t i = 0; i < sizeof(lines)/sizeof(lines[0]); i++) {
        char *line = lines[i];
        ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, line, strlen(line), i));
    }

    ASSERT_EQ(1, ds.columns);
    ASSERT_EQ(8, ds.rows);
    point exp0 = { .x = 0, .y = 1278 };
    point exp1 = { .x = 1, .y = 377 };
    point exp4 = { .x = 4, .y = EMPTY_VALUE };
    point exp5 = { .x = 5, .y = 93 };

    ASSERT_EQUAL_T(&exp0, &ds.pairs[0][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp1, &ds.pairs[0][1], type_point, NULL);
    ASSERT_EQUAL_T(&exp4, &ds.pairs[0][4], type_point, NULL);
    ASSERT_EQUAL_T(&exp5, &ds.pairs[0][5], type_point, NULL);
    
    PASS();
}

DEF_TEST(input_multiline_null) {
    init_pairs(&ds);
    ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, "1,2,3", 5, 0));
    ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, "4,,6", 4, 1));

    ASSERT_EQ(3, ds.columns);
    ASSERT_EQ(2, ds.rows);
    point exp0_0 = { .x = 0, .y = 1 };
    point exp0_1 = { .x = 0, .y = 2 };
    point exp0_2 = { .x = 0, .y = 3 };

    point exp1_0 = { .x = 1, .y = 4 };
    point exp1_1 = { .x = 1, .y = NAN };
    point exp1_2 = { .x = 1, .y = 6 };
    ASSERT_EQUAL_T(&exp0_0, &ds.pairs[0][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp0_1, &ds.pairs[1][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp0_2, &ds.pairs[2][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp1_0, &ds.pairs[0][1], type_point, NULL);
    ASSERT_EQUAL_T(&exp1_1, &ds.pairs[1][1], type_point, NULL);
    ASSERT_EQUAL_T(&exp1_2, &ds.pairs[2][1], type_point, NULL);
    
    PASS();
}

DEF_TEST(input_trailing_null) {
    init_pairs(&ds);
    ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, "1,2,3,", 6, 0));

    ASSERT_EQ(4, ds.columns);
    ASSERT_EQ(1, ds.rows);
    point exp0_0 = { .x = 0, .y = 1 };
    point exp0_1 = { .x = 0, .y = 2 };
    point exp0_2 = { .x = 0, .y = 3 };
    point exp0_3 = { .x = 0, .y = NAN };

    ASSERT_EQUAL_T(&exp0_0, &ds.pairs[0][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp0_1, &ds.pairs[1][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp0_2, &ds.pairs[2][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp0_3, &ds.pairs[3][0], type_point, NULL);
    
    PASS();
}

DEF_TEST(input_leading_null) {
    init_pairs(&ds);
    ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, ",1,2,3", 6, 0));

    ASSERT_EQ_FMT(4, ds.columns, "%u");
    ASSERT_EQ(1, ds.rows);
    point exp0_0 = { .x = 0, .y = NAN };
    point exp0_1 = { .x = 0, .y = 1 };
    point exp0_2 = { .x = 0, .y = 2 };
    point exp0_3 = { .x = 0, .y = 3 };

    ASSERT_EQUAL_T(&exp0_0, &ds.pairs[0][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp0_1, &ds.pairs[1][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp0_2, &ds.pairs[2][0], type_point, NULL);
    ASSERT_EQUAL_T(&exp0_3, &ds.pairs[3][0], type_point, NULL);
    
    PASS();
}

DEF_TEST(row_with_more_columns_pads_previous_with_nulls) {
    init_pairs(&ds);
    // Pascal's Triangle
    ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, "1", 1, 0));
    ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, "1,1", 3, 1));
    ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, "1,2,1", 5, 2));

    ASSERT_EQ(3, ds.columns);
    ASSERT_EQ(3, ds.rows);

    /* Expected data, exp[COL][ROW].
     * The normal layout makes this a bit confusing. */
    point exp[3][3] = {
        { { .x = 0, .y = 1},
          { .x = 1, .y = 1},
          { .x = 2, .y = 1}, },
                            { { .x = 0, .y = NAN},
                              { .x = 1, .y = 1},
                              { .x = 2, .y = 2}, },
                                                { { .x = 0, .y = NAN},
                                                  { .x = 1, .y = NAN},
                                                  { .x = 2, .y = 1}, },
    };

    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            ASSERT_EQUAL_T(&exp[c][r], &ds.pairs[c][r], type_point, NULL);
        }
    }

    PASS();
}

DEF_TEST(afl_crash0) {
    init_pairs(&ds);
    char *lines[] = {
        "-3 -3 -2",
        "- -2",
        "-\x01 -1",
        "0 1",
        "32",
        "-\x01 1",
        "3 \x11",
        "9 9",
    };
    for (size_t i = 0; i < sizeof(lines)/sizeof(lines[0]); i++) {
        char *line = lines[i];
        ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, line, strlen(line), i));
    }

    ASSERT_EQ(3, ds.columns);
    ASSERT_EQ(8, ds.rows);
    point exp[3][8] = {
        { { .x = 0, .y = -3},
          { .x = 1, .y = NAN},
          { .x = 2, .y = NAN},
          { .x = 3, .y = 0},
          { .x = 4, .y = 32},
          { .x = 5, .y = NAN},
          { .x = 6, .y = 3},
          { .x = 7, .y = 9}, },
                            { { .x = 0, .y = -3},
                              { .x = 1, .y = NAN},
                              { .x = 2, .y = NAN},
                              { .x = 3, .y = 1},
                              { .x = 4, .y = NAN},
                              { .x = 5, .y = NAN},
                              { .x = 6, .y = NAN},
                              { .x = 7, .y = 9}, },
                                                { { .x = 0, .y = -2},
                                                  { .x = 1, .y = NAN},
                                                  { .x = 2, .y = NAN},
                                                  { .x = 3, .y = NAN},
                                                  { .x = 4, .y = NAN},
                                                  { .x = 5, .y = NAN},
                                                  { .x = 6, .y = NAN},
                                                  { .x = 7, .y = NAN}, },
    };

    for (int r = 0; r < ds.rows; r++) {
        for (int c = 0; c < ds.columns; c++) {
            point *p = &ds.pairs[c][r];
            if (GREATEST_IS_VERBOSE()) { printf("(%8g, %8g)\t", p->x, p->y); }
            ASSERT_EQUAL_T(&exp[c][r], p, type_point, NULL);
        }
        if (GREATEST_IS_VERBOSE()) { printf("\n"); }
    }

    PASS();
}

DEF_TEST(afl_crash1) {
    init_pairs(&ds);
    char *lines[] = {
        "?",
    };
    for (size_t i = 0; i < sizeof(lines)/sizeof(lines[0]); i++) {
        char *line = lines[i];
        ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, line, strlen(line), i));
    }

    ASSERT_EQ(1, ds.columns);
    ASSERT_EQ(1, ds.rows);

    point exp = { .x = 0, .y = NAN };
    ASSERT_EQUAL_T(&exp, &ds.pairs[0][0], type_point, NULL);
    PASS();
}

DEF_TEST(afl_crash2) {
    init_pairs(&ds);
    char *lines[] = {
        "-3E3333333333333333",
    };
    for (size_t i = 0; i < sizeof(lines)/sizeof(lines[0]); i++) {
        char *line = lines[i];
        ASSERT_EQ(SINK_LINE_OK, sink_line(&empty_cfg, &ds, line, strlen(line), i));
    }

    ASSERT_EQ(1, ds.columns);
    ASSERT_EQ(1, ds.rows);

    point exp = { .x = 0, .y = EMPTY_VALUE };
    ASSERT_EQUAL_T(&exp, &ds.pairs[0][0], type_point, NULL);
    PASS();
}

SUITE(s_input) {
    SET_SETUP(setup_cb, NULL);
    SET_TEARDOWN(teardown_cb, NULL);

    RUN_TEST(input_empty);
    RUN_TEST(input_comment);
    RUN_TEST(input_single);
    RUN_TEST(input_pair);
    RUN_TEST(input_floats);
    RUN_TEST(input_csv);
    RUN_TEST(input_tab);
    RUN_TEST(input_exponent);
    RUN_TEST(input_multiline);

    // empty cell handling
    RUN_TEST(input_single_column_null);
    RUN_TEST(input_multiline_null);
    RUN_TEST(input_trailing_null);
    RUN_TEST(input_leading_null);
    RUN_TEST(row_with_more_columns_pads_previous_with_nulls);

    // fuzzer cases
    RUN_TEST(afl_crash0);
    RUN_TEST(afl_crash1);
    RUN_TEST(afl_crash2);
}
