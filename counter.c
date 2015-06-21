#include "counter.h"
#include "fnv.h"

/* Hash table, for tracking point counts. */

typedef struct {
    size_t x;
    size_t y;
    size_t count;
} bucket;

struct counter {
    uint8_t bucket_ceil2;
    size_t bucket_count;
    bucket buckets[];
};

#define EMPTY_BUCKET ((size_t)-1)

/* Alloc a hash table with 2*ceil(rows) buckets. */
counter *counter_init(size_t rows) {
    size_t ceil = rows;
    uint8_t ceil2 = 2;
    while ((1 << ceil2) < ceil) { ceil2++; }
    ceil2++;
    size_t bucket_count = 1 << ceil2;

    size_t size = sizeof(counter) + bucket_count * sizeof(bucket);
    counter *c = malloc(size);
    if (c) {
        c->bucket_ceil2 = ceil2;
        c->bucket_count = bucket_count;
        for (size_t i = 0; i < bucket_count; i++) {
            c->buckets[i].x = EMPTY_BUCKET;
            c->buckets[i].y = EMPTY_BUCKET;
        }
    }
    return c;
}

size_t point_hash(size_t x, size_t y) {
    size_t buf_size = 2*sizeof(size_t);
    uint8_t buf[buf_size];
    memcpy(&buf[0], &x, sizeof(x));
    memcpy(&buf[sizeof(x)], &y, sizeof(y));
    return (size_t)fnv1a(buf, buf_size);
}

static bucket *find_bucket(counter *c, size_t x, size_t y) {
    size_t hash = point_hash(x, y);
    size_t mask = c->bucket_count - 1;
    for (size_t i = 0; i < c->bucket_count; i++) {
        size_t bi = (hash + i) & mask;
        bucket *b = &c->buckets[bi];
        if (b->x == x && b->y == y) {
            return b;
        } else if (b->x == EMPTY_BUCKET) {
            b->x = x;
            b->y = y;
            b->count = 0;
            return b;
        }
    }
    return NULL;
}

void counter_increment(counter *c, size_t x, size_t y) {
    bucket *b = find_bucket(c, x, y);
    assert(b);
    b->count++;
}

size_t counter_get(counter *c, size_t x, size_t y) {
    bucket *b = find_bucket(c, x, y);
    assert(b);
    return b->count;
}

void counter_free(counter *c) {
    free(c);
}
