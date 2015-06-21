#ifndef COUNTER_H
#define COUNTER_H

#include "guff.h"

typedef struct counter counter;

/* Init a counter table with sufficient space for ROWS cells. */
counter *counter_init(size_t rows);

void counter_increment(counter *c, size_t x, size_t y);

size_t counter_get(counter *c, size_t x, size_t y);

void counter_free(counter *c);

#endif
