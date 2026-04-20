#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <assert.h>

#include <stufflib/context/context.h>
#include <stufflib/linalg/linalg.h>
#include <stufflib/macros/macros.h>
#include <stufflib/math/math.h>
#include <stufflib/matrix/sl_matrix_f32.h>
#include <stufflib/memory/memory.h>
#include <stufflib/vector/sl_vector_f32.h>

#ifdef __APPLE__
  #if __has_include(<Accelerate/Accelerate.h>)
    #define ACCELERATE_NEW_LAPACK
    #include <Accelerate/Accelerate.h>
  #else
    #error "cannot find Apple Accelerate framework"
  #endif
#elifdef __gnu_linux__
  #if __has_include(<cblas.h>)
    #include <cblas.h>
  #else
    #error "cannot find OpenBLAS headers"
  #endif
#else
  #error "no BLAS support available for this platform"
#endif

bool sl_la_vector_create(
    struct sl_context ctx[static 1],
    const size_t size,
    struct sl_vector_f32 out[static 1]
) {
  assert(size > 0);
  float* data = sl_alloc(ctx, size, sizeof(float));
  if (!data) {
    return false;
  }
  *out = (struct sl_vector_f32){
      .data     = data,
      .length   = {size},
      .capacity = {size},
  };
  return true;
}

void sl_la_vector_destroy(struct sl_vector_f32 v[const static 1]) {
  sl_free(v->data);
  *v = (struct sl_vector_f32){0};
}

bool sl_la_matrix_create(
    struct sl_context ctx[static 1],
    const size_t rows,
    const size_t cols,
    struct sl_matrix_f32 out[static 1]
) {
  assert(rows > 0 && cols > 0);
  float* data = sl_alloc(ctx, rows * cols, sizeof(float));
  if (!data) {
    return false;
  }
  *out = (struct sl_matrix_f32){
      .data     = data,
      .length   = {rows, cols},
      .capacity = {rows, cols},
  };
  return true;
}

void sl_la_matrix_destroy(struct sl_matrix_f32 a[const static 1]) {
  sl_free(a->data);
  *a = (struct sl_matrix_f32){0};
}

struct sl_vector_f32
sl_la_matrix_row_view(struct sl_matrix_f32 a[const static 1], const size_t row) {
  return (struct sl_vector_f32){
      .data     = sl_matrix_f32_get(a, row, 0),
      .length   = {sl_matrix_f32_num_cols(a)},
      .capacity = {a->capacity[1]},
  };
}

void sl_la_vec_add(const size_t count, float lhs[restrict count], const float rhs[restrict count]) {
  assert(count <= (size_t)INT32_MAX);
  cblas_saxpy((int)count, 1, rhs, 1, lhs, 1);
}

void sl_la_vec_sub(const size_t count, float lhs[restrict count], const float rhs[restrict count]) {
  assert(count <= (size_t)INT32_MAX);
  cblas_saxpy((int)count, -1, rhs, 1, lhs, 1);
}

void sl_la_vec_mul(const size_t count, float lhs[restrict count], const float rhs[restrict count]) {
  for (size_t i = 0; i < count; ++i) {
    lhs[i] *= rhs[i];
  }
}

void sl_la_vec_min(const size_t count, float lhs[restrict count], const float rhs[restrict count]) {
  for (size_t i = 0; i < count; ++i) {
    lhs[i] = fminf(lhs[i], rhs[i]);
  }
}

void sl_la_vec_max(const size_t count, float lhs[restrict count], const float rhs[restrict count]) {
  for (size_t i = 0; i < count; ++i) {
    lhs[i] = fmaxf(lhs[i], rhs[i]);
  }
}

bool sl_la_vector_is_finite(struct sl_vector_f32 v[const static 1]) {
  assert(sl_vector_f32_size(v) <= (size_t)INT32_MAX);
  return sl_math_is_finite((int)sl_vector_f32_size(v), v->data);
}

void sl_la_vector_scale(struct sl_vector_f32 v[const static 1], const float alpha) {
  assert(sl_vector_f32_size(v) <= (size_t)INT32_MAX);
  cblas_sscal((int)sl_vector_f32_size(v), alpha, v->data, 1);
}

void sl_la_vector_clear(struct sl_vector_f32 v[const static 1]) {
  memset(v->data, 0, sizeof(float) * sl_vector_f32_size(v));
}

float sl_la_vector_dot(
    const struct sl_vector_f32 a[const static 1],
    const struct sl_vector_f32 b[const static 1]
) {
  assert(sl_vector_f32_size(a) == sl_vector_f32_size(b));
  assert(sl_vector_f32_size(a) <= (size_t)INT32_MAX);
  return cblas_sdot((int)sl_vector_f32_size(a), a->data, 1, b->data, 1);
}

void sl_la_vector_add(
    const struct sl_vector_f32 a[const static 1],
    const struct sl_vector_f32 b[const static 1]
) {
  assert(sl_vector_f32_size(a) == sl_vector_f32_size(b));
  sl_la_vec_add(sl_vector_f32_size(a), a->data, b->data);
}

void sl_la_vector_sub(
    const struct sl_vector_f32 a[const static 1],
    const struct sl_vector_f32 b[const static 1]
) {
  assert(sl_vector_f32_size(a) == sl_vector_f32_size(b));
  sl_la_vec_sub(sl_vector_f32_size(a), a->data, b->data);
}

void sl_la_vector_mul(
    struct sl_vector_f32 lhs[const static 1],
    const struct sl_vector_f32 rhs[const static 1]
) {
  assert(sl_vector_f32_size(lhs) == sl_vector_f32_size(rhs));
  sl_la_vec_mul(sl_vector_f32_size(lhs), lhs->data, rhs->data);
}

void sl_la_vector_copy(
    struct sl_vector_f32 dst[const static 1],
    struct sl_vector_f32 src[const static 1]
) {
  assert(sl_vector_f32_size(dst) == sl_vector_f32_size(src));
  assert(sl_vector_f32_size(src) <= (size_t)INT32_MAX);
  cblas_scopy((int)sl_vector_f32_size(src), src->data, 1, dst->data, 1);
}

bool sl_la_vector_print(
    struct sl_context ctx[static 1],
    FILE stream[const static 1],
    const struct sl_vector_f32 v[const static 1]
) {
  for (size_t i = 0; i < sl_vector_f32_size(v); ++i) {
    const double value = (double)v->data[i];
    if (fprintf(stream, (SL_LA_FLOAT_FORMAT " "), value) < 0) {
      SL_ERROR(ctx, "failed printing vector value at (%zu): " SL_LA_FLOAT_FORMAT, i, value);
      return false;
    }
  }
  if (fprintf(stream, "\n") < 0) {
    SL_ERROR(ctx, "failed printing newline");
    return false;
  }
  return true;
}

bool sl_la_matrix_print(
    struct sl_context ctx[static 1],
    FILE stream[const static 1],
    struct sl_matrix_f32 a[const static 1]
) {
  for (size_t row = 0; row < sl_matrix_f32_num_rows(a); ++row) {
    for (size_t col = 0; col < sl_matrix_f32_num_cols(a); ++col) {
      const double value = (double)*sl_matrix_f32_get(a, row, col);
      if (fprintf(stream, (SL_LA_FLOAT_FORMAT " "), value) < 0) {
        SL_ERROR(
            ctx,
            "failed printing matrix value at (%zu, %zu): " SL_LA_FLOAT_FORMAT,
            row,
            col,
            value
        );
        return false;
      }
    }
    if (fprintf(stream, "\n") < 0) {
      SL_ERROR(ctx, "failed printing newline after row %zu", row);
      return false;
    }
  }
  return true;
}

void sl_la_matrix_multiply(
    const struct sl_matrix_f32 a[const static 1],
    const struct sl_matrix_f32 b[const static 1],
    struct sl_matrix_f32 c[const static 1]
) {
  // c := a b
  assert(
      sl_matrix_f32_num_cols(a) == sl_matrix_f32_num_rows(b)
      && sl_matrix_f32_num_rows(a) == sl_matrix_f32_num_rows(c)
      && sl_matrix_f32_num_cols(b) == sl_matrix_f32_num_cols(c)
  );
  assert(sl_matrix_f32_num_rows(a) <= (size_t)INT32_MAX);
  assert(sl_matrix_f32_num_cols(a) <= (size_t)INT32_MAX);
  assert(sl_matrix_f32_num_cols(b) <= (size_t)INT32_MAX);
  // column-major implicit transpose trickery from
  // https://stackoverflow.com/a/56064726/5951112
  // 2024-06-10
  cblas_sgemm(
      CblasColMajor,
      CblasNoTrans,
      CblasNoTrans,
      (int)sl_matrix_f32_num_cols(b), /* m */
      (int)sl_matrix_f32_num_rows(a), /* n */
      (int)sl_matrix_f32_num_cols(a), /* k */
      1,                              /* alpha */
      b->data,                        /* a */
      (int)sl_matrix_f32_num_cols(b), /* lda */
      a->data,                        /* b */
      (int)sl_matrix_f32_num_cols(a), /* ldb */
      0,                              /* beta */
      c->data,                        /* c */
      (int)sl_matrix_f32_num_cols(c)  /* ldc */
  );
}

double sl_la_matrix_trace(struct sl_matrix_f32 a[const static 1]) {
  double tr      = 0;
  const size_t n = SL_MIN(sl_matrix_f32_num_rows(a), sl_matrix_f32_num_cols(a));
  for (size_t i = 0; i < n; ++i) {
    tr += (double)*sl_matrix_f32_get(a, i, i);
  }
  return tr;
}

double sl_la_matrix_frobenius_norm(struct sl_matrix_f32 a[const static 1]) {
  // https://en.wikipedia.org/wiki/Matrix_norm#Frobenius_norm
  // 2024-06-09
  assert(sl_matrix_f32_num_cols(a) <= (size_t)INT32_MAX);
  double norm = 0;
  for (size_t r = 0; r < sl_matrix_f32_num_rows(a); ++r) {
    const float* row = sl_matrix_f32_get(a, r, 0);
    norm += (double)cblas_sdot((int)sl_matrix_f32_num_cols(a), row, 1, row, 1);
  }
  return sqrt(norm);
}

void sl_la_matrix_copy_row(
    struct sl_vector_f32 dst[const static 1],
    struct sl_matrix_f32 src[const static 1],
    const size_t row
) {
  assert(sl_vector_f32_size(dst) == sl_matrix_f32_num_cols(src));
  assert(sl_vector_f32_size(dst) <= (size_t)INT32_MAX);
  cblas_scopy((int)sl_vector_f32_size(dst), sl_matrix_f32_get(src, row, 0), 1, dst->data, 1);
}

void sl_la_matrix_saxpy_axis0(
    struct sl_matrix_f32 m[const static 1],
    struct sl_vector_f32 v[const static 1],
    const float alpha
) {
  assert(sl_matrix_f32_num_cols(m) == sl_vector_f32_size(v));
  assert(sl_matrix_f32_num_cols(m) <= (size_t)INT32_MAX);
  for (size_t row = 0; row < sl_matrix_f32_num_rows(m); ++row) {
    cblas_saxpy((int)sl_matrix_f32_num_cols(m), alpha, v->data, 1, sl_matrix_f32_get(m, row, 0), 1);
  }
}

void sl_la_matrix_add_axis0(
    struct sl_matrix_f32 m[const static 1],
    struct sl_vector_f32 v[const static 1]
) {
  sl_la_matrix_saxpy_axis0(m, v, 1);
}

void sl_la_matrix_sub_axis0(
    struct sl_matrix_f32 m[const static 1],
    struct sl_vector_f32 v[const static 1]
) {
  sl_la_matrix_saxpy_axis0(m, v, -1);
}

void sl_la_matrix_mul_axis0(
    struct sl_matrix_f32 m[const static 1],
    struct sl_vector_f32 v[const static 1]
) {
  assert(sl_matrix_f32_num_cols(m) == sl_vector_f32_size(v));
  for (size_t row = 0; row < sl_matrix_f32_num_rows(m); ++row) {
    sl_la_vec_mul(sl_matrix_f32_num_cols(m), sl_matrix_f32_get(m, row, 0), v->data);
  }
}

void sl_la_matrix_diffdiv_axis0(
    struct sl_matrix_f32 m[const static 1],
    struct sl_vector_f32 lhs[const static 1],
    struct sl_vector_f32 rhs[const static 1]
) {
  assert(
      sl_matrix_f32_num_cols(m) == sl_vector_f32_size(lhs)
      && sl_matrix_f32_num_cols(m) == sl_vector_f32_size(rhs)
  );
  for (size_t row = 0; row < sl_matrix_f32_num_rows(m); ++row) {
    // TODO extremely slow
    for (size_t col = 0; col < sl_matrix_f32_num_cols(m); ++col) {
      *sl_matrix_f32_get(m, row, col) /= lhs->data[col] - rhs->data[col];
    }
  }
}

void sl_la_matrix_add_axis2(struct sl_matrix_f32 m[const static 1], const float x) {
  const size_t n = sl_matrix_f32_size(m);
  for (size_t i = 0; i < n; ++i) {
    m->data[i] += x;
  }
}

void sl_la_matrix_mul_axis2(struct sl_matrix_f32 m[const static 1], const float x) {
  const size_t n = sl_matrix_f32_size(m);
  assert(n <= (size_t)INT32_MAX);
  cblas_sscal((int)n, x, m->data, 1);
}
