#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_linalg.h"
#include "stufflib_macros.h"
#include "stufflib_math.h"

bool fequal(const double a, const double b) {
  return sl_math_double_almost(a, b, 1e-9);
}

bool check_vector_equal(struct sl_la_vector a[const static 1],
                        struct sl_la_vector b[const static 1]) {
  if (!sl_la_vector_equal(a, b)) {
    fprintf(stderr, "!expected vectors a and b to be equal\n");
    fprintf(stderr, "a:\n");
    sl_la_vector_print(stderr, a);
    fprintf(stderr, "b:\n");
    sl_la_vector_print(stderr, b);
    return false;
  }
  return true;
}

bool check_matrix_equal(struct sl_la_matrix a[const static 1],
                        struct sl_la_matrix b[const static 1]) {
  if (!sl_la_matrix_equal(a, b)) {
    fprintf(stderr, "!expected matrices a and b to be equal\n");
    fprintf(stderr, "a:\n");
    sl_la_matrix_print(stderr, a);
    fprintf(stderr, "b:\n");
    sl_la_matrix_print(stderr, b);
    return false;
  }
  return true;
}

bool test_matrix_get(const bool) {
  struct sl_la_matrix a = {
      .rows = 3,
      .cols = 3,
      .data = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8}
  };
  for (int row = 0; row < a.rows; ++row) {
    for (int col = 0; col < a.cols; ++col) {
      assert(a.data + row * a.cols + col == sl_la_matrix_get(&a, row, col));
    }
  }
  return true;
}

bool test_matrix_equal(const bool) {
  {
    struct sl_la_matrix a = {
        .rows = 1,
        .cols = 1,
        .data = (float[]){0},
    };
    struct sl_la_matrix b = {
        .rows = 1,
        .cols = 1,
        .data = (float[]){0},
    };
    assert(sl_la_matrix_equal(&a, &b));
  }
  {
    struct sl_la_matrix a = {
        .rows = 1,
        .cols = 1,
        .data = (float[]){1},
    };
    struct sl_la_matrix b = {
        .rows = 1,
        .cols = 1,
        .data = (float[]){0},
    };
    assert(!sl_la_matrix_equal(&a, &b));
  }
  {
    struct sl_la_matrix a = {
        .rows = 2,
        .cols = 1,
        .data = (float[]){0, 0},
    };
    struct sl_la_matrix b = {
        .rows = 1,
        .cols = 1,
        .data = (float[]){0},
    };
    assert(!sl_la_matrix_equal(&a, &b));
  }
  {
    struct sl_la_matrix a = {
        .rows = 1,
        .cols = 1,
        .data = (float[]){0},
    };
    struct sl_la_matrix b = {
        .rows = 1,
        .cols = 2,
        .data = (float[]){0, 0},
    };
    assert(!sl_la_matrix_equal(&a, &b));
  }
  {
    struct sl_la_matrix a = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}
    };
    struct sl_la_matrix b = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}
    };
    assert(sl_la_matrix_equal(&a, &b));
  }
  {
    struct sl_la_matrix a = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}
    };
    struct sl_la_matrix b = {
        .rows = 4,
        .cols = 3,
        .data = (float[12]){0, 1, 2, 3, 4, 5}
    };
    assert(!sl_la_matrix_equal(&a, &b));
  }
  return true;
}

bool test_matrix_create(const bool) {
  {
    struct sl_la_matrix a = sl_la_matrix_create(1, 1);
    sl_la_matrix_destroy(&a);
  }
  {
    struct sl_la_matrix a = sl_la_matrix_create(1, 10);
    sl_la_matrix_destroy(&a);
  }
  {
    struct sl_la_matrix a = sl_la_matrix_create(10, 1);
    sl_la_matrix_destroy(&a);
  }
  {
    struct sl_la_matrix a = sl_la_matrix_create(1024, 4096);
    sl_la_matrix_destroy(&a);
  }
  {
    struct sl_la_matrix a = sl_la_matrix_create(65536, 256);
    sl_la_matrix_destroy(&a);
  }
  return true;
}

bool test_matrix_trace(const bool) {
  struct sl_la_matrix a = {
      .rows = 3,
      .cols = 3,
      .data = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8}
  };
  double tr = sl_la_matrix_trace(&a);
  double expected = 0 + 4 + 8;
  if (fequal(tr, expected)) {
    return true;
  }
  fprintf(stderr, "trace should be %g, not %g\n", expected, tr);
  return false;
}

bool test_matrix_add_axis0(const bool) {
  {
    struct sl_la_matrix a1 = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){2, -7, 3, 7, 1, -5, -3, 9, -5, -1, 6, -1},
    };
    struct sl_la_matrix a2 = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){3, -7, 1, 8, 1, -7, -2, 9, -7, 0, 6, -3},
    };
    struct sl_la_vector v = {
        .size = 3,
        .data = (float[]){1, 0, -2},
    };
    sl_la_matrix_add_axis0(&a1, &v);
    if (!check_matrix_equal(&a1, &a2)) {
      return false;
    }
  }
  return true;
}

bool test_matrix_sub_axis0(const bool) {
  {
    struct sl_la_matrix a1 = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){2, -7, 3, 7, 1, -5, -3, 9, -5, -1, 6, -1},
    };
    struct sl_la_matrix a2 = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){1, -7, 5, 6, 1, -3, -4, 9, -3, -2, 6, 1},
    };
    struct sl_la_vector v = {
        .size = 3,
        .data = (float[]){1, 0, -2},
    };
    sl_la_matrix_sub_axis0(&a1, &v);
    if (!check_matrix_equal(&a1, &a2)) {
      return false;
    }
  }
  return true;
}

bool test_matrix_mul_axis0(const bool) {
  {
    struct sl_la_matrix a1 = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){2, -7, 3, 7, 1, -5, -3, 9, -5, -1, 6, -1},
    };
    struct sl_la_matrix a2 = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){2, 0, -6, 7, 0, 10, -3, 0, 10, -1, 0, 2},
    };
    struct sl_la_vector v = {
        .size = 3,
        .data = (float[]){1, 0, -2},
    };
    sl_la_matrix_mul_axis0(&a1, &v);
    if (!check_matrix_equal(&a1, &a2)) {
      return false;
    }
  }
  return true;
}

bool test_matrix_diffdiv_axis0(const bool) {
  {
    struct sl_la_matrix a1 = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){2, -7, 3, 7, 1, -5, -3, 9, -5, -1, 6, -1},
    };
    struct sl_la_matrix a2 = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){-0.666666667f,
                          -7.0f,
                          -1.50f,
                          -2.333333333f,
                          1.0f, 2.50f,
                          1.0f, 9.0f,
                          2.50f, 0.333333333f,
                          6.0f, 0.50f},
    };
    struct sl_la_vector lhs = {
        .size = 3,
        .data = (float[]){0, 3, 10},
    };
    struct sl_la_vector rhs = {
        .size = 3,
        .data = (float[]){3, 2, 12},
    };
    sl_la_matrix_diffdiv_axis0(&a1, &lhs, &rhs);
    if (!check_matrix_equal(&a1, &a2)) {
      return false;
    }
  }
  return true;
}

bool test_matrix_multiply_square(const bool) {
  struct sl_la_matrix a = {
      .rows = 3,
      .cols = 3,
      .data = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8}
  };
  struct sl_la_matrix result = {
      .rows = 3,
      .cols = 3,
      .data = (float[3 * 3]){0},
  };
  sl_la_matrix_multiply(&a, &a, &result);
  struct sl_la_matrix expected = {
      .rows = a.rows,
      .cols = a.cols,
      .data = (float[]){15, 18, 21, 42, 54, 66, 69, 90, 111}
  };
  return check_matrix_equal(&result, &expected);
}

bool test_matrix_multiply_zeros(const bool) {
  struct sl_la_matrix a = {
      .rows = 3,
      .cols = 3,
      .data = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8},
  };
  struct sl_la_matrix b = {
      .rows = 3,
      .cols = 3,
      .data = (float[3 * 3]){0},
  };
  struct sl_la_matrix result = b;
  struct sl_la_matrix expected = b;
  sl_la_matrix_multiply(&a, &b, &result);
  return check_matrix_equal(&result, &expected);
}

bool test_matrix_multiply(const bool) {
  struct sl_la_matrix a = {
      .rows = 4,
      .cols = 3,
      .data = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}
  };
  struct sl_la_matrix b = {
      .rows = 3,
      .cols = 2,
      .data = (float[]){-3, -1, 1, 3, 5, 7}
  };
  struct sl_la_matrix result = {
      .rows = a.rows,
      .cols = b.cols,
      .data = (float[4 * 2]){0},
  };
  sl_la_matrix_multiply(&a, &b, &result);
  struct sl_la_matrix expected = {
      .rows = a.rows,
      .cols = b.cols,
      .data = (float[]){11, 17, 20, 44, 29, 71, 38, 98}
  };
  return check_matrix_equal(&result, &expected);
}

bool test_matrix_frobenius_norm(const bool) {
  struct sl_la_matrix a = {
      .rows = 4,
      .cols = 3,
      .data = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}
  };
  double norm = sl_la_matrix_frobenius_norm(&a);
  double expected = 22.494443758403985;
  if (fequal(norm, expected)) {
    return true;
  }
  fprintf(stderr, "frobenius norm should be %g, not %g\n", expected, norm);
  return false;
}

bool test_vector_scale(const bool) {
  for (int alpha = -10; alpha <= 10; ++alpha) {
    struct sl_la_vector v = {
        .size = 9,
        .data = (float[]){-4, -3, -2, -1, 0, 1, 2, 3, 4},
    };
    sl_la_vector_scale(&v, (float)alpha);
    for (int i = 0; i < v.size; ++i) {
      assert(fequal(v.data[i], (i - 4) * alpha));
    }
  }
  return true;
}

bool test_vector_equal(const bool) {
  {
    struct sl_la_vector v1 = {
        .size = 5,
        .data = (float[]){-1, 0, 1, 2, 3},
    };
    struct sl_la_vector v2 = {
        .size = 5,
        .data = (float[]){-1, 0, 1, 2, 3},
    };
    assert(sl_la_vector_equal(&v1, &v2));
  }
  return true;
}

bool test_vector_dot(const bool) {
  {
    struct sl_la_vector v1 = {
        .size = 5,
        .data = (float[]){-1, 0, 1, 2, 3},
    };
    struct sl_la_vector v2 = {
        .size = 5,
        .data = (float[5]){0},
    };
    assert(fequal(sl_la_vector_dot(&v1, &v2), 0));
  }
  {
    struct sl_la_vector v1 = {
        .size = 5,
        .data = (float[]){-1, 0, 1, 2, 3},
    };
    struct sl_la_vector v2 = {
        .size = 5,
        .data = (float[]){1, 2, 3, 4, 5},
    };
    assert(fequal(sl_la_vector_dot(&v1, &v2), 25));
  }
  return true;
}

bool test_vector_add(const bool) {
  {
    struct sl_la_vector v1 = {
        .size = 5,
        .data = (float[]){-1, 0, 1, 2, 3},
    };
    struct sl_la_vector v2 = {
        .size = 5,
        .data = (float[]){0, 1, 2, 3, 4},
    };
    struct sl_la_vector w1 = {
        .size = 5,
        .data = (float[]){1, 1, 1, 1, 1},
    };
    struct sl_la_vector w2 = {
        .size = 5,
        .data = (float[]){1, 1, 1, 1, 1},
    };
    sl_la_vector_add(&v1, &w1);
    if (!check_vector_equal(&v1, &v2)) {
      return false;
    }
    if (!check_vector_equal(&w1, &w2)) {
      return false;
    }
  }
  return true;
}

bool test_vector_sub(const bool) {
  {
    struct sl_la_vector v1 = {
        .size = 5,
        .data = (float[]){-1, 0, 1, 2, 3},
    };
    struct sl_la_vector v2 = {
        .size = 5,
        .data = (float[]){-2, -1, 0, 1, 2},
    };
    struct sl_la_vector w1 = {
        .size = 5,
        .data = (float[]){1, 1, 1, 1, 1},
    };
    struct sl_la_vector w2 = {
        .size = 5,
        .data = (float[]){1, 1, 1, 1, 1},
    };
    sl_la_vector_sub(&v1, &w1);
    if (!check_vector_equal(&v1, &v2)) {
      return false;
    }
    if (!check_vector_equal(&w1, &w2)) {
      return false;
    }
  }
  return true;
}

SL_TEST_MAIN(test_matrix_get,
             test_matrix_equal,
             test_matrix_create,
             test_matrix_trace,
             test_matrix_add_axis0,
             test_matrix_sub_axis0,
             test_matrix_mul_axis0,
             test_matrix_diffdiv_axis0,
             test_matrix_multiply_square,
             test_matrix_multiply_zeros,
             test_matrix_multiply,
             test_matrix_frobenius_norm,
             test_vector_scale,
             test_vector_equal,
             test_vector_dot,
             test_vector_add,
             test_vector_sub)
