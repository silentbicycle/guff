#include "test_guff.h"

/* Add all the definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();      /* command-line arguments, initialization. */
    RUN_SUITE(s_input);
    RUN_SUITE(s_draw);
    RUN_SUITE(s_regression);
    RUN_SUITE(s_scale);
    GREATEST_MAIN_END();        /* display results */
}
