#include <assert.h>
#include <math.h>
#include <stdlib.h>

double linalg_dot(size_t n, double v1[n], double v2[n]) {
  double dot = 0;
  for (size_t i = 0; i < n; ++i) {
    dot += v1[i] * v2[i];
  }
  return dot;
}

void linalg_matmul(size_t nrows, size_t ncols, double m[nrows][ncols], double v[ncols],
                   double out[nrows]) {
  for (size_t row = 0; row < nrows; ++row) {
    out[row] = linalg_dot(ncols, m[row], v);
  }
}

int _linalg_is_close(double lhs, double rhs, double tolerance) {
  return fabs(lhs - rhs) < tolerance;
}

int main() {
  double v1[] = {1, 2, 3, 4};
  double v2[] = {0, -2, 4, -6};
  size_t n = sizeof(v1) / sizeof(v1[0]);
  assert(_linalg_is_close(linalg_dot(n, v1, v2), -16, 1e-16));

  double m[3][4] = {
      {1, 2, 3, 4},
      {5, 0, 0, 0},
      {9, 10, 11, 12},
  };
  double res[3] = {0};
  linalg_matmul(3, 4, m, v2, res);
  assert(_linalg_is_close(res[0], -16, 1e-16));
  assert(_linalg_is_close(res[1], 0, 1e-16));
  assert(_linalg_is_close(res[2], -48, 1e-16));
  return 0;
}
