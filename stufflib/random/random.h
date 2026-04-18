#ifndef SL_RANDOM_H_INCLUDED
#define SL_RANDOM_H_INCLUDED

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <stufflib/context/context.h>

static inline uint32_t sl_random_pcg_rotate(uint32_t x, unsigned r) {
  return x >> r | x << (-r & 31);
}

bool sl_random_read_device_seed(struct sl_context ctx[static 1], uint64_t out[static 1]);
uint32_t sl_random_pcg32(uint64_t state[static 1]);
void sl_random_pcg32_init(uint64_t state[static 1], uint64_t seed);
size_t sl_random_int(uint64_t state[static 1], size_t a, size_t b);
void sl_random_fill_double(uint64_t state[static 1], size_t n, double dst[n], double scale);
void sl_random_set_zero_double(
    uint64_t state[static 1],
    size_t n,
    double dst[n],
    double probability
);
void sl_random_shuffle(uint64_t state[static 1], void* data, size_t size, size_t count);
void sl_random_shuffle_together(
    uint64_t state[static 1],
    void* data1,
    void* data2,
    size_t size1,
    size_t size2,
    size_t count
);

#endif  // SL_RANDOM_H_INCLUDED
