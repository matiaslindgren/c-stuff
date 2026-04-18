#include <stufflib/testing/testing.h>
#include <stufflib/vector/sl_vector_f32.h>
#include <stufflib/vector/sl_vector_f64.h>

SL_TEST(test_vector_f64_wipe) {
  (void)ctx;
  (void)verbose;
  struct sl_vector_f64 v = {.data = (double[]){1.0}, .length = {1}, .capacity = {1}};
  sl_vector_f64_wipe(&v);
  SL_ASSERT_TRUE(v.data == nullptr);
  SL_ASSERT_TRUE(sl_vector_f64_size(&v) == 0);
  SL_ASSERT_TRUE(v.capacity[0] == 0);
  return true;
}

SL_TEST(test_vector_f64_clear) {
  (void)ctx;
  (void)verbose;
  double buf[]           = {1.0, 2.0, 3.0};
  struct sl_vector_f64 v = {.data = buf, .length = {3}, .capacity = {3}};
  sl_vector_f64_clear(&v);
  SL_ASSERT_TRUE(v.data == buf);
  SL_ASSERT_TRUE(sl_vector_f64_size(&v) == 0);
  SL_ASSERT_TRUE(v.capacity[0] == 3);
  return true;
}

SL_TEST(test_vector_f64_size) {
  (void)ctx;
  (void)verbose;
  double buf[]           = {0.0, 0.0, 0.0};
  struct sl_vector_f64 v = {.data = buf, .length = {3}, .capacity = {3}};
  SL_ASSERT_EQ_LL(sl_vector_f64_size(&v), 3);
  return true;
}

SL_TEST(test_vector_f64_get_set) {
  (void)ctx;
  (void)verbose;
  double buf[]           = {0.0, 0.0, 0.0};
  struct sl_vector_f64 v = {.data = buf, .length = {3}, .capacity = {3}};
  sl_vector_f64_set(&v, 1, 2.5);
  SL_ASSERT_EQ_DOUBLE(*sl_vector_f64_get(&v, 1), 2.5, 1e-15);
  SL_ASSERT_EQ_DOUBLE(buf[1], 2.5, 1e-15);
  return true;
}

SL_TEST(test_vector_f64_offset) {
  (void)ctx;
  (void)verbose;
  double buf[]           = {0.0, 0.0, 0.0};
  struct sl_vector_f64 v = {.data = buf, .length = {3}, .capacity = {3}};
  SL_ASSERT_EQ_LL(sl_vector_f64_offset(&v, 0), 0);
  SL_ASSERT_EQ_LL(sl_vector_f64_offset(&v, 1), 1);
  SL_ASSERT_EQ_LL(sl_vector_f64_offset(&v, 2), 2);
  return true;
}

SL_TEST(test_vector_f32_wipe) {
  (void)ctx;
  (void)verbose;
  struct sl_vector_f32 v = {.data = (float[]){1.0f}, .length = {1}, .capacity = {1}};
  sl_vector_f32_wipe(&v);
  SL_ASSERT_TRUE(v.data == nullptr);
  SL_ASSERT_TRUE(sl_vector_f32_size(&v) == 0);
  SL_ASSERT_TRUE(v.capacity[0] == 0);
  return true;
}

SL_TEST(test_vector_f32_clear) {
  (void)ctx;
  (void)verbose;
  float buf[]            = {1.0f, 2.0f, 3.0f};
  struct sl_vector_f32 v = {.data = buf, .length = {3}, .capacity = {3}};
  sl_vector_f32_clear(&v);
  SL_ASSERT_TRUE(v.data == buf);
  SL_ASSERT_TRUE(sl_vector_f32_size(&v) == 0);
  SL_ASSERT_TRUE(v.capacity[0] == 3);
  return true;
}

SL_TEST(test_vector_f32_size) {
  (void)ctx;
  (void)verbose;
  float buf[]            = {0.0f, 0.0f, 0.0f};
  struct sl_vector_f32 v = {.data = buf, .length = {3}, .capacity = {3}};
  SL_ASSERT_EQ_LL(sl_vector_f32_size(&v), 3);
  return true;
}

SL_TEST(test_vector_f32_get_set) {
  (void)ctx;
  (void)verbose;
  float buf[]            = {0.0f, 0.0f, 0.0f};
  struct sl_vector_f32 v = {.data = buf, .length = {3}, .capacity = {3}};
  sl_vector_f32_set(&v, 1, 2.5f);
  SL_ASSERT_EQ_DOUBLE(*sl_vector_f32_get(&v, 1), 2.5f, 1e-6);
  SL_ASSERT_EQ_DOUBLE(buf[1], 2.5f, 1e-6);
  return true;
}

SL_TEST(test_vector_f32_offset) {
  (void)ctx;
  (void)verbose;
  float buf[]            = {0.0f, 0.0f, 0.0f};
  struct sl_vector_f32 v = {.data = buf, .length = {3}, .capacity = {3}};
  SL_ASSERT_EQ_LL(sl_vector_f32_offset(&v, 0), 0);
  SL_ASSERT_EQ_LL(sl_vector_f32_offset(&v, 1), 1);
  SL_ASSERT_EQ_LL(sl_vector_f32_offset(&v, 2), 2);
  return true;
}

SL_TEST_MAIN()
