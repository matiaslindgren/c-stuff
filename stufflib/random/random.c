#include <math.h>
#include <stdlib.h>
#include <stufflib/misc/misc.h>
#include <stufflib/random/random.h>
#include <time.h>

void sl_random_seed(unsigned int seed) {
  srand(seed);
}

void sl_random_fill_double(const size_t n, double dst[n], double scale) {
  for (size_t i = 0; i < n; ++i) {
    int rand_val = rand();
    double r     = (double)rand_val / RAND_MAX;
    dst[i]       = scale * (2.0 * r - 1.0);
  }
}

void sl_random_set_zero_double(const size_t n, double dst[n], double probability) {
  probability = fmax(0, fmin(1, probability));
  for (size_t i = 0; i < n; ++i) {
    if ((1.0 * rand() / RAND_MAX) < probability) {
      dst[i] = 0;
    }
  }
}

// https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
// 2024-06-14
void sl_random_shuffle(void* data, const size_t size, const size_t count) {
  if (size <= 0 || count < 2) {
    return;
  }
  unsigned char* v = data;
  for (size_t i = 0; i < count - 1; ++i) {
    const size_t j = sl_random_int(i, count);
    sl_misc_swap(v + (i * size), v + (j * size), size);
  }
}

void sl_random_shuffle_together(
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
    const size_t j = sl_random_int(i, count);
    sl_misc_swap(v1 + (i * size1), v1 + (j * size1), size1);
    sl_misc_swap(v2 + (i * size2), v2 + (j * size2), size2);
  }
}
