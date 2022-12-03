#include <assert.h>
#include <stdlib.h>

#include "stufflib.h"

double linalg_dot(const size_t n, const double v1[n], const double v2[n]) {
  double dot = 0;
  for (size_t i = 0; i < n; ++i) {
    dot += v1[i] * v2[i];
  }
  return dot;
}

void linalg_matmul(const size_t nrows, const size_t ncols, const double m[nrows][ncols],
                   const double v[ncols], double out[nrows]) {
  for (size_t row = 0; row < nrows; ++row) {
    out[row] = linalg_dot(ncols, m[row], v);
  }
}

int main() {
  const double cmp_tolerance = 1e-16;

  const double v1[] = {1, 2, 3, 4};
  const double v2[] = {0, -2, 4, -6};
  const size_t n = sizeof(v1) / sizeof(v1[0]);
  assert(stufflib_double_almost(linalg_dot(n, v1, v2), -16, cmp_tolerance));

  double m[3][4] = {
      {1, 2, 3, 4},
      {5},
      {9, 10, 11, 12},
  };
  double res[3] = {0};
  linalg_matmul(3, 4, m, v2, res);
  assert(stufflib_double_almost(res[0], -16, cmp_tolerance));
  assert(stufflib_double_almost(res[1], 0, cmp_tolerance));
  assert(stufflib_double_almost(res[2], -48, cmp_tolerance));
  return 0;
}
