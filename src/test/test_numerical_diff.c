#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "stufflib_argv.h"
#include "stufflib_math.h"
#include "stufflib_rand.h"

double pow2(double x) { return pow(x, 2); }

int main(int argc, char* const argv[argc + 1]) {
  int verbose = stufflib_argv_parse_flag(argc, argv, "-v");

  const size_t n = 10000;
  double x[n];
  stufflib_rand_fill(n, x, 10);
  stufflib_rand_set_zero(n, x, 0.01);

  if (verbose) {
    printf("%4s %8s %8s %8s %9s %9s %9s %9s\n", "i", "x[i]", "cos(x)", "sin(x)", "2x", "cos'(x)",
           "sin'(x)", "pow2'(x)");
  }
  for (size_t i = 0; i < n; ++i) {
    double cos_x = cos(x[i]);
    double sin_x = sin(x[i]);
    double d_cos_x = stufflib_math_diff(cos, x[i]);
    double d_sin_x = stufflib_math_diff(sin, x[i]);
    double d_pow2_x = stufflib_math_diff(pow2, x[i]);
    if (verbose) {
      printf("%4zd %8.1g %8.3g %8.3g %9.3g %9.3g %9.3g %9.3f\n", i, x[i], cos_x, sin_x, 2 * x[i],
             d_cos_x, d_sin_x, d_pow2_x);
    }
    assert(stufflib_math_double_almost(d_cos_x, -sin_x, 1e-3));
    assert(stufflib_math_double_almost(d_sin_x, cos_x, 1e-3));
    assert(stufflib_math_double_almost(d_pow2_x, 2 * x[i], 1e-3));
  }

  return EXIT_SUCCESS;
}
