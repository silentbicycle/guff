#include "braille.h"
#include "scale.h"

/* Plotting to UTF-8 braille characters. */

static void init_rows(plot_info *pi);
static void draw_axes(plot_info *pi);
static void print_header(plot_info *pi, uint8_t columns);
static void plot_points(config *cfg, plot_info *pi, data_set *ds);

static void free_rows(plot_info *pi);

static void utf8(int cp, char c[]) {
    if (cp <= 0x7f) {
        c[0] =  cp;
        return;
    }

    if (cp <= 0x7ff) {
        c[0] = (cp >>  6) + 192;
        c[1] = (cp  & 63) + 128;
        return;
    }

    if (0xd800 <= cp && cp <= 0xdfff) {
        /* invalid */
        goto error;
    }

    if (cp <= 0xffff) {
        c[0] =  (cp >> 12) + 224;
        c[1] = ((cp >>  6) &  63) + 128;
        c[2] =  (cp  & 63) + 128;
        return;
    }

    if (cp <= 0x10ffff) {
        c[0] =  (cp >> 18) + 240;
        c[1] = ((cp >> 12) &  63) + 128;
        c[2] = ((cp >>  6) &  63) + 128;
        c[3] =  (cp  & 63) + 128;
        return;
    }

error:

    fprintf(stderr, "codepoint out of range\n");
    exit(1);
}

int braille_plot(config *cfg, plot_info *pi, data_set *ds) {
    struct {
        int j;
        int i;
        int m;
    } braille[] = {
        { 0, 0, 0x2801 }, { 1, 0, 0x2808 },
        { 0, 1, 0x2802 }, { 1, 1, 0x2810 },
        { 0, 2, 0x2804 }, { 1, 2, 0x2820 },
        { 0, 3, 0x2840 }, { 1, 3, 0x2880 }
    };

    init_rows(pi);
    if (cfg->axis) {
        draw_calc_axis_pos(pi);
        draw_axes(pi);
    }

    print_header(pi, ds->columns);
    plot_points(cfg, pi, ds);

    for (size_t i = 0; i < pi->h - 3; i += 4) {
        for (size_t j = 0; j < pi->w - 1; j += 2) {
            char s[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
            size_t k;
            int cp;

            cp = 0x2800; /* blank */

            for (k = 0; k < sizeof braille / sizeof *braille; k++) {
                if (pi->rows[i + braille[k].i][j + braille[k].j] != ' ') {
                    cp |= braille[k].m;
                }
            }

            utf8(cp, s);

            printf("%s", s);
        }
        printf("\n");
    }
    free_rows(pi);
    return 0;
}

static void free_rows(plot_info *pi) {
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

static void print_header(plot_info *pi, uint8_t columns) {
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

    printf("\n");
}

static void draw_axes(plot_info *pi) {
    char c;

    /* TODO: use same step size calc as SVG */

    for (size_t i = 0; i < pi->h; i++) {
        if (pi->draw_y_axis) {
            c = '|';
        } else {
            c = ' ';
        }

        pi->rows[i][pi->axis_x] = c;
    }

    for (size_t i = 0; i < pi->w; i++) {
        if (pi->draw_x_axis) {
            c = '-';
        } else {
            c = ' ';
        }
        pi->rows[pi->axis_y][i] = c;
    }

    pi->rows[pi->axis_y][pi->axis_x] = '+';
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

            pi->rows[sp.y][sp.x] = '@';
        }
    }
}
