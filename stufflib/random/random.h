#ifndef SL_RANDOM_H_INCLUDED
#define SL_RANDOM_H_INCLUDED

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>

void sl_random_seed(unsigned int seed);
void sl_random_fill_double(size_t n, double dst[n], double scale);
void sl_random_set_zero_double(size_t n, double dst[n], double probability);
void sl_random_shuffle(void* data, size_t size, size_t count);
void sl_random_shuffle_together(void* data1, void* data2, size_t size1, size_t size2, size_t count);

static inline size_t sl_random_int(const size_t a, const size_t b) {
  // return random integer i such that a <= i < b
  if ((a >= b) || ((b - a) > (size_t)INT_MAX)) {
    return a;
  }
  int rand_val = rand();
  return a + (size_t)(rand_val % (int)(b - a));
}

#endif  // SL_RANDOM_H_INCLUDED
