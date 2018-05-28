#include "args.h"

#include <getopt.h>

#include "svg.h"

/* CLI argument handling. */

static void init_ascii(config *cfg);
static void init_svg(config *cfg);

static void usage(const char *msg) {
    if (msg) { fprintf(stderr, "%s\n\n", msg); }
    fprintf(stderr, "guff v. %d.%d.%d by %s\n",
        GUFF_VERSION_MAJOR, GUFF_VERSION_MINOR,
        GUFF_VERSION_PATCH, GUFF_AUTHOR);
    fprintf(stderr,
        "\n"
        "Usage: guff [-A] [-c] [-d WxH] [-f] [-h] [-l xyc]\n"
        "            [-m MODE] [-r] [-s] [-S] [-x] [FILE]\n"
        "\n"
        "Common options:\n"
        "    -b: render to UTF-8 braille characters\n"
        "    -d WxH: set width and height (e.g. \"-d 72x40\", \"-d 640x480\")\n"
        "    -f: flip x & y axes in plot\n"
        "    -h: print this message\n"
        "    -l LOG: any of 'x', 'y', 'c' -- set X, Y, and/or count to log scale\n"
        "    -m MODE: dot, count, line (SVG only), default dot\n"
        "    -s: render to SVG\n"
        "    -x: treat first column as X for all following Y columns (def: use row count)\n"
        "\n"
        "SVG only:\n"
        "    -c: use colorblind-safe default colors\n"
        "    -r: draw linear regression lines\n"
        "\n"
        "Other options:\n"
        "    -A: don't draw axes\n"
        "    -S: disable stream mode\n"
        );
    exit(1);
}

static void parse_dims(config *cfg, const char *opt) {
    char *x = strchr(opt, 'x');
    if (x) {
        cfg->width = atoi(opt);
        x++;
        cfg->height = atoi(x);
    } else {
        usage("Bad -d argument, should be formatted like -d 72x40");
    }
}

void args_handle(config *cfg, int argc, char **argv) {
    int fl;
    while ((fl = getopt(argc, argv, "Abcd:fhl:m:rsSx")) != -1) {
        switch (fl) {
        case 'A':               /* no axis */
            cfg->axis = false;
            break;
        case 'b':               /* braille */
            cfg->plot_type = PLOT_BRAILLE;
            break;
        case 'c':               /* use colorblind-safe default colors */
            cfg->colorblind = true;
            break;
        case 'd':               /* dimensions */
            parse_dims(cfg, optarg);
            break;
        case 'f':               /* flip x/y */
            cfg->flip_xy = true;
            break;
        case 'h':               /* help */
            usage(NULL);
            break;
        case 'l':               /* log */
            if (strchr(optarg, 'x')) { cfg->log_x = true; }
            if (strchr(optarg, 'y')) { cfg->log_y = true; }
            if (strchr(optarg, 'c')) { cfg->log_count = true; }
            break;
        case 'm':               /* mode */
            switch (optarg[0]) {
            case 'c':
                cfg->mode = MODE_COUNT;
                break;
            case 'd':
                cfg->mode = MODE_DOT;
                break;
            case 'l':
                cfg->mode = MODE_LINE;
                break;
            default:
                usage("Bad argument to -m: must be 'count', 'dot', or 'line'.");
            }
            break;
        case 'r':               /* linear regression */
            cfg->regression = true;
            break;
        case 's':               /* SVG */
            cfg->plot_type = PLOT_SVG;
            break;
        case 'S':               /* disable stream mode */
            cfg->stream_mode = false;
            break;
        case 'x':               /* col 0 is X value */
            cfg->x_column = true;
            break;
        case '?':
        default:
            usage(NULL);
        }
    }

    argc -= (optind - 1);
    argv += (optind - 1);
    if (argc > 1) {
        cfg->in_path = argv[1];
        if (0 != strcmp("-", cfg->in_path)) {
            cfg->in = fopen(cfg->in_path, "r");
            if (cfg->in == NULL) { err(1, "fopen"); }
        }
    }

    if (cfg->plot_type == PLOT_SVG) {
        init_svg(cfg);
    } else {
        init_ascii(cfg);
    }
}

static void init_ascii(config *cfg) {
    if (cfg->width == 0) { cfg->width = 72; }
    if (cfg->height == 0) { cfg->height = 40; }
}

static char *read_env_var(char *name) {
    char *v = getenv(name);
    if (v) {
        char *quote = strchr(v, '"');
        if (quote) { *quote = '\0'; }
    }
    return v;
}

/* Colors chosen using http://colorbrewer2.org/ ,
 * qualitative color scheme, 9 data classes, not colorblind-safe. */
static char *default_colors[] = {
    "#377eb8",
    "#e41a1c",
    "#4daf4a",
    "#984ea3",
    "#ff7f00",
    "#ffff33",
    "#a65628",
    "#f781bf",
    "#999999",
};

/* Colors chosen using http://colorbrewer2.org/ ,
 * diverging color scheme, 9 data classes, colorblind-safe. */
static char *default_colorblind_safe[] = {
    "#762a83",
    "#9970ab",
    "#c2a5cf",
    "#e7d4e8",
    "#f7f7f7",
    "#d9f0d3",
    "#a6dba0",
    "#5aae61",
    "#1b7837",
};

static void init_svg(config *cfg) {
    svg_theme *theme = calloc(1, sizeof(*theme));
    assert(theme);
    if (cfg->width == 0) { cfg->width = 320; }
    if (cfg->height == 0) { cfg->height = 200; }


#define DEF_STR_OPTION(VAR, ENV_VAR, DEFAULT)                           \
    do {                                                                \
        theme->VAR = read_env_var("GUFF_" ENV_VAR);                     \
        if (theme->VAR == NULL) { theme->VAR = DEFAULT; }               \
    } while (0)

#define DEF_INT_OPTION(VAR, ENV_VAR, DEFAULT)                           \
    do {                                                                \
        char *var = read_env_var("GUFF_" ENV_VAR);                      \
        if (var == NULL) { var = DEFAULT; }                             \
        theme->VAR = atol(var);                                         \
    } while (0)

    DEF_STR_OPTION(bg_color, "BG_COLOR", "black");
    DEF_STR_OPTION(border_color, "BORDER_COLOR", "black");
    DEF_STR_OPTION(axis_color, "AXIS_COLOR", "lightgray");

    char **palette = cfg->colorblind ? default_colorblind_safe : default_colors;

    DEF_STR_OPTION(colors[0], "COLOR0", palette[0]);
    DEF_STR_OPTION(colors[1], "COLOR1", palette[1]);
    DEF_STR_OPTION(colors[2], "COLOR2", palette[2]);
    DEF_STR_OPTION(colors[3], "COLOR3", palette[3]);
    DEF_STR_OPTION(colors[4], "COLOR4", palette[4]);
    DEF_STR_OPTION(colors[5], "COLOR5", palette[5]);
    DEF_STR_OPTION(colors[6], "COLOR6", palette[6]);
    DEF_STR_OPTION(colors[7], "COLOR7", palette[7]);
    DEF_STR_OPTION(colors[8], "COLOR8", palette[8]);

    DEF_INT_OPTION(border_width, "BORDER_WIDTH", "2");
    DEF_INT_OPTION(line_width, "LINE_WIDTH", "2");
    DEF_INT_OPTION(axis_width, "AXIS_WIDTH", "2");

    cfg->svg_theme = theme;
}
