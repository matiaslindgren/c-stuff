#ifndef SL_LINALG_H_INCLUDED
#define SL_LINALG_H_INCLUDED

#include <math.h>

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
#include "stufflib_memory.h"

struct sl_la_vector {
  int size;
  float* data;
};

struct sl_la_matrix {
  int rows;
  int cols;
  float* data;
};

struct sl_la_matrix sl_la_matrix_create(const int rows, const int cols) {
  if (rows < 0 || cols < 0) {
    SL_LOG_ERROR("cannot allocate a matrix with negative dimensions");
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

void sl_la_matrix_print(FILE stream[const static 1],
                        struct sl_la_matrix a[const static 1]) {
  for (int row = 0; row < a->rows; ++row) {
    for (int col = 0; col < a->cols; ++col) {
      float value = *sl_la_matrix_get(a, row, col);
      if (fprintf(stream, "%.3f ", value) < 0) {
        SL_LOG_ERROR("failed printing matrix value at (%d, %d): %.3g",
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

#endif  // SL_LINALG_H_INCLUDED
