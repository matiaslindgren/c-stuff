#ifndef SL_MATH_H_INCLUDED
#define SL_MATH_H_INCLUDED
#include <math.h>
#include <stdlib.h>

#include "stufflib_memory.h"

typedef double sl_math_function(double);

double sl_math_diff(sl_math_function* f, double x) {
  const double h = 0.0001;
  return (f(x + h) - f(x)) / h;
}

bool sl_math_double_almost(const double lhs,
                           const double rhs,
                           const double eps) {
  // TODO improve, test around 0 within [-eps, eps]
  return fabs(lhs - rhs) < eps;
}

double sl_math_clamp(const double lo, const double mid, const double hi) {
  return fmin(hi, fmax(lo, mid));
}

double sl_math_inv(const double x) { return 1.0 / x; }

size_t sl_math_next_power_of_two(const size_t x1) {
  size_t x2 = 1;
  while (x2 <= x1) {
    x2 *= 2;
  }
  return x2;
}

bool sl_math_is_prime(size_t x) {
  for (size_t i = 2; i * i < x; ++i) {
    if (x % i == 0) {
      return false;
    }
  }
  return true;
}

size_t sl_math_next_prime(size_t x) {
  for (size_t p = x + 1;; ++p) {
    if (sl_math_is_prime(p)) {
      return p;
    }
  }
}

size_t* sl_math_factorize(size_t n) {
  if (n == 0 || n == 1) {
    return nullptr;
  }

  size_t capacity = 4;
  size_t* factors = sl_alloc(capacity, sizeof(size_t));

  size_t num_factors = 0;
  size_t k = sl_math_next_prime(1);
  while (k <= n) {
    if (n % k == 0) {
      if (num_factors >= capacity) {
        factors = sl_realloc(factors, capacity, 2 * capacity, sizeof(size_t));
        capacity *= 2;
      }
      factors[num_factors] = k;
      ++num_factors;
      n /= k;
    } else {
      k = sl_math_next_prime(k);
    }
  }

  factors = sl_realloc(factors, capacity, num_factors + 1, sizeof(size_t));
  factors[num_factors] = 0;
  return factors;
}

double* sl_math_linalg_scalar_vmul(const double a,
                                   const size_t n,
                                   double dst[n],
                                   const double src[n]) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = a * src[i];
  }
  return dst;
}

double* sl_math_linalg_vadd(const size_t n,
                            double dst[n],
                            const double lhs[n],
                            const double rhs[n]) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = lhs[i] + rhs[i];
  }
  return dst;
}

double* sl_math_linalg_vsub(const size_t n,
                            double dst[n],
                            const double lhs[n],
                            const double rhs[n]) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = lhs[i] - rhs[i];
  }
  return dst;
}

void sl_math_linalg_vadd_inplace(const size_t n,
                                 double dst[n],
                                 const double src[n]) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] += src[i];
  }
}

double sl_math_linalg_norm2(const size_t n, const double v[n]) {
  double s = 0;
  for (size_t i = 0; i < n; ++i) {
    s += v[i] * v[i];
  }
  return sqrt(s);
}

double sl_math_linalg_dot(const size_t n,
                          const double v1[n],
                          const double v2[n]) {
  double dot = 0;
  for (size_t i = 0; i < n; ++i) {
    dot += v1[i] * v2[i];
  }
  return dot;
}

void sl_math_linalg_matmul(const size_t nrows,
                           const size_t ncols,
                           const double m[nrows][ncols],
                           const double v[ncols],
                           double out[nrows]) {
  for (size_t row = 0; row < nrows; ++row) {
    out[row] = sl_math_linalg_dot(ncols, m[row], v);
  }
}

static inline bool sl_math_is_finite(const int count,
                                     const float values[restrict count]) {
  for (int i = 0; i < count; ++i) {
    if (!isfinite(values[i])) {
      return false;
    }
  }
  return true;
}

#endif  // SL_MATH_H_INCLUDED
