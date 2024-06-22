#ifndef SL_LINALG_H_INCLUDED
#define SL_LINALG_H_INCLUDED

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
  if (size <= 0) {
    SL_LOG_ERROR("cannot allocate a vector with non-positive size");
    return (struct sl_la_vector){0};
  }
  return (struct sl_la_vector){
      .size = size,
      .data = sl_alloc((size_t)(size), sizeof(float)),
  };
}

void sl_la_vector_destroy(struct sl_la_vector v[const static 1]) {
  sl_free(v->data);
  *v = (struct sl_la_vector){0};
}

struct sl_la_matrix sl_la_matrix_create(const int rows, const int cols) {
  if (rows <= 0 || cols <= 0) {
    SL_LOG_ERROR("cannot allocate a matrix with non-positive size");
    return (struct sl_la_matrix){0};
  }
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

static inline struct sl_la_vector sl_la_matrix_row_view(
    struct sl_la_matrix a[const static 1],
    const int row) {
  return (struct sl_la_vector){
      .size = a->cols,
      .data = sl_la_matrix_get_row(a, row),
  };
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
  if (a->size != b->size) {
    SL_LOG_ERROR("mismatching vector dimensions, cannot compute dot");
    return 0;
  }
  return cblas_sdot(a->size, a->data, 1, b->data, 1);
}

void sl_la_vector_scale_add(const struct sl_la_vector a[const static 1],
                            const float alpha,
                            const struct sl_la_vector b[const static 1]) {
  if (a->size != b->size) {
    SL_LOG_ERROR("mismatching vector dimensions, cannot add vectors");
    return;
  }
  cblas_saxpy(a->size, alpha, b->data, 1, a->data, 1);
}

void sl_la_vector_add(const struct sl_la_vector a[const static 1],
                      const struct sl_la_vector b[const static 1]) {
  sl_la_vector_scale_add(a, 1, b);
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
  if (a->cols == b->rows && a->rows == c->rows && b->cols == c->cols) {
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
  } else {
    SL_LOG_ERROR("mismatching matrix dimensions, will not multiply");
  }
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

void sl_la_matrix_copy_row(struct sl_la_vector dst[const static 1],
                           struct sl_la_matrix src[const static 1],
                           const int row) {
  if (dst->size != src->cols) {
    SL_LOG_ERROR(
        "destination vector size is not equal to matrix column count, cannot "
        "copy row");
    return;
  }
  cblas_scopy(dst->size, sl_la_matrix_get_row(src, row), 1, dst->data, 1);
}

void sl_la_matrix_add_axis0(struct sl_la_matrix m[const static 1],
                            struct sl_la_vector v[const static 1]) {
  if (m->cols != v->size) {
    SL_LOG_ERROR("dimension mismatch %d != %d", m->cols, v->size);
    return;
  }
  for (int row = 0; row < m->rows; ++row) {
    cblas_saxpy(m->cols, 1, v->data, 1, sl_la_matrix_get_row(m, row), 1);
  }
}

void sl_la_matrix_div_axis0(struct sl_la_matrix m[const static 1],
                            struct sl_la_vector v[const static 1]) {
  if (m->cols != v->size) {
    SL_LOG_ERROR("dimension mismatch %d != %d", m->cols, v->size);
    return;
  }
  for (int row = 0; row < m->rows; ++row) {
    // TODO blas invert values?
    for (int col = 0; col < m->cols; ++col) {
      *sl_la_matrix_get(m, row, col) /= v->data[col];
    }
  }
}

void sl_la_matrix_add_axis2(struct sl_la_matrix m[const static 1],
                            const float x) {
  for (int i = 0; i < m->rows * m->cols; ++i) {
    m->data[i] += x;
  }
}

void sl_la_matrix_mul_axis2(struct sl_la_matrix m[const static 1],
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
