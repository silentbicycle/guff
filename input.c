#include "input.h"
#include "input_internal.h"

/* Input handling. */

static void add_pair(config *cfg, data_set *ds, size_t row, uint8_t col, point *p);
static bool number_head_char(char c);

static char buf[64 * 1024];

int input_read(config *cfg, data_set *ds) {
    char *line = NULL;
    
    size_t row_count = 0;

    init_pairs(ds);

    while ((line = fgets(buf, sizeof(buf) - 1, cfg->in))) {
        size_t len = strlen(line);
        
        sink_line_res res = sink_line(cfg, ds, line, len, row_count);
        switch (res) {
        case SINK_LINE_OK:
            row_count++;
            break;
        case SINK_LINE_EMPTY:
            return (cfg->stream_mode ? 0 : -1);
        case SINK_LINE_COMMENT:
            continue;
        case SINK_LINE_DONE:
            return 0;
            
        default:
            assert(false);
        }
    }

    return -1;  // end of stream
}

static bool is_comment_marker(char c) {
    switch (c) {
    case '#': case '/':
        return true;
    default:
        return false;
    }
}

sink_line_res sink_line(config *cfg, data_set *ds, char *line, size_t len, size_t row_count) {
    size_t col = 0;

    if (len == 0) { return SINK_LINE_EMPTY; }
    if (line[len - 1] == '\n') { line[len - 1] = '\0'; len--; }
    if (*line == '\0') { return SINK_LINE_EMPTY; }
    LOG(3, "sink_line: %s\n", line);
    
    float cur_x = row_count;
    bool has_x = false;

    // ignore comments
    if (is_comment_marker(line[0])) { return SINK_LINE_COMMENT; }

    size_t offset = 0;
    while (offset < len && line[offset]) {
        double v = 0;
        if (offset >= len) { break; }

        // ignore comment to EOL
        if (is_comment_marker(line[offset])) { return SINK_LINE_OK; }

        if (!number_head_char(line[offset])) {
            if (offset == 0) {
                v = EMPTY_VALUE;
            } else {
                offset++;
                if (!number_head_char(line[offset])) {
                    v = EMPTY_VALUE;
                }
            }
        }

        char *cur_line = &line[offset];
        char *out_line = NULL;
        if (isnan(v)) {
            offset++;   // already got the value
        } else {
            v = strtod(cur_line, &out_line);
            if (isinf(v)) { v = EMPTY_VALUE; }
        }
        if (out_line == cur_line) {
            break;
        } else {
            // could do this in terms of *line == '\0' or line[offset] == len.
            // strtod shifts &line.
            while(&line[offset] < out_line) {
                offset++;
            }
        }

        if (cfg->x_column && !has_x) {
            cur_x = v;
            has_x = true;
        } else {
            point p = { .x = cur_x, .y = v };
            add_pair(cfg, ds, row_count, col, &p);
            col++;
            if (col == MAX_COLUMNS) { break; }
        }
    }

    // Fill remaining columns with EMPTY_VALUE
    for (size_t c = col; c < ds->columns; c++) {
        point p = { .x = cur_x, .y = EMPTY_VALUE };
        add_pair(cfg, ds, row_count, c, &p);
    }

    return SINK_LINE_OK;
}

/* Is c a character which can be at the start of a double literal? */
static bool number_head_char(char c) {
    switch (c) {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    case '-': case '+': case '.': case 'e':
        return true;
    default:
        return false;
    }
}

#define MIN_ROW_CEIL2 1

void init_pairs(data_set *ds) {
    point **cols = calloc(1, sizeof(point *));
    if (cols == NULL) { err(1, "calloc"); }

    cols[0] = calloc(1 << MIN_ROW_CEIL2, sizeof(point));
    if (cols[0] == NULL) { err(1, "calloc"); }

    ds->row_ceil2 = MIN_ROW_CEIL2;
    ds->columns = 1;
    ds->pairs = cols;
}

static void add_pair(config *cfg, data_set *ds, size_t row, uint8_t col, point *p) {
    // add columns, padding out empty cells as necessary
    if (col >= ds->columns) {
        point **npairs = realloc(ds->pairs, (1 + col) * sizeof(*npairs));
        LOG(2, "growing ds->pairs: %p(%u) => %p(%u), %zu bytes\n",
            ds->pairs, ds->columns, npairs, 1 + col, (1 + col) * sizeof(*npairs));
        if (npairs == NULL) { err(1, "realloc"); }

        uint8_t row_ceil2 = ds->row_ceil2;
        if (row_ceil2 == 0) {
            ds->row_ceil2 = MIN_ROW_CEIL2;
            row_ceil2 = ds->row_ceil2;
        }

        point *column = calloc(1, (1 << row_ceil2) * sizeof(point));
        if (column == NULL) { err(1, "calloc"); }
        npairs[col] = column;
        ds->pairs = npairs;
        ds->columns = col + 1;
        for (size_t i = 0; i < ds->rows; i++) {
            point np = { .x = i, .y = EMPTY_VALUE };
            if (cfg->x_column) {
                assert(col > 0);
                np.x = npairs[col][0].x;
            }
            npairs[col][i] = np;
        }
    }

    // grow rows
    if (row >= (1 << ds->row_ceil2)) {
        uint8_t nceil2 = ds->row_ceil2;
        while ((1 << nceil2) <= row) { nceil2++; }

        if (nceil2 > ds->row_ceil2) {
            for (uint8_t c = 0; c < ds->columns; c++) {
                point *ncol = realloc(ds->pairs[c], (1 << nceil2) * sizeof(point));
                LOG(2, "growing column %u, %p (%d) => %p (%d, %zu bytes)\n",
                    c, ds->pairs[c], 1 << ds->row_ceil2,
                    ncol, 1 << nceil2, (1 << nceil2) * sizeof(point));
                if (ncol == NULL) { err(1, "realloc"); }
                ds->pairs[c] = ncol;
            }
            ds->row_ceil2 = nceil2;
        }
    }

    if (row >= ds->rows) { ds->rows = row + 1; }

    assert(ds->columns > col);
    assert(ds->rows > row);

    if (cfg->flip_xy) {
        ds->pairs[col][row] = (point){ .x = p->y, .y = p->x };
    } else {
        ds->pairs[col][row] = *p;
    }
    LOG(2, "-- set [c:%u,r:%zu] to (%g, %g)\n",
        col, row, ds->pairs[col][row].x, ds->pairs[col][row].y);
}

void input_free(data_set *ds) {
    if (ds && ds->pairs) {
        point **pairs = ds->pairs;
        for (uint8_t c = 0; c < ds->columns; c++) {
            free(pairs[c]);
            pairs[c] = NULL;
        }
        free(pairs);
        memset(ds, 0, sizeof(*ds));
    }
}
