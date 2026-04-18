// implements the permuted congruental generator with xorshift high bits and random rotation
// (PCG-XSH-RR)
// references
// 1. https://en.wikipedia.org/wiki/Permuted_congruential_generator#Example_code
// 2. https://www.pcg-random.org/pdf/hmc-cs-2014-0905.pdf
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <stufflib/context/context.h>
#include <stufflib/io/io.h>
#include <stufflib/misc/misc.h>
#include <stufflib/random/random.h>

bool sl_random_read_device_seed(struct sl_context ctx[static const 1], uint64_t out[static 1]) {
  return sl_io_read(ctx, "/dev/urandom", (unsigned char*)out, 8);
}

static uint64_t const SL_RANDOM_PCG_MULTIPLIER = 6'364'136'223'846'793'005UL;
static uint64_t const SL_RANDOM_PCG_INCREMENT  = 1'442'695'040'888'963'407UL;

uint32_t sl_random_pcg32(uint64_t state[const static 1]) {
  uint64_t x     = *state;
  unsigned count = (unsigned)(x >> 59);  // 59 = 64 - 5
  *state         = (x * SL_RANDOM_PCG_MULTIPLIER) + SL_RANDOM_PCG_INCREMENT;
  x ^= x >> 18;                                             // 18 = (64 - 27)/2
  return sl_random_pcg_rotate((uint32_t)(x >> 27), count);  // 27 = 32 - 5
}

void sl_random_pcg32_init(uint64_t state[const static 1], uint64_t seed) {
  *state = seed + SL_RANDOM_PCG_INCREMENT;
  (void)sl_random_pcg32(state);
}

size_t sl_random_int(uint64_t state[const static 1], size_t a, size_t b) {
  // return random integer i such that a <= i < b
  if (a >= b) {
    return a;
  }
  uint32_t rand_val = sl_random_pcg32(state);
  return a + (size_t)(rand_val % (uint32_t)(b - a));
}

void sl_random_fill_double(
    uint64_t state[const static 1],
    const size_t n,
    double dst[n],
    double scale
) {
  for (size_t i = 0; i < n; ++i) {
    uint32_t rand_val = sl_random_pcg32(state);
    double r          = (double)rand_val / (double)UINT32_MAX;
    dst[i]            = scale * (2.0 * r - 1.0);
  }
}

void sl_random_set_zero_double(
    uint64_t state[const static 1],
    const size_t n,
    double dst[n],
    double probability
) {
  probability = fmax(0, fmin(1, probability));
  for (size_t i = 0; i < n; ++i) {
    uint32_t rand_val = sl_random_pcg32(state);
    if ((double)rand_val / (double)UINT32_MAX < probability) {
      dst[i] = 0;
    }
  }
}

// https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
// 2024-06-14
void sl_random_shuffle(
    uint64_t state[const static 1],
    void* data,
    const size_t size,
    const size_t count
) {
  if (size <= 0 || count < 2) {
    return;
  }
  unsigned char* v = data;
  for (size_t i = 0; i < count - 1; ++i) {
    const size_t j = sl_random_int(state, i, count);
    sl_misc_swap(v + (i * size), v + (j * size), size);
  }
}

void sl_random_shuffle_together(
    uint64_t state[const static 1],
    void* data1,
    void* data2,
    const size_t size1,
    const size_t size2,
    const size_t count
) {
  // sl_random_shuffle but with two arrays of same length
  if (size1 <= 0 || size2 <= 0 || count < 2) {
    return;
  }
  unsigned char* v1 = data1;
  unsigned char* v2 = data2;
  for (size_t i = 0; i < count - 1; ++i) {
    const size_t j = sl_random_int(state, i, count);
    sl_misc_swap(v1 + (i * size1), v1 + (j * size1), size1);
    sl_misc_swap(v2 + (i * size2), v2 + (j * size2), size2);
  }
}
