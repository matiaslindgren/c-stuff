#include <stufflib/tensor/sl_tensor_f64.h>
#include <stufflib/testing/testing.h>

SL_TEST(test_tensor3_wipe) {
  (void)ctx;
  struct sl_tensor3_f64 t = {
      .data     = (double[]){1.0},
      .length   = {1, 1, 1},
      .capacity = {1, 1, 1},
  };
  sl_tensor3_f64_wipe(&t);
  SL_ASSERT_TRUE(t.data == nullptr);
  SL_ASSERT_TRUE(t.length[0] == 0);
  SL_ASSERT_TRUE(t.length[1] == 0);
  SL_ASSERT_TRUE(t.length[2] == 0);
  SL_ASSERT_TRUE(t.capacity[0] == 0);
  SL_ASSERT_TRUE(t.capacity[1] == 0);
  SL_ASSERT_TRUE(t.capacity[2] == 0);
  return true;
}

SL_TEST(test_tensor3_clear) {
  (void)ctx;
  double buf[]            = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
  struct sl_tensor3_f64 t = {
      .data     = buf,
      .length   = {2, 2, 2},
      .capacity = {2, 2, 2}
  };
  sl_tensor3_f64_clear(&t);
  SL_ASSERT_TRUE(t.data == buf);
  SL_ASSERT_TRUE(t.length[0] == 0);
  SL_ASSERT_TRUE(t.length[1] == 0);
  SL_ASSERT_TRUE(t.length[2] == 0);
  SL_ASSERT_TRUE(t.capacity[0] == 2);
  SL_ASSERT_TRUE(t.capacity[1] == 2);
  SL_ASSERT_TRUE(t.capacity[2] == 2);
  return true;
}

SL_TEST(test_tensor3_size) {
  (void)ctx;
  double buf[24]          = {0};
  struct sl_tensor3_f64 t = {
      .data     = buf,
      .length   = {2, 3, 4},
      .capacity = {2, 3, 4}
  };
  SL_ASSERT_EQ_LL(sl_tensor3_f64_size(&t), 24);
  return true;
}

SL_TEST(test_tensor3_get_set) {
  (void)ctx;
  double buf[8]           = {0};
  struct sl_tensor3_f64 t = {
      .data     = buf,
      .length   = {2, 2, 2},
      .capacity = {2, 2, 2}
  };
  sl_tensor3_f64_set(&t, 1, 0, 1, 5.0);
  SL_ASSERT_EQ_DOUBLE(*sl_tensor3_f64_get(&t, 1, 0, 1), 5.0, 1e-15);
  SL_ASSERT_EQ_DOUBLE(buf[sl_tensor3_f64_offset(&t, 1, 0, 1)], 5.0, 1e-15);
  return true;
}

SL_TEST(test_tensor3_offset) {
  (void)ctx;
  double buf[24]          = {0};
  struct sl_tensor3_f64 t = {
      .data     = buf,
      .length   = {2, 3, 4},
      .capacity = {2, 3, 4}
  };
  SL_ASSERT_EQ_LL(sl_tensor3_f64_offset(&t, 0, 0, 0), 0);
  SL_ASSERT_EQ_LL(sl_tensor3_f64_offset(&t, 0, 0, 1), 1);
  SL_ASSERT_EQ_LL(sl_tensor3_f64_offset(&t, 0, 1, 0), 4);
  SL_ASSERT_EQ_LL(sl_tensor3_f64_offset(&t, 1, 0, 0), 12);
  SL_ASSERT_EQ_LL(sl_tensor3_f64_offset(&t, 1, 2, 3), 23);
  return true;
}

SL_TEST(test_tensor3_f64_create_zero_length) {
  struct sl_tensor3_f64 t = {0};
  SL_ASSERT_FALSE(sl_tensor3_f64_create(ctx, &t, 0, 2, 3));
  SL_ASSERT_EQ_LL(sl_error_depth(&ctx->errors), 1);
  struct sl_error_msg err = {0};
  SL_ASSERT_TRUE(sl_error_pop(&ctx->errors, &err));
  SL_ASSERT_EQ_STR(err.msg, "will not allocate sl_tensor3_f64 of length 0");
  return true;
}

SL_TEST(test_tensor3_f64_create) {
  struct sl_tensor3_f64 t = {0};
  SL_ASSERT_TRUE(sl_tensor3_f64_create(ctx, &t, 2, 3, 4));
  SL_ASSERT_TRUE(t.data != nullptr);
  SL_ASSERT_EQ_LL(sl_tensor3_f64_size(&t), 24);
  SL_ASSERT_EQ_LL(t.capacity[0], 2);
  SL_ASSERT_EQ_LL(t.capacity[1], 3);
  SL_ASSERT_EQ_LL(t.capacity[2], 4);
  sl_tensor3_f64_destroy(&t);
  return true;
}

SL_TEST_MAIN()
