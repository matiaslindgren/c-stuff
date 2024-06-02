#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_blas.h"
#include "stufflib_macros.h"

bool fequal(const double a, const double b) { return fabs(a - b) < 1e-6; }

bool check_matrix_equal(const struct sl_blas_matrix a[const static 1],
                        const struct sl_blas_matrix b[const static 1]) {
  if (a->rows != b->rows) {
    return false;
  }
  if (a->cols != b->cols) {
    return false;
  }
  const int rows = a->rows;
  const int cols = a->cols;
  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      const float a_val = a->data[row * cols + col];
      const float b_val = b->data[row * cols + col];
      if (!fequal(a_val, b_val)) {
        fprintf(stderr, "(row %d, col %d) %g != %g\n", row, col, a_val, b_val);
        return false;
      }
    }
  }
  return true;
}

bool test_matrix_get(const bool) {
  struct sl_blas_matrix a = {
      .rows = 3,
      .cols = 3,
      .data = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8}
  };
  for (int row = 0; row < a.rows; ++row) {
    for (int col = 0; col < a.cols; ++col) {
      assert(a.data + row * a.cols + col == sl_blas_matrix_get(&a, row, col));
    }
  }
  return true;
}

bool test_matmul_square(const bool) {
  const struct sl_blas_matrix a = {
      .rows = 3,
      .cols = 3,
      .data = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8}
  };
  struct sl_blas_matrix result = sl_blas_matrix_zeros(a.rows, a.cols);

  sl_blas_matmul_f(&a, &a, &result);

  const struct sl_blas_matrix expected = {
      .rows = a.rows,
      .cols = a.cols,
      .data = (float[]){15, 18, 21, 42, 54, 66, 69, 90, 111}
  };
  bool ok = check_matrix_equal(&result, &expected);
  sl_blas_matrix_destroy(&result);
  return ok;
}

bool test_matmul_zeros(const bool) {
  const struct sl_blas_matrix a = {
      .rows = 3,
      .cols = 3,
      .data = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8}
  };
  struct sl_blas_matrix b = sl_blas_matrix_zeros(a.rows, a.cols);
  struct sl_blas_matrix result = b;

  sl_blas_matmul_f(&a, &b, &result);

  const struct sl_blas_matrix expected = b;
  bool ok = check_matrix_equal(&result, &expected);
  sl_blas_matrix_destroy(&b);
  return ok;
}

SL_TEST_MAIN(test_matrix_get, test_matmul_square, test_matmul_zeros)
