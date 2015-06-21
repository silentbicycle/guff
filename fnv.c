#include "fnv.h"

/* Fowler/Noll/Vo hash, 64-bit FNV-1a.
 * This hashing algorithm is in the public domain.
 * For more details, see: http://www.isthe.com/chongo/tech/comp/fnv/. */

static const uint64_t fnv64_prime = 1099511628211L;
static const uint64_t fnv64_offset_basis = 14695981039346656037UL;

uint64_t fnv1a(uint8_t *buf, size_t buf_size) {
    uint64_t h = fnv64_offset_basis;
    for (size_t i = 0; i < buf_size; i++) {
        h = (h ^ buf[i]) * fnv64_prime;
    }
    return h;
}
