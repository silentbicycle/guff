#ifndef TYPES_H
#define TYPES_H

typedef struct {
    double x;
    double y;
} point;

typedef struct {
    uint8_t row_ceil2;
    uint8_t columns;
    size_t rows;
    point **pairs;  // p[col] -> p[row]
} data_set;

typedef enum {
    PLOT_ASCII,
    PLOT_BRAILLE,
    PLOT_SVG,
} output_t;

typedef enum {
    MODE_DOT,
    MODE_COUNT,
    MODE_LINE,
} plot_t;

typedef struct {
    bool log_x;
    bool log_y;
    bool log_count;
    bool flip_xy;
    bool x_column;
    plot_t mode;
    bool axis;
    bool stream_mode;
    bool colorblind;
    bool regression;
    size_t width;
    size_t height;
    char *in_path;
    FILE *in;
    output_t plot_type;

    struct svg_theme *svg_theme;
} config;

#endif
