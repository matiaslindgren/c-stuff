#ifndef _STUFFLIB_MATH_H_INCLUDED
#define _STUFFLIB_MATH_H_INCLUDED
#include <math.h>
#include <stdlib.h>

typedef double (*stufflib_math_function)(double);

double stufflib_math_diff(stufflib_math_function f, double x) {
  const double h = 0.0001;
  return (f(x + h) - f(x)) / h;
}

int stufflib_math_double_almost(const double lhs,
                                const double rhs,
                                const double eps) {
  // TODO improve, test around 0 within [-eps, eps]
  return fabs(lhs - rhs) < eps;
}

int stufflib_math_is_prime(size_t x) {
  for (size_t i = 2; i * i < x; ++i) {
    if (x % i == 0) {
      return 0;
    }
  }
  return 1;
}

size_t stufflib_math_next_prime(size_t x) {
  for (size_t p = x + 1;; ++p) {
    if (stufflib_math_is_prime(p)) {
      return p;
    }
  }
}

size_t* stufflib_math_factorize(size_t n) {
  if (n == 0 || n == 1) {
    return 0;
  }

  size_t capacity = 4;
  size_t* factors = calloc(capacity, sizeof(size_t));
  if (!factors) {
    return 0;
  }

  size_t num_factors = 0;
  size_t k = stufflib_math_next_prime(1);
  while (k <= n) {
    if (n % k == 0) {
      if (num_factors >= capacity) {
        capacity *= 2;
        {
          size_t* resized = realloc(factors, capacity * sizeof(size_t));
          if (!resized) {
            goto error;
          }
          factors = resized;
        }
      }
      factors[num_factors] = k;
      ++num_factors;
      n /= k;
    } else {
      k = stufflib_math_next_prime(k);
    }
  }

  {
    size_t* resized = realloc(factors, (num_factors + 1) * sizeof(size_t));
    if (!resized) {
      goto error;
    }
    factors = resized;
  }
  factors[num_factors] = 0;

  return factors;

error:
  free(factors);
  return 0;
}

double stufflib_math_linalg_dot(const size_t n,
                                const double v1[n],
                                const double v2[n]) {
  double dot = 0;
  for (size_t i = 0; i < n; ++i) {
    dot += v1[i] * v2[i];
  }
  return dot;
}

void stufflib_math_linalg_matmul(const size_t nrows,
                                 const size_t ncols,
                                 const double m[nrows][ncols],
                                 const double v[ncols],
                                 double out[nrows]) {
  for (size_t row = 0; row < nrows; ++row) {
    out[row] = stufflib_math_linalg_dot(ncols, m[row], v);
  }
}

#endif  // _STUFFLIB_MATH_H_INCLUDED
