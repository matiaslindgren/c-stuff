#ifndef SL_LINALG_H_INCLUDED
#define SL_LINALG_H_INCLUDED

#include <assert.h>
#include <math.h>
#include <string.h>

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

#include "stufflib_macros.h"
#include "stufflib_math.h"
#include "stufflib_memory.h"

#ifndef SL_LA_FLOAT_EQ_TOL
  #define SL_LA_FLOAT_EQ_TOL 1e-12
#endif

#ifndef SL_LA_FLOAT_FORMAT
  #define SL_LA_FLOAT_FORMAT "%+15.12e"
#endif

static inline void sl_la_vec_add(const int count,
                                 float lhs[restrict count],
                                 const float rhs[restrict count]) {
  cblas_saxpy(count, 1, rhs, 1, lhs, 1);
}

static inline void sl_la_vec_sub(const int count,
                                 float lhs[restrict count],
                                 const float rhs[restrict count]) {
  cblas_saxpy(count, -1, rhs, 1, lhs, 1);
}

static inline void sl_la_vec_mul(const int count,
                                 float lhs[restrict count],
                                 const float rhs[restrict count]) {
  for (int i = 0; i < count; ++i) {
    lhs[i] *= rhs[i];
  }
}

static inline void sl_la_vec_min(const int count,
                                 float lhs[restrict count],
                                 const float rhs[restrict count]) {
  for (int i = 0; i < count; ++i) {
    lhs[i] = fminf(lhs[i], rhs[i]);
  }
}

static inline void sl_la_vec_max(const int count,
                                 float lhs[restrict count],
                                 const float rhs[restrict count]) {
  for (int i = 0; i < count; ++i) {
    lhs[i] = fmaxf(lhs[i], rhs[i]);
  }
}

// TODO generic n dim row-major tensor?
// vector and matrix as views instead of alloc

struct sl_la_vector {
  int size;
  float* data;
};

struct sl_la_matrix {
  int rows;
  int cols;
  float* data;
};

struct sl_la_vector sl_la_vector_create(const int size) {
  assert(size > 0);
  return (struct sl_la_vector){
      .size = size,
      .data = sl_alloc((size_t)(size), sizeof(float)),
  };
}

#define SL_LA_VECTOR_CREATE_INLINE(length)            \
  (struct sl_la_vector) {                             \
    .size = (length), .data = (float[(length)]) { 0 } \
  }

void sl_la_vector_destroy(struct sl_la_vector v[const static 1]) {
  sl_free(v->data);
  *v = (struct sl_la_vector){0};
}

struct sl_la_matrix sl_la_matrix_create(const int rows, const int cols) {
  assert(rows > 0 && cols > 0);
  return (struct sl_la_matrix){
      .rows = rows,
      .cols = cols,
      .data = sl_alloc((size_t)(rows * cols), sizeof(float)),
  };
}

void sl_la_matrix_destroy(struct sl_la_matrix a[const static 1]) {
  sl_free(a->data);
  *a = (struct sl_la_matrix){0};
}

static inline float* sl_la_matrix_get(struct sl_la_matrix a[const static 1],
                                      const int row,
                                      const int col) {
  return a->data + row * a->cols + col;
}

static inline float* sl_la_matrix_get_row(struct sl_la_matrix a[const static 1],
                                          const int row) {
  return sl_la_matrix_get(a, row, 0);
}

static inline size_t sl_la_matrix_size(struct sl_la_matrix a[const static 1]) {
  return ((size_t)a->rows) * ((size_t)a->cols);
}

static inline struct sl_la_vector sl_la_matrix_row_view(
    struct sl_la_matrix a[const static 1],
    const int row) {
  return (struct sl_la_vector){
      .size = a->cols,
      .data = sl_la_matrix_get_row(a, row),
  };
}

bool sl_la_vector_is_finite(struct sl_la_vector v[const static 1]) {
  return sl_math_is_finite(v->size, v->data);
}

void sl_la_vector_scale(struct sl_la_vector v[const static 1],
                        const float alpha) {
  cblas_sscal(v->size, alpha, v->data, 1);
}

void sl_la_vector_clear(struct sl_la_vector v[const static 1]) {
  memset(v->data, 0, sizeof(float) * (size_t)v->size);
}

float sl_la_vector_dot(const struct sl_la_vector a[const static 1],
                       const struct sl_la_vector b[const static 1]) {
  assert(a->size == b->size);
  return cblas_sdot(a->size, a->data, 1, b->data, 1);
}

static inline void sl_la_vector_add(
    const struct sl_la_vector a[const static 1],
    const struct sl_la_vector b[const static 1]) {
  assert(a->size == b->size);
  sl_la_vec_add(a->size, a->data, b->data);
}

static inline void sl_la_vector_sub(
    const struct sl_la_vector a[const static 1],
    const struct sl_la_vector b[const static 1]) {
  assert(a->size == b->size);
  sl_la_vec_sub(a->size, a->data, b->data);
}

static inline void sl_la_vector_mul(
    struct sl_la_vector lhs[const static 1],
    const struct sl_la_vector rhs[const static 1]) {
  assert(lhs->size == rhs->size);
  sl_la_vec_mul(lhs->size, lhs->data, rhs->data);
}

static inline void sl_la_vector_copy(struct sl_la_vector dst[const static 1],
                                     struct sl_la_vector src[const static 1]) {
  assert(dst->size == src->size);
  cblas_scopy(src->size, src->data, 1, dst->data, 1);
}

void sl_la_vector_print(FILE stream[const static 1],
                        const struct sl_la_vector v[const static 1]) {
  for (int i = 0; i < v->size; ++i) {
    const float value = v->data[i];
    if (fprintf(stream, (SL_LA_FLOAT_FORMAT " "), value) < 0) {
      SL_LOG_ERROR(
          ("failed printing vector value at (%d): " SL_LA_FLOAT_FORMAT),
          i,
          value);
      return;
    }
  }
  if (fprintf(stream, "\n") < 0) {
    SL_LOG_ERROR("failed printing newline");
    return;
  }
}

void sl_la_matrix_print(FILE stream[const static 1],
                        struct sl_la_matrix a[const static 1]) {
  for (int row = 0; row < a->rows; ++row) {
    for (int col = 0; col < a->cols; ++col) {
      float value = *sl_la_matrix_get(a, row, col);
      if (fprintf(stream, (SL_LA_FLOAT_FORMAT " "), value) < 0) {
        SL_LOG_ERROR(
            ("failed printing matrix value at (%d, %d): " SL_LA_FLOAT_FORMAT),
            row,
            col,
            value);
        return;
      }
    }
    if (fprintf(stream, "\n") < 0) {
      SL_LOG_ERROR("failed printing newline after row %d", row);
      return;
    }
  }
}

void sl_la_matrix_multiply(const struct sl_la_matrix a[const static 1],
                           const struct sl_la_matrix b[const static 1],
                           struct sl_la_matrix c[const static 1]) {
  // c := a b
  assert(a->cols == b->rows && a->rows == c->rows && b->cols == c->cols);
  // column-major implicit transpose trickery from
  // https://stackoverflow.com/a/56064726/5951112
  // 2024-06-10
  cblas_sgemm(CblasColMajor,
              CblasNoTrans,
              CblasNoTrans,
              b->cols, /* m */
              a->rows, /* n */
              a->cols, /* k */
              1,       /* alpha */
              b->data, /* a */
              b->cols, /* lda */
              a->data, /* b */
              a->cols, /* ldb */
              0,       /* beta */
              c->data, /* c */
              c->cols  /* ldc */
  );
}

double sl_la_matrix_trace(struct sl_la_matrix a[const static 1]) {
  double tr = 0;
  for (int i = 0; i < SL_MIN(a->rows, a->cols); ++i) {
    tr += *sl_la_matrix_get(a, i, i);
  }
  return tr;
}

double sl_la_matrix_frobenius_norm(struct sl_la_matrix a[const static 1]) {
  // https://en.wikipedia.org/wiki/Matrix_norm#Frobenius_norm
  // 2024-06-09
  double norm = 0;
  for (int r = 0; r < a->rows; ++r) {
    const float* row = sl_la_matrix_get_row(a, r);
    norm += cblas_sdot(a->cols, row, 1, row, 1);
  }
  return sqrt(norm);
}

static inline void sl_la_matrix_copy_row(
    struct sl_la_vector dst[const static 1],
    struct sl_la_matrix src[const static 1],
    const int row) {
  assert(dst->size == src->cols);
  cblas_scopy(dst->size, sl_la_matrix_get_row(src, row), 1, dst->data, 1);
}

static inline void sl_la_matrix_saxpy_axis0(
    struct sl_la_matrix m[const static 1],
    struct sl_la_vector v[const static 1],
    const float alpha) {
  assert(m->cols == v->size);
  for (int row = 0; row < m->rows; ++row) {
    cblas_saxpy(m->cols, alpha, v->data, 1, sl_la_matrix_get_row(m, row), 1);
  }
}

static inline void sl_la_matrix_add_axis0(
    struct sl_la_matrix m[const static 1],
    struct sl_la_vector v[const static 1]) {
  sl_la_matrix_saxpy_axis0(m, v, 1);
}

static inline void sl_la_matrix_sub_axis0(
    struct sl_la_matrix m[const static 1],
    struct sl_la_vector v[const static 1]) {
  sl_la_matrix_saxpy_axis0(m, v, -1);
}

static inline void sl_la_matrix_mul_axis0(
    struct sl_la_matrix m[const static 1],
    struct sl_la_vector v[const static 1]) {
  assert(m->cols == v->size);
  for (int row = 0; row < m->rows; ++row) {
    sl_la_vec_mul(m->cols, sl_la_matrix_get_row(m, row), v->data);
  }
}

static inline void sl_la_matrix_diffdiv_axis0(
    struct sl_la_matrix m[const static 1],
    struct sl_la_vector lhs[const static 1],
    struct sl_la_vector rhs[const static 1]) {
  assert(m->cols == lhs->size && m->cols == rhs->size);
  for (int row = 0; row < m->rows; ++row) {
    // TODO extremely slow
    for (int col = 0; col < m->cols; ++col) {
      *sl_la_matrix_get(m, row, col) /= lhs->data[col] - rhs->data[col];
    }
  }
}

static inline void sl_la_matrix_add_axis2(struct sl_la_matrix m[const static 1],
                                          const float x) {
  for (int i = 0; i < m->rows * m->cols; ++i) {
    m->data[i] += x;
  }
}

static inline void sl_la_matrix_mul_axis2(struct sl_la_matrix m[const static 1],
                                          const float x) {
  cblas_sscal(m->rows * m->cols, x, m->data, 1);
}

bool sl_la_vector_equal(struct sl_la_vector lhs[const static 1],
                        struct sl_la_vector rhs[const static 1]) {
  if (lhs->size != rhs->size) {
    return false;
  }
  for (int i = 0; i < lhs->size; ++i) {
    if (!sl_math_double_almost(lhs->data[i],
                               rhs->data[i],
                               SL_LA_FLOAT_EQ_TOL)) {
      return false;
    }
  }
  return true;
}

bool sl_la_matrix_equal(struct sl_la_matrix lhs[const static 1],
                        struct sl_la_matrix rhs[const static 1]) {
  if (lhs->rows != rhs->rows || lhs->cols != rhs->cols) {
    return false;
  }
  const int rows = lhs->rows;
  const int cols = lhs->cols;
  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      if (!sl_math_double_almost(*sl_la_matrix_get(lhs, row, col),
                                 *sl_la_matrix_get(rhs, row, col),
                                 SL_LA_FLOAT_EQ_TOL)) {
        return false;
      }
    }
  }
  return true;
}

#endif  // SL_LINALG_H_INCLUDED
