#include <stufflib/testing/testing.h>
#include <stufflib/vector/sl_vector_f64.h>

SL_TEST(test_vector_wipe) {
  (void)ctx;
  (void)verbose;
  struct sl_vector_f64 v = {.data = (double[]){1.0}, .length = {1}, .capacity = {1}};
  sl_vector_f64_wipe(&v);
  SL_ASSERT_TRUE(v.data == nullptr);
  SL_ASSERT_TRUE(v.length[0] == 0);
  SL_ASSERT_TRUE(v.capacity[0] == 0);
  return true;
}

SL_TEST(test_vector_clear) {
  (void)ctx;
  (void)verbose;
  double buf[]           = {1.0, 2.0, 3.0};
  struct sl_vector_f64 v = {.data = buf, .length = {3}, .capacity = {3}};
  sl_vector_f64_clear(&v);
  SL_ASSERT_TRUE(v.data == buf);
  SL_ASSERT_TRUE(v.length[0] == 0);
  SL_ASSERT_TRUE(v.capacity[0] == 3);
  return true;
}

SL_TEST(test_vector_count) {
  (void)ctx;
  (void)verbose;
  double buf[]           = {0.0, 0.0, 0.0};
  struct sl_vector_f64 v = {.data = buf, .length = {3}, .capacity = {3}};
  SL_ASSERT_EQ_LL(sl_vector_f64_count(&v), 3);
  return true;
}

SL_TEST(test_vector_get_set) {
  (void)ctx;
  (void)verbose;
  double buf[]           = {0.0, 0.0, 0.0};
  struct sl_vector_f64 v = {.data = buf, .length = {3}, .capacity = {3}};
  sl_vector_f64_set(&v, 1, 2.5);
  SL_ASSERT_EQ_DOUBLE(*sl_vector_f64_get(&v, 1), 2.5, 1e-15);
  SL_ASSERT_EQ_DOUBLE(buf[1], 2.5, 1e-15);
  return true;
}

SL_TEST(test_vector_offset) {
  (void)ctx;
  (void)verbose;
  double buf[]           = {0.0, 0.0, 0.0};
  struct sl_vector_f64 v = {.data = buf, .length = {3}, .capacity = {3}};
  SL_ASSERT_EQ_LL(sl_vector_f64_offset(&v, 0), 0);
  SL_ASSERT_EQ_LL(sl_vector_f64_offset(&v, 1), 1);
  SL_ASSERT_EQ_LL(sl_vector_f64_offset(&v, 2), 2);
  return true;
}

SL_TEST_MAIN()
