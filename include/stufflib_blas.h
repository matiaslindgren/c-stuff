#ifndef SL_BLAS_H_INCLUDED
#define SL_BLAS_H_INCLUDED

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

struct sl_blas_vector {
  int size;
  float* data;
};

struct sl_blas_matrix {
  int rows;
  int cols;
  float* data;
};

struct sl_blas_matrix sl_blas_matrix_zeros(const int rows, const int cols) {
  if (rows < 0 || cols < 0) {
    SL_LOG_ERROR("cannot allocate a matrix with negative dimensions");
    return (struct sl_blas_matrix){0};
  }
  return (struct sl_blas_matrix){
      .rows = rows,
      .cols = cols,
      .data = sl_alloc((size_t)(rows * cols), sizeof(float)),
  };
}

void sl_blas_matrix_destroy(struct sl_blas_matrix a[const static 1]) {
  sl_free(a->data);
  *a = (struct sl_blas_matrix){0};
}

float* sl_blas_matrix_get(struct sl_blas_matrix a[const static 1],
                          const int row,
                          const int col) {
  return a->data + row * a->cols + col;
}

void sl_blas_matmul_f(const struct sl_blas_matrix a[const static 1],
                      const struct sl_blas_matrix b[const static 1],
                      struct sl_blas_matrix c[const static 1]) {
  cblas_sgemm(CblasRowMajor,
              CblasNoTrans,
              CblasNoTrans,
              a->rows,
              b->cols,
              b->cols,
              1.0f,
              a->data,
              a->rows,
              b->data,
              b->rows,
              1.0f,
              c->data,
              c->rows);
}

#endif  // SL_BLAS_H_INCLUDED
