#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define SL_LA_FLOAT_EQ_TOL 1e-6

#include "stufflib_args.h"
#include "stufflib_linalg.h"
#include "stufflib_macros.h"
#include "stufflib_math.h"
#include "stufflib_ml.h"

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

bool test_minmax_normalization(const bool) {
  {
    struct sl_la_matrix a1 = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){-6, -2, -1, -3, -3, -8, -1, 6, 10, 8, 10, -4},
    };
    struct sl_la_matrix a2 = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){-1.0f,
                          -0.84615385f,
                          -0.22222222f,
                          -0.57142857f,
                          -1.0f,
                          -1.0f,
                          -0.28571429f,
                          0.38461538f, 1.0f,
                          1.0f, 1.0f,
                          -0.55555556f},
    };
    sl_ml_rescale_features(&a1, -1, 1);
    if (!check_matrix_equal(&a1, &a2)) {
      return false;
    }
  }
  {
    struct sl_la_matrix a1 = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){-6, -2, -1, -3, -3, -8, -1, 6, 10, 8, 10, -4},
    };
    struct sl_la_matrix a2 = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){0.0f,
                          0.07692308f, 0.38888889f,
                          0.21428571f, 0.0f,
                          0.0f, 0.35714286f,
                          0.69230769f, 1.0f,
                          1.0f, 1.0f,
                          0.22222222f},
    };
    sl_ml_rescale_features(&a1, 0, 1);
    if (!check_matrix_equal(&a1, &a2)) {
      return false;
    }
  }
  {
    struct sl_la_matrix a1 = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){-6, -2, -1, -3, -3, -8, -1, 6, 10, 8, 10, -4},
    };
    struct sl_la_matrix a2 = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){1.0f,
                          1.69230769f, 4.5f,
                          2.92857143f, 1.0f,
                          1.0f, 4.21428571f,
                          7.23076923f, 10.0f,
                          10.0f, 10.0f,
                          3.0f},
    };
    sl_ml_rescale_features(&a1, 1, 10);
    if (!check_matrix_equal(&a1, &a2)) {
      return false;
    }
  }
  return true;
}

bool test_random_train_test_split(const bool) {
  for (int iter = 0; iter < 1000; ++iter) {
    struct sl_la_matrix data = {
        .rows = 12,
        .cols = 3,
        .data = (float[]){0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                          12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                          24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35},
    };
    int labels[] = {0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33};
    struct sl_la_matrix test = {.rows = 4, .cols = 3};
    struct sl_la_matrix train = {.rows = data.rows - test.rows, .cols = 3};
    int train_classes[12 - 4] = {0};
    int test_classes[4] = {0};
    sl_ml_random_train_test_split(&data,
                                  &train,
                                  &test,
                                  labels,
                                  train_classes,
                                  test_classes);

    int value_freq[12 * 3] = {0};
    int label_freq[12 * 3] = {0};
    for (int row = 0; row < data.rows; ++row) {
      for (int col = 0; col < data.cols; ++col) {
        const int value = (int)lroundf(*sl_la_matrix_get(&data, row, col));
        value_freq[value] += 1;
        if (col == 0) {
          assert(value == labels[row]);
          label_freq[value] += 1;
        }
      }
    }
    for (size_t i = 0; i < SL_ARRAY_LEN(value_freq); ++i) {
      assert(value_freq[i] == 1);
    }
    for (size_t i = 0; i < SL_ARRAY_LEN(labels); ++i) {
      assert(label_freq[labels[i]] == 1);
    }
  }
  return true;
}

SL_TEST_MAIN(test_minmax_normalization, test_random_train_test_split)
