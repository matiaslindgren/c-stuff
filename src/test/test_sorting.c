#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "stufflib_argv.h"
#include "stufflib_rand.h"
#include "stufflib_sort.h"

enum sort_type { quick, merge, num_sort_types };

int main(int argc, char* const argv[argc + 1]) {
  int verbose = stufflib_argv_parse_flag(argc, argv, "-v");

  const size_t num_tests_per_size = 5;
  const size_t array_sizes[] = {1, 2, 10, 1000, 10000, 100000, 1000000};
  const char* const sort_types[num_sort_types] = {
      [quick] = "quick",
      [merge] = "merge",
  };
  const stufflib_sort_function sort_funcs[num_sort_types] = {
      [quick] = stufflib_sort_quick,
      [merge] = stufflib_sort_merge,
  };

  if (verbose) {
    printf("%7s %5s %8s %6s\n", "func_t", "test", "n", "msec");
  }
  for (size_t f = 0; f < sizeof(sort_types) / sizeof(*sort_types); ++f) {
    for (size_t s = 0; s < sizeof(array_sizes) / sizeof(*array_sizes); ++s) {
      const size_t n = array_sizes[s];

      double* x = malloc(n * sizeof(double));
      if (!x) {
        fprintf(stderr, "failed allocating memory for test data\n");
        goto error;
      }

      for (size_t test = 0; test < num_tests_per_size; ++test) {
        stufflib_rand_fill(n, x, 1e6);
        stufflib_rand_set_zero(n, x, 0.01);

        const stufflib_sort_function sort = sort_funcs[f];
        clock_t start_time = clock();
        if (!sort(n, x)) {
          fprintf(stderr, "failed allocating working memory\n");
          free(x);
          goto error;
        }
        clock_t end_time = clock();
        double sort_msec =
            1e3 * ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

        if (!stufflib_sort_is_sorted(n, x)) {
          fprintf(stderr, "result is not sorted\n");
          free(x);
          goto error;
        }

        if (verbose) {
          printf("%7s %5zu %8zu %6.1f\n",
                 sort_types[f],
                 test + 1,
                 n,
                 sort_msec);
        }
      }

      free(x);
    }
  }

  return EXIT_SUCCESS;

error:
  return EXIT_FAILURE;
}
