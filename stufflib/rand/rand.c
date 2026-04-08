#include <math.h>
#include <stdlib.h>
#include <stufflib/misc/misc.h>
#include <stufflib/rand/rand.h>
#include <time.h>

void sl_rand_seed(unsigned int seed) {
  srand(seed);
}

void sl_rand_fill_double(const size_t n, double dst[n], double scale) {
  for (size_t i = 0; i < n; ++i) {
    double r = rand() - RAND_MAX / 2;
    dst[i]   = scale * (r / RAND_MAX);
  }
}

void sl_rand_set_zero_double(const size_t n, double dst[n], double probability) {
  probability = fmax(0, fmin(1, probability));
  for (size_t i = 0; i < n; ++i) {
    if ((1.0 * rand() / RAND_MAX) < probability) {
      dst[i] = 0;
    }
  }
}

// https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
// 2024-06-14
void sl_rand_shuffle(void* data, const size_t size, const size_t count) {
  if (size <= 0 || count < 2) {
    return;
  }
  unsigned char* v = data;
  for (size_t i = 0; i < count - 1; ++i) {
    const size_t j = sl_rand_int(i, count);
    sl_misc_swap(v + (i * size), v + (j * size), size);
  }
}

void sl_rand_shuffle_together(
    void* data1,
    void* data2,
    const size_t size1,
    const size_t size2,
    const size_t count
) {
  // sl_rand_shuffle but with two arrays of same length
  if (size1 <= 0 || size2 <= 0 || count < 2) {
    return;
  }
  unsigned char* v1 = data1;
  unsigned char* v2 = data2;
  for (size_t i = 0; i < count - 1; ++i) {
    const size_t j = sl_rand_int(i, count);
    sl_misc_swap(v1 + (i * size1), v1 + (j * size1), size1);
    sl_misc_swap(v2 + (i * size2), v2 + (j * size2), size2);
  }
}
