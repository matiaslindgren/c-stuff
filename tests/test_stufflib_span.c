#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stufflib/iterator/iterator.h>
#include <stufflib/macros/macros.h>
#include <stufflib/span/span.h>
#include <stufflib/testing/testing.h>

SL_TEST(test_data_view_one) {
  (void)ctx;
  unsigned char x     = 1;
  struct sl_span data = sl_span_view(1, &x);
  SL_ASSERT_TRUE(data.size == 1);
  SL_ASSERT_TRUE(!data.owned);
  SL_ASSERT_TRUE(data.data == &x);
  sl_span_destroy(&data);
  SL_ASSERT_TRUE(data.data == nullptr);
  return true;
}

SL_TEST(test_data_view_array) {
  (void)ctx;
  unsigned char x[]   = {1, 2, 3, 4};
  const size_t n      = SL_ARRAY_LEN(x);
  struct sl_span data = sl_span_view(n, x);
  SL_ASSERT_TRUE(data.size == n);
  SL_ASSERT_TRUE(!data.owned);
  SL_ASSERT_TRUE(data.data == x);
  for (size_t i = 0; i < n; ++i) {
    SL_ASSERT_TRUE(data.data + i == x + i);
  }
  sl_span_destroy(&data);
  SL_ASSERT_TRUE(data.data == nullptr);
  return true;
}

SL_TEST(test_data_create) {
  const size_t n      = 10;
  struct sl_span data = {0};
  SL_ASSERT_TRUE(sl_span_create(ctx, n, &data));
  SL_ASSERT_TRUE(data.size == n);
  SL_ASSERT_TRUE(data.owned);
  SL_ASSERT_TRUE(data.data);
  for (size_t i = 0; i < n; ++i) {
    SL_ASSERT_TRUE(data.data[i] == 0);
  }
  sl_span_destroy(&data);
  SL_ASSERT_TRUE(data.data == nullptr);
  return true;
}

SL_TEST(test_data_create_empty) {
  struct sl_span data = {0};
  SL_ASSERT_TRUE(sl_span_create(ctx, 0, &data));
  SL_ASSERT_TRUE(data.size == 0);
  SL_ASSERT_TRUE(data.owned);
  SL_ASSERT_TRUE(!data.data);
  sl_span_destroy(&data);
  SL_ASSERT_TRUE(data.data == nullptr);
  return true;
}

SL_TEST(test_data_create_from_cstr) {
  const char* strings[] = {
      "000",
      "0",
      "",
      "x0",
      "0x",
      "00x",
  };
  const size_t lengths[] = {
      3,
      1,
      0,
      2,
      2,
      3,
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(strings); ++i) {
    const char* cstr   = strings[i];
    struct sl_span str = {0};
    SL_ASSERT_TRUE(sl_span_from_str(ctx, cstr, &str));
    SL_ASSERT_TRUE(str.size == lengths[i]);
    SL_ASSERT_TRUE(str.owned);
    if (str.size == 0) {
      SL_ASSERT_TRUE(!str.data);
    } else {
      SL_ASSERT_TRUE(str.data);
      SL_ASSERT_TRUE(memcmp(str.data, cstr, str.size) == 0);
    }
    SL_ASSERT_TRUE(!sl_span_is_hexadecimal_str(&str));
    sl_span_destroy(&str);
  }
  return true;
}

SL_TEST(test_data_create_from_hexadecimal_cstr) {
  const char* strings[] = {
      "0x00",
      "0x1",
      "0x0",
      "0xf",
      "0xff",
      "0x0f0",
      "0xff00ee",
      "0x0123456789abcdef1",
  };
  const size_t str_lengths[] = {
      4,
      3,
      3,
      3,
      4,
      5,
      8,
      19,
  };
  const unsigned char* bytes[] = {
      (unsigned char[]){0},
      (unsigned char[]){1},
      (unsigned char[]){0},
      (unsigned char[]){0xf},
      (unsigned char[]){0xff},
      (unsigned char[]){0xf, 0},
      (unsigned char[]){0xff, 0, 0xee},
      (unsigned char[]){0x1, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 1},
  };
  const size_t data_lengths[] = {
      1,
      1,
      1,
      1,
      1,
      2,
      3,
      9,
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(strings); ++i) {
    const char* cstr   = strings[i];
    struct sl_span str = {0};
    SL_ASSERT_TRUE(sl_span_from_str(ctx, cstr, &str));
    SL_ASSERT_TRUE(str.size == str_lengths[i]);
    SL_ASSERT_TRUE(str.owned);
    SL_ASSERT_TRUE(str.data);
    SL_ASSERT_TRUE(memcmp(str.data, cstr, str.size) == 0);
    SL_ASSERT_TRUE(sl_span_is_hexadecimal_str(&str));
    struct sl_span num = {0};
    SL_ASSERT_TRUE(sl_span_parse_hex(ctx, &str, &num));
    SL_ASSERT_TRUE(num.size == data_lengths[i]);
    SL_ASSERT_TRUE(num.owned);
    SL_ASSERT_TRUE(num.data);
    SL_ASSERT_TRUE(memcmp(num.data, bytes[i], num.size) == 0);
    sl_span_destroy(&num);
    sl_span_destroy(&str);
  }
  return true;
}

SL_TEST(test_data_copy_view) {
  unsigned char x[]    = {1, 2, 3, 4};
  const size_t n       = SL_ARRAY_LEN(x);
  struct sl_span data1 = sl_span_view(n, x);
  struct sl_span data2 = {0};
  SL_ASSERT_TRUE(sl_span_copy(ctx, &data1, &data2));
  SL_ASSERT_TRUE(!data1.owned);
  SL_ASSERT_TRUE(data1.data == x);
  SL_ASSERT_TRUE(data2.owned);
  SL_ASSERT_TRUE(data2.data != x);
  for (size_t i = 0; i < n; ++i) {
    SL_ASSERT_TRUE(data1.data + i == x + i);
    SL_ASSERT_TRUE(data2.data + i != x + i);
    SL_ASSERT_TRUE(data2.data[i] == x[i]);
  }
  sl_span_destroy(&data2);
  SL_ASSERT_TRUE(data2.data == nullptr);
  return true;
}

SL_TEST(test_data_concat_views) {
  unsigned char x1[]   = {1, 2, 3, 4};
  unsigned char x2[]   = {6, 7, 8, 9, 10, 11};
  const size_t n1      = SL_ARRAY_LEN(x1);
  const size_t n2      = SL_ARRAY_LEN(x2);
  struct sl_span data1 = sl_span_view(n1, x1);
  struct sl_span data2 = sl_span_view(n2, x2);
  struct sl_span data3 = {0};
  SL_ASSERT_TRUE(sl_span_concat(ctx, &data1, &data2, &data3));
  SL_ASSERT_TRUE(!data1.owned);
  SL_ASSERT_TRUE(!data2.owned);
  SL_ASSERT_TRUE(data3.owned);
  SL_ASSERT_TRUE(data1.data == x1);
  SL_ASSERT_TRUE(data2.data == x2);
  SL_ASSERT_TRUE(data3.data != x1);
  SL_ASSERT_TRUE(data3.data != x2);
  SL_ASSERT_TRUE(data3.size == data1.size + data2.size);
  for (size_t i = 0; i < n1; ++i) {
    SL_ASSERT_TRUE(data1.data + i != data3.data + i);
    SL_ASSERT_TRUE(data1.data[i] == data3.data[i]);
  }
  for (size_t i = 0; i < n2; ++i) {
    SL_ASSERT_TRUE(data2.data + i != data3.data + n1 + i);
    SL_ASSERT_TRUE(data2.data[i] == data3.data[n1 + i]);
  }
  sl_span_destroy(&data3);
  SL_ASSERT_TRUE(data3.data == nullptr);
  return true;
}

SL_TEST(test_data_concat_empty) {
  unsigned char x1[]   = {1, 2, 3, 4};
  const size_t n1      = SL_ARRAY_LEN(x1);
  struct sl_span data1 = sl_span_view(n1, x1);
  struct sl_span data2 = {0};
  struct sl_span data3 = {0};
  SL_ASSERT_TRUE(sl_span_create(ctx, 0, &data2));
  SL_ASSERT_TRUE(sl_span_concat(ctx, &data1, &data2, &data3));
  SL_ASSERT_TRUE(!data1.owned);
  SL_ASSERT_TRUE(data2.owned);
  SL_ASSERT_TRUE(data3.owned);
  SL_ASSERT_TRUE(data1.data == x1);
  SL_ASSERT_TRUE(data2.data == nullptr);
  SL_ASSERT_TRUE(data3.data != x1);
  SL_ASSERT_TRUE(data3.size == data1.size);
  for (size_t i = 0; i < n1; ++i) {
    SL_ASSERT_TRUE(data1.data + i != data3.data + i);
    SL_ASSERT_TRUE(data1.data[i] == data3.data[i]);
  }
  sl_span_destroy(&data3);
  SL_ASSERT_TRUE(data3.data == nullptr);
  return true;
}

SL_TEST(test_data_slice) {
  (void)ctx;
  unsigned char x[]   = {1, 2, 3, 4, 5, 6};
  const size_t n      = SL_ARRAY_LEN(x);
  struct sl_span data = sl_span_view(n, x);
  for (size_t begin = 0; begin < n; ++begin) {
    for (size_t end = begin; end < n; ++end) {
      struct sl_span slice = sl_span_slice(&data, begin, end);
      SL_ASSERT_TRUE(!slice.owned);
      SL_ASSERT_TRUE(slice.size == end - begin);
      if (!slice.size) {
        SL_ASSERT_TRUE(!slice.data);
      }
      for (size_t i = 0; i < slice.size; ++i) {
        SL_ASSERT_TRUE(slice.data + i == data.data + begin + i);
      }
    }
  }
  return true;
}

SL_TEST(test_data_slice_past_end) {
  (void)ctx;
  unsigned char x[]    = {1, 2, 3, 4, 5, 6};
  const size_t n       = SL_ARRAY_LEN(x);
  struct sl_span data  = sl_span_view(n, x);
  struct sl_span slice = sl_span_slice(&data, 0, SIZE_MAX);
  SL_ASSERT_TRUE(!slice.owned);
  SL_ASSERT_TRUE(slice.size == data.size);
  for (size_t i = 0; i < slice.size; ++i) {
    SL_ASSERT_TRUE(slice.data + i == data.data + i);
  }
  return true;
}

SL_TEST(test_data_find) {
  (void)ctx;
  unsigned char x1[]   = {1, 2, 3, 1, 2, 3};
  unsigned char x2[]   = {2, 3};
  const size_t n1      = SL_ARRAY_LEN(x1);
  const size_t n2      = SL_ARRAY_LEN(x2);
  struct sl_span data1 = sl_span_view(n1, x1);
  struct sl_span data2 = sl_span_view(n2, x2);
  struct sl_span empty = {0};
  SL_ASSERT_TRUE(!sl_span_find(&data2, &data1).data);
  SL_ASSERT_TRUE(!sl_span_find(&data1, &empty).data);
  SL_ASSERT_TRUE(!sl_span_find(&empty, &data1).data);
  SL_ASSERT_TRUE(sl_span_find(&data1, &data1).data == x1);
  SL_ASSERT_TRUE(sl_span_find(&data2, &data2).data == x2);
  SL_ASSERT_TRUE(sl_span_find(&data1, &data2).data == x1 + 1);
  return true;
}

SL_TEST(test_data_iter) {
  (void)ctx;
  unsigned char x[]       = {1, 2, 3, 4, 5, 6};
  const size_t n          = SL_ARRAY_LEN(x);
  struct sl_span data     = sl_span_view(n, x);
  struct sl_iterator iter = sl_span_iter(&data);
  for (size_t i = 0; i < n; ++i) {
    SL_ASSERT_TRUE(!sl_span_iter_is_done(&iter));
    SL_ASSERT_TRUE(iter.index == i);
    SL_ASSERT_TRUE(iter.pos == i);
    unsigned char* item = sl_span_iter_get(&iter);
    SL_ASSERT_TRUE(item == data.data + i);
    SL_ASSERT_TRUE(item == x + i);
    sl_span_iter_advance(&iter);
  }
  SL_ASSERT_TRUE(sl_span_iter_is_done(&iter));
  return true;
}

SL_TEST_MAIN()
