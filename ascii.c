#include "ascii.h"
#include "counter.h"
#include "scale.h"

/* Plotting to ASCII. */

static void init_rows(plot_info *pi);
static void draw_axes(plot_info *pi);
static void print_header(plot_info *pi, bool point_counts, uint8_t columns);
static char col_mark(uint8_t col);
static void plot_points(config *cfg, plot_info *pi, data_set *ds);

void free_rows(plot_info *pi);

int ascii_plot(config *cfg, plot_info *pi, data_set *ds) {
    init_rows(pi);
    if (cfg->axis) {
        draw_calc_axis_pos(pi);
        draw_axes(pi);
    }
    
    print_header(pi, cfg->mode == MODE_COUNT, ds->columns);
    plot_points(cfg, pi, ds);
    
    for (size_t i = 0; i < pi->h; i++) {
        printf("%s\n", pi->rows[i]);
    }
    free_rows(pi);
    return 0;
}

void free_rows(plot_info *pi) {
    if (pi) {
        for (size_t i = 0; i < pi->h; i++) {
            free(pi->rows[i]);
        }
        free(pi->rows);
        pi->rows = NULL;
    }
}

static void init_rows(plot_info *pi) {
    char **rows = calloc(pi->h, sizeof(*rows));
    if (rows == NULL) { err(1, "calloc"); }

    for (size_t i = 0; i < pi->h; i++) {
        rows[i] = malloc(pi->w + 1);
        if (rows[i] == NULL) { err(1, "malloc"); }
        memset(rows[i], ' ', pi->w);
        rows[i][pi->w] = '\0';
    }
    pi->rows = rows;
}

static void print_header(plot_info *pi, bool point_counts, uint8_t columns) {
    if (pi->log_x) {
        printf("    x: log [%g - %g]", exp(pi->min_x), exp(pi->max_x));
    } else {
        printf("    x: [%g - %g]", pi->min_x, pi->max_x);
    }

    if (pi->log_y) {
        printf("    y: log [%g - %g]", exp(pi->min_y), exp(pi->max_y));
    } else {
        printf("    y: [%g - %g]", pi->min_y, pi->max_y);
    }

    if (!point_counts) {
        printf(" -- ");
        for (uint8_t i = 0; i < columns; i++) {
            printf("%s%d: %c", i > 0 ? ", " : "", i, col_mark(i));
        }
    }
    printf("\n");
}

static void draw_axes(plot_info *pi) {
    char c;

    /* TODO: use same step size calc as SVG */

    for (size_t i = 0; i < pi->h; i++) {
        if (pi->draw_y_axis) {
            c = (i % 5 == 0 ? '+' : '|');
        } else {
            c = (i % 5 == 0 ? '.' : ' ');
        }

        pi->rows[i][pi->axis_x] = c;
    }
    
    for (size_t i = 0; i < pi->w; i++) {
        if (pi->draw_x_axis) {
            c = (i % 5 == 0 ? '+' : '-');
        } else {
            c = (i % 5 == 0 ? '.' : ' ');
        }
        pi->rows[pi->axis_y][i] = c;
    }
    
    pi->rows[pi->axis_y][pi->axis_x] = '+';
}

static char col_marks[] = "#@*^!~%ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static char col_mark(uint8_t col) {
    if (col > sizeof(col_marks) / sizeof(col_marks[0])) { return '*'; }
    return col_marks[col];
}

static void plot_points(config *cfg, plot_info *pi, data_set *ds) {
    for (uint8_t c = 0; c < ds->columns; c++) {
        for (size_t r = 0; r < ds->rows; r++) {
            point *p = &ds->pairs[c][r];

            if (IS_EMPTY(p->x) || IS_EMPTY(p->y)) { continue; }
            transform_t t = scale_get_transform(pi->log_x, pi->log_y);
            scaled_point sp;
            scale_point(pi, p, &sp, t);
            LOG(2, "{ %g, %g } => [%u, %u]\n", p->x, p->y, sp.x, sp.y);

            char mark = col_mark(c);
            if (pi->counters) {
                size_t count = counter_get(pi->counters[c], sp.x, sp.y);
                if (count < 10) {
                    mark = '0' + count;
                } else if (count < 36) {
                    mark = 'a' + count - 10;
                } else {
                    mark = '#';
                }
            }
            pi->rows[sp.y][sp.x] = mark;
        }
    }
}
