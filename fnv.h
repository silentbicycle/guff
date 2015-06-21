#ifndef FNV_H
#define FNV_H

#include <stdlib.h>
#include <stdint.h>

/* Fowler/Noll/Vo hash, 64-bit FNV-1a.
 * This hashing algorithm is in the public domain.
 * For more details, see: http://www.isthe.com/chongo/tech/comp/fnv/. */
uint64_t fnv1a(uint8_t *buf, size_t buf_size);

#endif
