#ifndef SL_MATH_H_INCLUDED
#define SL_MATH_H_INCLUDED

typedef double sl_math_function(double);

double sl_math_diff(sl_math_function* f, double x);

bool sl_math_double_almost(const double lhs,
                           const double rhs,
                           const double eps);

double sl_math_clamp(const double lo, const double mid, const double hi);

double sl_math_inv(const double x);

size_t sl_math_next_power_of_two(const size_t x1);

bool sl_math_is_prime(size_t x);

size_t sl_math_next_prime(size_t x);

size_t* sl_math_factorize(size_t n);

double* sl_math_linalg_scalar_vmul(const double a,
                                   const size_t n,
                                   double dst[n],
                                   const double src[n]);

double* sl_math_linalg_vadd(const size_t n,
                            double dst[n],
                            const double lhs[n],
                            const double rhs[n]);

double* sl_math_linalg_vsub(const size_t n,
                            double dst[n],
                            const double lhs[n],
                            const double rhs[n]);

void sl_math_linalg_vadd_inplace(const size_t n,
                                 double dst[n],
                                 const double src[n]);

double sl_math_linalg_norm2(const size_t n, const double v[n]);

double sl_math_linalg_dot(const size_t n,
                          const double v1[n],
                          const double v2[n]);

void sl_math_linalg_matmul(const size_t nrows,
                           const size_t ncols,
                           const double m[nrows][ncols],
                           const double v[ncols],
                           double out[nrows]);

bool sl_math_is_finite(const int count, const float values[restrict count]);

#endif  // SL_MATH_H_INCLUDED
