#include <assert.h>
#include <stdlib.h>

#include "stufflib_math.h"

int main(void) {
  const double cmp_eps = 1e-16;

  const double v1[] = {1, 2, 3, 4};
  const double v2[] = {0, -2, 4, -6};
  const size_t n = sizeof(v1) / sizeof(v1[0]);
  const double dot = stufflib_math_linalg_dot(n, v1, v2);
  assert(stufflib_math_double_almost(dot, -16, cmp_eps));

  double m[3][4] = {
      {1, 2, 3, 4},
      {5},
      {9, 10, 11, 12},
  };
  double res[3] = {0};
  stufflib_math_linalg_matmul(3, 4, m, v2, res);
  assert(stufflib_math_double_almost(res[0], -16, cmp_eps));
  assert(stufflib_math_double_almost(res[1], 0, cmp_eps));
  assert(stufflib_math_double_almost(res[2], -48, cmp_eps));

  return EXIT_SUCCESS;
}
