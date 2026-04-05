#ifndef SL_RAND_H_INCLUDED
#define SL_RAND_H_INCLUDED

#include <stddef.h>
#include <stdlib.h>

#include <stufflib/misc/misc.h>

void sl_rand_fill_double(const size_t n, double dst[n], double scale);
void sl_rand_set_zero_double(const size_t n, double dst[n], double probability);
void sl_rand_shuffle(void* data, const size_t size, const size_t count);
void sl_rand_shuffle_together(
    void* data1,
    void* data2,
    const size_t size1,
    const size_t size2,
    const size_t count
);

static inline size_t sl_rand_int(const size_t a, const size_t b) {
  // return random integer i such that a <= i < b
  if (a >= b) {
    return a;
  }
  return a + (size_t)(rand() % (int)(b - a));
}

#endif  // SL_RAND_H_INCLUDED
