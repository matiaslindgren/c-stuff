#ifndef SL_MATH_H_INCLUDED
#define SL_MATH_H_INCLUDED

#include <stddef.h>
#include <stufflib/context/context.h>
#include <stufflib/memory/memory.h>

typedef double sl_math_function(double);

double sl_math_diff(sl_math_function* f, double x);
bool sl_math_double_almost(double lhs, double rhs, double eps);
double sl_math_clamp(double lo, double mid, double hi);
double sl_math_inv(double x);
size_t sl_math_next_power_of_two(size_t x1);
bool sl_math_is_prime(size_t x);
size_t sl_math_next_prime(size_t x);
size_t* sl_math_factorize(struct sl_context ctx[static 1], size_t n);
double* sl_math_linalg_scalar_vmul(double a, size_t n, double dst[n], const double src[n]);
double* sl_math_linalg_vadd(size_t n, double dst[n], const double lhs[n], const double rhs[n]);
double* sl_math_linalg_vsub(size_t n, double dst[n], const double lhs[n], const double rhs[n]);
void sl_math_linalg_vadd_inplace(size_t n, double dst[n], const double src[n]);
double sl_math_linalg_norm2(size_t n, const double v[n]);
double sl_math_linalg_dot(size_t n, const double v1[n], const double v2[n]);
void sl_math_linalg_matmul(
    size_t nrows,
    size_t ncols,
    const double m[nrows][ncols],
    const double v[ncols],
    double out[nrows]
);
bool sl_math_is_finite(int count, const float values[restrict count]);

#endif  // SL_MATH_H_INCLUDED
