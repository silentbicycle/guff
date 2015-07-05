/* 
 * Copyright (c) 2015 Scott Vokes <vokes.s@gmail.com>
 *  
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "guff.h"
#include "args.h"
#include "input.h"
#include "draw.h"

static void read_env(config *cfg) {
    if (getenv("GUFF_FLIP")) { cfg->flip_xy = true; }

    char *value = getenv("GUFF_WIDTH");
    if (value) { cfg->width = atoi(value); }
    value = getenv("GUFF_HEIGHT");
    if (value) { cfg->height = atoi(value); }
}

int main(int argc, char **argv) {
    config cfg = {
        .axis = true,
        .in = stdin,
        .stream_mode = true,
    };

    read_env(&cfg);

    args_handle(&cfg, argc, argv);

    bool end_of_stream = false;
    bool has_rendered = false;

    if(isatty(fileno(cfg.in))) {
        fprintf(stderr, " -- Reading from stdin. Use `guff -h` for usage info, ^D to exit.\n");
    }

    while (!end_of_stream) {
        data_set ds = { .pairs = NULL };
        int res = input_read(&cfg, &ds);

        if (res == -1) {
            end_of_stream = true;
        } else if (res != 0) {
            input_free(&ds);
            return res;
        }

        /* Explicitly allow a blank line before the first graph. */
        if (ds.rows == 0 && has_rendered) {  // no input
            input_free(&ds);
            return 0;
        }
        
        res = draw(&cfg, &ds);
        input_free(&ds);
        has_rendered = true;
        if (res != 0) { return res; }

        if (!end_of_stream) { printf("\n"); }
    }

    if (cfg.svg_theme) { free(cfg.svg_theme); }
    
    return 0;
}
