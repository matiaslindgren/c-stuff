#include <stufflib/matrix/sl_matrix_f32.h>
#include <stufflib/matrix/sl_matrix_f64.h>
#include <stufflib/testing/testing.h>

SL_TEST(test_matrix_f64_wipe) {
  (void)ctx;
  struct sl_matrix_f64 m = {
      .data     = (double[]){1.0},
      .length   = {1, 1},
      .capacity = {1, 1},
  };
  sl_matrix_f64_wipe(&m);
  SL_ASSERT_TRUE(m.data == nullptr);
  SL_ASSERT_TRUE(sl_matrix_f64_num_rows(&m) == 0);
  SL_ASSERT_TRUE(sl_matrix_f64_num_cols(&m) == 0);
  SL_ASSERT_TRUE(m.capacity[0] == 0);
  SL_ASSERT_TRUE(m.capacity[1] == 0);
  return true;
}

SL_TEST(test_matrix_f64_clear) {
  (void)ctx;
  double buf[]           = {1.0, 2.0, 3.0, 4.0};
  struct sl_matrix_f64 m = {
      .data     = buf,
      .length   = {2, 2},
      .capacity = {2, 2}
  };
  sl_matrix_f64_clear(&m);
  SL_ASSERT_TRUE(m.data == buf);
  SL_ASSERT_TRUE(sl_matrix_f64_num_rows(&m) == 0);
  SL_ASSERT_TRUE(sl_matrix_f64_num_cols(&m) == 0);
  SL_ASSERT_TRUE(m.capacity[0] == 2);
  SL_ASSERT_TRUE(m.capacity[1] == 2);
  return true;
}

SL_TEST(test_matrix_f64_size) {
  (void)ctx;
  double buf[]           = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  struct sl_matrix_f64 m = {
      .data     = buf,
      .length   = {2, 3},
      .capacity = {2, 3}
  };
  SL_ASSERT_EQ_LL(sl_matrix_f64_size(&m), 6);
  return true;
}

SL_TEST(test_matrix_f64_get_set) {
  (void)ctx;
  double buf[]           = {0.0, 0.0, 0.0, 0.0};
  struct sl_matrix_f64 m = {
      .data     = buf,
      .length   = {2, 2},
      .capacity = {2, 2}
  };
  sl_matrix_f64_set(&m, 1, 0, 7.0);
  SL_ASSERT_EQ_DOUBLE(*sl_matrix_f64_get(&m, 1, 0), 7.0, 1e-15);
  SL_ASSERT_EQ_DOUBLE(buf[2], 7.0, 1e-15);
  return true;
}

SL_TEST(test_matrix_f64_offset) {
  (void)ctx;
  double buf[]           = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  struct sl_matrix_f64 m = {
      .data     = buf,
      .length   = {2, 3},
      .capacity = {2, 3}
  };
  SL_ASSERT_EQ_LL(sl_matrix_f64_offset(&m, 0, 0), 0);
  SL_ASSERT_EQ_LL(sl_matrix_f64_offset(&m, 0, 1), 1);
  SL_ASSERT_EQ_LL(sl_matrix_f64_offset(&m, 1, 0), 3);
  SL_ASSERT_EQ_LL(sl_matrix_f64_offset(&m, 1, 2), 5);
  return true;
}

SL_TEST(test_matrix_f32_wipe) {
  (void)ctx;
  struct sl_matrix_f32 m = {
      .data     = (float[]){1.0f},
      .length   = {1, 1},
      .capacity = {1, 1},
  };
  sl_matrix_f32_wipe(&m);
  SL_ASSERT_TRUE(m.data == nullptr);
  SL_ASSERT_TRUE(sl_matrix_f32_num_rows(&m) == 0);
  SL_ASSERT_TRUE(sl_matrix_f32_num_cols(&m) == 0);
  SL_ASSERT_TRUE(m.capacity[0] == 0);
  SL_ASSERT_TRUE(m.capacity[1] == 0);
  return true;
}

SL_TEST(test_matrix_f32_clear) {
  (void)ctx;
  float buf[]            = {1.0f, 2.0f, 3.0f, 4.0f};
  struct sl_matrix_f32 m = {
      .data     = buf,
      .length   = {2, 2},
      .capacity = {2, 2}
  };
  sl_matrix_f32_clear(&m);
  SL_ASSERT_TRUE(m.data == buf);
  SL_ASSERT_TRUE(sl_matrix_f32_num_rows(&m) == 0);
  SL_ASSERT_TRUE(sl_matrix_f32_num_cols(&m) == 0);
  SL_ASSERT_TRUE(m.capacity[0] == 2);
  SL_ASSERT_TRUE(m.capacity[1] == 2);
  return true;
}

SL_TEST(test_matrix_f32_size) {
  (void)ctx;
  float buf[]            = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  struct sl_matrix_f32 m = {
      .data     = buf,
      .length   = {2, 3},
      .capacity = {2, 3}
  };
  SL_ASSERT_EQ_LL(sl_matrix_f32_size(&m), 6);
  return true;
}

SL_TEST(test_matrix_f32_get_set) {
  (void)ctx;
  float buf[]            = {0.0f, 0.0f, 0.0f, 0.0f};
  struct sl_matrix_f32 m = {
      .data     = buf,
      .length   = {2, 2},
      .capacity = {2, 2}
  };
  sl_matrix_f32_set(&m, 1, 0, 7.0f);
  SL_ASSERT_EQ_DOUBLE(*sl_matrix_f32_get(&m, 1, 0), 7.0f, 1e-6);
  SL_ASSERT_EQ_DOUBLE(buf[2], 7.0f, 1e-6);
  return true;
}

SL_TEST(test_matrix_f32_offset) {
  (void)ctx;
  float buf[]            = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  struct sl_matrix_f32 m = {
      .data     = buf,
      .length   = {2, 3},
      .capacity = {2, 3}
  };
  SL_ASSERT_EQ_LL(sl_matrix_f32_offset(&m, 0, 0), 0);
  SL_ASSERT_EQ_LL(sl_matrix_f32_offset(&m, 0, 1), 1);
  SL_ASSERT_EQ_LL(sl_matrix_f32_offset(&m, 1, 0), 3);
  SL_ASSERT_EQ_LL(sl_matrix_f32_offset(&m, 1, 2), 5);
  return true;
}

SL_TEST_MAIN()
