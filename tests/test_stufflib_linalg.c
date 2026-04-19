#include <stdio.h>

#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/linalg/linalg.h>
#include <stufflib/macros/macros.h>
#include <stufflib/math/math.h>
#include <stufflib/testing/testing.h>

bool fequal(const double a, const double b) {
  return sl_math_double_almost(a, b, 1e-9);
}

bool check_vector_equal(
    struct sl_context ctx[static 1],
    struct sl_vector_f32 a[const static 1],
    struct sl_vector_f32 b[const static 1]
) {
  if (!sl_la_vector_equal(a, b)) {
    fprintf(stderr, "!expected vectors a and b to be equal\n");
    fprintf(stderr, "a:\n");
    if (!sl_la_vector_print(ctx, stderr, a)) {
      return false;
    }
    fprintf(stderr, "b:\n");
    if (!sl_la_vector_print(ctx, stderr, b)) {
      return false;
    }
    return false;
  }
  return true;
}

bool check_matrix_equal(
    struct sl_context ctx[static 1],
    struct sl_matrix_f32 a[const static 1],
    struct sl_matrix_f32 b[const static 1]
) {
  if (!sl_la_matrix_equal(a, b)) {
    fprintf(stderr, "!expected matrices a and b to be equal\n");
    fprintf(stderr, "a:\n");
    if (!sl_la_matrix_print(ctx, stderr, a)) {
      return false;
    }
    fprintf(stderr, "b:\n");
    if (!sl_la_matrix_print(ctx, stderr, b)) {
      return false;
    }
    return false;
  }
  return true;
}

SL_TEST(test_matrix_get) {
  (void)ctx;
  struct sl_matrix_f32 a = {
      .length   = {3, 3},
      .capacity = {3, 3},
      .data     = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8}
  };
  for (size_t row = 0; row < sl_matrix_f32_num_rows(&a); ++row) {
    for (size_t col = 0; col < sl_matrix_f32_num_cols(&a); ++col) {
      SL_ASSERT_TRUE(
          a.data + row * sl_matrix_f32_num_cols(&a) + col == sl_matrix_f32_get(&a, row, col)
      );
    }
  }
  return true;
}

SL_TEST(test_matrix_equal) {
  (void)ctx;
  {
    struct sl_matrix_f32 a = {
        .length   = {1, 1},
        .capacity = {1, 1},
        .data     = (float[]){0},
    };
    struct sl_matrix_f32 b = {
        .length   = {1, 1},
        .capacity = {1, 1},
        .data     = (float[]){0},
    };
    SL_ASSERT_TRUE(sl_la_matrix_equal(&a, &b));
  }
  {
    struct sl_matrix_f32 a = {
        .length   = {1, 1},
        .capacity = {1, 1},
        .data     = (float[]){1},
    };
    struct sl_matrix_f32 b = {
        .length   = {1, 1},
        .capacity = {1, 1},
        .data     = (float[]){0},
    };
    SL_ASSERT_TRUE(!sl_la_matrix_equal(&a, &b));
  }
  {
    struct sl_matrix_f32 a = {
        .length   = {2, 1},
        .capacity = {2, 1},
        .data     = (float[]){0, 0},
    };
    struct sl_matrix_f32 b = {
        .length   = {1, 1},
        .capacity = {1, 1},
        .data     = (float[]){0},
    };
    SL_ASSERT_TRUE(!sl_la_matrix_equal(&a, &b));
  }
  {
    struct sl_matrix_f32 a = {
        .length   = {1, 1},
        .capacity = {1, 1},
        .data     = (float[]){0},
    };
    struct sl_matrix_f32 b = {
        .length   = {1, 2},
        .capacity = {1, 2},
        .data     = (float[]){0, 0},
    };
    SL_ASSERT_TRUE(!sl_la_matrix_equal(&a, &b));
  }
  {
    struct sl_matrix_f32 a = {
        .length   = {4, 3},
        .capacity = {4, 3},
        .data     = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}
    };
    struct sl_matrix_f32 b = {
        .length   = {4, 3},
        .capacity = {4, 3},
        .data     = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}
    };
    SL_ASSERT_TRUE(sl_la_matrix_equal(&a, &b));
  }
  {
    struct sl_matrix_f32 a = {
        .length   = {4, 3},
        .capacity = {4, 3},
        .data     = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}
    };
    struct sl_matrix_f32 b = {
        .length   = {4, 3},
        .capacity = {4, 3},
        .data     = (float[12]){0, 1, 2, 3, 4, 5}
    };
    SL_ASSERT_TRUE(!sl_la_matrix_equal(&a, &b));
  }
  return true;
}

SL_TEST(test_matrix_create) {
  {
    struct sl_matrix_f32 a = {0};
    if (!sl_la_matrix_create(ctx, 1, 1, &a)) {
      return false;
    }
    sl_la_matrix_destroy(&a);
  }
  {
    struct sl_matrix_f32 a = {0};
    if (!sl_la_matrix_create(ctx, 1, 10, &a)) {
      return false;
    }
    sl_la_matrix_destroy(&a);
  }
  {
    struct sl_matrix_f32 a = {0};
    if (!sl_la_matrix_create(ctx, 10, 1, &a)) {
      return false;
    }
    sl_la_matrix_destroy(&a);
  }
  {
    struct sl_matrix_f32 a = {0};
    if (!sl_la_matrix_create(ctx, 1024, 4096, &a)) {
      return false;
    }
    sl_la_matrix_destroy(&a);
  }
  {
    struct sl_matrix_f32 a = {0};
    if (!sl_la_matrix_create(ctx, 65536, 256, &a)) {
      return false;
    }
    sl_la_matrix_destroy(&a);
  }
  return true;
}

SL_TEST(test_matrix_trace) {
  (void)ctx;
  struct sl_matrix_f32 a = {
      .length   = {3, 3},
      .capacity = {3, 3},
      .data     = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8}
  };
  double tr       = sl_la_matrix_trace(&a);
  double expected = 0 + 4 + 8;
  if (fequal(tr, expected)) {
    return true;
  }
  fprintf(stderr, "trace should be %g, not %g\n", expected, tr);
  return false;
}

SL_TEST(test_matrix_add_axis0) {
  (void)ctx;
  {
    struct sl_matrix_f32 a1 = {
        .length   = {4, 3},
        .capacity = {4, 3},
        .data     = (float[]){2, -7, 3, 7, 1, -5, -3, 9, -5, -1, 6, -1},
    };
    struct sl_matrix_f32 a2 = {
        .length   = {4, 3},
        .capacity = {4, 3},
        .data     = (float[]){3, -7, 1, 8, 1, -7, -2, 9, -7, 0, 6, -3},
    };
    struct sl_vector_f32 v = {
        .length   = {3},
        .capacity = {3},
        .data     = (float[]){1, 0, -2},
    };
    sl_la_matrix_add_axis0(&a1, &v);
    if (!check_matrix_equal(ctx, &a1, &a2)) {
      return false;
    }
  }
  return true;
}

SL_TEST(test_matrix_sub_axis0) {
  (void)ctx;
  {
    struct sl_matrix_f32 a1 = {
        .length   = {4, 3},
        .capacity = {4, 3},
        .data     = (float[]){2, -7, 3, 7, 1, -5, -3, 9, -5, -1, 6, -1},
    };
    struct sl_matrix_f32 a2 = {
        .length   = {4, 3},
        .capacity = {4, 3},
        .data     = (float[]){1, -7, 5, 6, 1, -3, -4, 9, -3, -2, 6, 1},
    };
    struct sl_vector_f32 v = {
        .length   = {3},
        .capacity = {3},
        .data     = (float[]){1, 0, -2},
    };
    sl_la_matrix_sub_axis0(&a1, &v);
    if (!check_matrix_equal(ctx, &a1, &a2)) {
      return false;
    }
  }
  return true;
}

SL_TEST(test_matrix_mul_axis0) {
  (void)ctx;
  {
    struct sl_matrix_f32 a1 = {
        .length   = {4, 3},
        .capacity = {4, 3},
        .data     = (float[]){2, -7, 3, 7, 1, -5, -3, 9, -5, -1, 6, -1},
    };
    struct sl_matrix_f32 a2 = {
        .length   = {4, 3},
        .capacity = {4, 3},
        .data     = (float[]){2, 0, -6, 7, 0, 10, -3, 0, 10, -1, 0, 2},
    };
    struct sl_vector_f32 v = {
        .length   = {3},
        .capacity = {3},
        .data     = (float[]){1, 0, -2},
    };
    sl_la_matrix_mul_axis0(&a1, &v);
    if (!check_matrix_equal(ctx, &a1, &a2)) {
      return false;
    }
  }
  return true;
}

SL_TEST(test_matrix_diffdiv_axis0) {
  (void)ctx;
  {
    struct sl_matrix_f32 a1 = {
        .length   = {4, 3},
        .capacity = {4, 3},
        .data     = (float[]){2, -7, 3, 7, 1, -5, -3, 9, -5, -1, 6, -1},
    };
    struct sl_matrix_f32 a2 = {
        .length   = {4, 3},
        .capacity = {4, 3},
        .data     = (float[]){-0.666666667f,
                     -7.0f,
                     -1.50f,
                     -2.333333333f,
                     1.0f, 2.50f,
                     1.0f, 9.0f,
                     2.50f, 0.333333333f,
                     6.0f, 0.50f},
    };
    struct sl_vector_f32 lhs = {
        .length   = {3},
        .capacity = {3},
        .data     = (float[]){0, 3, 10},
    };
    struct sl_vector_f32 rhs = {
        .length   = {3},
        .capacity = {3},
        .data     = (float[]){3, 2, 12},
    };
    sl_la_matrix_diffdiv_axis0(&a1, &lhs, &rhs);
    if (!check_matrix_equal(ctx, &a1, &a2)) {
      return false;
    }
  }
  return true;
}

SL_TEST(test_matrix_multiply_square) {
  (void)ctx;
  struct sl_matrix_f32 a = {
      .length   = {3, 3},
      .capacity = {3, 3},
      .data     = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8}
  };
  struct sl_matrix_f32 result = {
      .length   = {3, 3},
      .capacity = {3, 3},
      .data     = (float[3 * 3]){0},
  };
  sl_la_matrix_multiply(&a, &a, &result);
  struct sl_matrix_f32 expected = {
      .length   = {sl_matrix_f32_num_rows(&a), sl_matrix_f32_num_cols(&a)},
      .capacity = {sl_matrix_f32_num_rows(&a), sl_matrix_f32_num_cols(&a)},
      .data     = (float[]){15, 18, 21, 42, 54, 66, 69, 90, 111}
  };
  return check_matrix_equal(ctx, &result, &expected);
}

SL_TEST(test_matrix_multiply_zeros) {
  (void)ctx;
  struct sl_matrix_f32 a = {
      .length   = {3, 3},
      .capacity = {3, 3},
      .data     = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8},
  };
  struct sl_matrix_f32 b = {
      .length   = {3, 3},
      .capacity = {3, 3},
      .data     = (float[3 * 3]){0},
  };
  struct sl_matrix_f32 result   = b;
  struct sl_matrix_f32 expected = b;
  sl_la_matrix_multiply(&a, &b, &result);
  return check_matrix_equal(ctx, &result, &expected);
}

SL_TEST(test_matrix_multiply) {
  (void)ctx;
  struct sl_matrix_f32 a = {
      .length   = {4, 3},
      .capacity = {4, 3},
      .data     = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}
  };
  struct sl_matrix_f32 b = {
      .length   = {3, 2},
      .capacity = {3, 2},
      .data     = (float[]){-3, -1, 1, 3, 5, 7}
  };
  struct sl_matrix_f32 result = {
      .length   = {sl_matrix_f32_num_rows(&a), sl_matrix_f32_num_cols(&b)},
      .capacity = {sl_matrix_f32_num_rows(&a), sl_matrix_f32_num_cols(&b)},
      .data     = (float[4 * 2]){0},
  };
  sl_la_matrix_multiply(&a, &b, &result);
  struct sl_matrix_f32 expected = {
      .length   = {sl_matrix_f32_num_rows(&a), sl_matrix_f32_num_cols(&b)},
      .capacity = {sl_matrix_f32_num_rows(&a), sl_matrix_f32_num_cols(&b)},
      .data     = (float[]){11, 17, 20, 44, 29, 71, 38, 98}
  };
  return check_matrix_equal(ctx, &result, &expected);
}

SL_TEST(test_matrix_frobenius_norm) {
  (void)ctx;
  struct sl_matrix_f32 a = {
      .length   = {4, 3},
      .capacity = {4, 3},
      .data     = (float[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}
  };
  double norm     = sl_la_matrix_frobenius_norm(&a);
  double expected = 22.494443758403985;
  if (fequal(norm, expected)) {
    return true;
  }
  fprintf(stderr, "frobenius norm should be %g, not %g\n", expected, norm);
  return false;
}

SL_TEST(test_vector_scale) {
  (void)ctx;
  for (int alpha = -10; alpha <= 10; ++alpha) {
    struct sl_vector_f32 v = {
        .length   = {9},
        .capacity = {9},
        .data     = (float[]){-4, -3, -2, -1, 0, 1, 2, 3, 4},
    };
    sl_la_vector_scale(&v, (float)alpha);
    for (size_t i = 0; i < sl_vector_f32_size(&v); ++i) {
      SL_ASSERT_TRUE(fequal((double)v.data[i], (double)((double)i - 4) * alpha));
    }
  }
  return true;
}

SL_TEST(test_vector_equal) {
  (void)ctx;
  {
    struct sl_vector_f32 v1 = {
        .length   = {5},
        .capacity = {5},
        .data     = (float[]){-1, 0, 1, 2, 3},
    };
    struct sl_vector_f32 v2 = {
        .length   = {5},
        .capacity = {5},
        .data     = (float[]){-1, 0, 1, 2, 3},
    };
    SL_ASSERT_TRUE(sl_la_vector_equal(&v1, &v2));
  }
  return true;
}

SL_TEST(test_vector_dot) {
  (void)ctx;
  {
    struct sl_vector_f32 v1 = {
        .length   = {5},
        .capacity = {5},
        .data     = (float[]){-1, 0, 1, 2, 3},
    };
    struct sl_vector_f32 v2 = {
        .length   = {5},
        .capacity = {5},
        .data     = (float[5]){0},
    };
    SL_ASSERT_TRUE(fequal((double)sl_la_vector_dot(&v1, &v2), 0));
  }
  {
    struct sl_vector_f32 v1 = {
        .length   = {5},
        .capacity = {5},
        .data     = (float[]){-1, 0, 1, 2, 3},
    };
    struct sl_vector_f32 v2 = {
        .length   = {5},
        .capacity = {5},
        .data     = (float[]){1, 2, 3, 4, 5},
    };
    SL_ASSERT_TRUE(fequal((double)sl_la_vector_dot(&v1, &v2), 25));
  }
  return true;
}

SL_TEST(test_vector_add) {
  (void)ctx;
  {
    struct sl_vector_f32 v1 = {
        .length   = {5},
        .capacity = {5},
        .data     = (float[]){-1, 0, 1, 2, 3},
    };
    struct sl_vector_f32 v2 = {
        .length   = {5},
        .capacity = {5},
        .data     = (float[]){0, 1, 2, 3, 4},
    };
    struct sl_vector_f32 w1 = {
        .length   = {5},
        .capacity = {5},
        .data     = (float[]){1, 1, 1, 1, 1},
    };
    struct sl_vector_f32 w2 = {
        .length   = {5},
        .capacity = {5},
        .data     = (float[]){1, 1, 1, 1, 1},
    };
    sl_la_vector_add(&v1, &w1);
    if (!check_vector_equal(ctx, &v1, &v2)) {
      return false;
    }
    if (!check_vector_equal(ctx, &w1, &w2)) {
      return false;
    }
  }
  return true;
}

SL_TEST(test_vector_sub) {
  (void)ctx;
  {
    struct sl_vector_f32 v1 = {
        .length   = {5},
        .capacity = {5},
        .data     = (float[]){-1, 0, 1, 2, 3},
    };
    struct sl_vector_f32 v2 = {
        .length   = {5},
        .capacity = {5},
        .data     = (float[]){-2, -1, 0, 1, 2},
    };
    struct sl_vector_f32 w1 = {
        .length   = {5},
        .capacity = {5},
        .data     = (float[]){1, 1, 1, 1, 1},
    };
    struct sl_vector_f32 w2 = {
        .length   = {5},
        .capacity = {5},
        .data     = (float[]){1, 1, 1, 1, 1},
    };
    sl_la_vector_sub(&v1, &w1);
    if (!check_vector_equal(ctx, &v1, &v2)) {
      return false;
    }
    if (!check_vector_equal(ctx, &w1, &w2)) {
      return false;
    }
  }
  return true;
}

SL_TEST_MAIN()
