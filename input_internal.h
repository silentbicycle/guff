#ifndef INPUT_INTERNAL_H
#define INPUT_INTERNAL_H

typedef enum {
    SINK_LINE_OK,
    SINK_LINE_EMPTY,
    SINK_LINE_COMMENT,
    SINK_LINE_DONE,
} sink_line_res;

void init_pairs(data_set *ds);
sink_line_res sink_line(config *cfg, data_set *ds, char *line, size_t len, size_t row_count);

#endif
