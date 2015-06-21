#ifndef GUFF_H
#define GUFF_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>

#include <stdint.h>
#include <stdbool.h>

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <math.h>

#include "types.h"

/* Version 0.1.0. */
#define GUFF_VERSION_MAJOR 0
#define GUFF_VERSION_MINOR 1
#define GUFF_VERSION_PATCH 0
#define GUFF_AUTHOR "Scott Vokes <vokes.s@gmail.com>"

/* Include the axes if they are within CROSS_PAD * the range from the
 * min or max values, otherwise omit them..*/
#define CROSS_PAD 2.0

#define EMPTY_VALUE NAN
#define IS_EMPTY(V) isnan(V)
#define IS_EMPTY_POINT(P) (IS_EMPTY(P->x) || IS_EMPTY(P->y))

#ifdef DEBUG
#define LOG(LVL, ...)                                                  \
    do {                                                               \
        if (DEBUG >= LVL) {                                            \
            fprintf(stderr, __VA_ARGS__);                              \
        }                                                              \
    } while(0)
#else
#define LOG(_, ...)
#endif

#define MAX_COLUMNS 255

#endif
