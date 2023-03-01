#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_data.h"
#include "stufflib_macros.h"

bool test_data_view_one(const bool verbose) {
  unsigned char x = 1;
  struct sl_data data = sl_data_view(1, &x);
  assert(data.size == 1);
  assert(!data.owned);
  assert(data.data == &x);
  sl_data_delete(&data);
  assert(data.data == nullptr);
  return true;
}

bool test_data_view_array(const bool verbose) {
  unsigned char x[] = {1, 2, 3, 4};
  const size_t n = SL_ARRAY_LEN(x);
  struct sl_data data = sl_data_view(n, x);
  assert(data.size == n);
  assert(!data.owned);
  assert(data.data == x);
  for (size_t i = 0; i < n; ++i) {
    assert(data.data + i == x + i);
  }
  sl_data_delete(&data);
  assert(data.data == nullptr);
  return true;
}

bool test_data_create(const bool verbose) {
  const size_t n = 10;
  struct sl_data data = sl_data_create(n);
  assert(data.size == n);
  assert(data.owned);
  assert(data.data);
  for (size_t i = 0; i < n; ++i) {
    assert(data.data[i] == 0);
  }
  sl_data_delete(&data);
  assert(data.data == nullptr);
  return true;
}

bool test_data_create_empty(const bool verbose) {
  struct sl_data data = sl_data_create(0);
  assert(data.size == 0);
  assert(data.owned);
  assert(!data.data);
  sl_data_delete(&data);
  assert(data.data == nullptr);
  return true;
}

bool test_data_create_from_cstr(const bool verbose) {
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
    const char* cstr = strings[i];
    struct sl_data str = sl_data_from_str(cstr);
    assert(str.size == lengths[i]);
    assert(str.owned);
    if (str.size == 0) {
      assert(!str.data);
    } else {
      assert(str.data);
      assert(memcmp(str.data, cstr, str.size) == 0);
    }
    assert(!sl_data_is_hexadecimal_str(&str));
    sl_data_delete(&str);
  }
  return true;
}

bool test_data_create_from_hexadecimal_cstr(const bool verbose) {
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
    const char* cstr = strings[i];
    struct sl_data str = sl_data_from_str(cstr);
    assert(str.size == str_lengths[i]);
    assert(str.owned);
    assert(str.data);
    assert(memcmp(str.data, cstr, str.size) == 0);
    assert(sl_data_is_hexadecimal_str(&str));
    struct sl_data num = sl_data_parse_hex(&str);
    assert(num.size == data_lengths[i]);
    assert(num.owned);
    assert(num.data);
    assert(memcmp(num.data, bytes[i], num.size) == 0);
    sl_data_delete(&num);
    sl_data_delete(&str);
  }
  return true;
}

bool test_data_copy_view(const bool verbose) {
  unsigned char x[] = {1, 2, 3, 4};
  const size_t n = SL_ARRAY_LEN(x);
  struct sl_data data1 = sl_data_view(n, x);
  struct sl_data data2 = sl_data_copy(&data1);
  assert(!data1.owned);
  assert(data1.data == x);
  assert(data2.owned);
  assert(data2.data != x);
  for (size_t i = 0; i < n; ++i) {
    assert(data1.data + i == x + i);
    assert(data2.data + i != x + i);
    assert(data2.data[i] == x[i]);
  }
  sl_data_delete(&data2);
  assert(data2.data == nullptr);
  return true;
}

bool test_data_concat_views(const bool verbose) {
  unsigned char x1[] = {1, 2, 3, 4};
  unsigned char x2[] = {6, 7, 8, 9, 10, 11};
  const size_t n1 = SL_ARRAY_LEN(x1);
  const size_t n2 = SL_ARRAY_LEN(x2);
  struct sl_data data1 = sl_data_view(n1, x1);
  struct sl_data data2 = sl_data_view(n2, x2);
  struct sl_data data3 = sl_data_concat(&data1, &data2);
  assert(!data1.owned);
  assert(!data2.owned);
  assert(data3.owned);
  assert(data1.data == x1);
  assert(data2.data == x2);
  assert(data3.data != x1);
  assert(data3.data != x2);
  assert(data3.size == data1.size + data2.size);
  for (size_t i = 0; i < n1; ++i) {
    assert(data1.data + i != data3.data + i);
    assert(data1.data[i] == data3.data[i]);
  }
  for (size_t i = 0; i < n2; ++i) {
    assert(data2.data + i != data3.data + n1 + i);
    assert(data2.data[i] == data3.data[n1 + i]);
  }
  sl_data_delete(&data3);
  assert(data3.data == nullptr);
  return true;
}

bool test_data_concat_empty(const bool verbose) {
  unsigned char x1[] = {1, 2, 3, 4};
  const size_t n1 = SL_ARRAY_LEN(x1);
  struct sl_data data1 = sl_data_view(n1, x1);
  struct sl_data data2 = sl_data_create(0);
  struct sl_data data3 = sl_data_concat(&data1, &data2);
  assert(!data1.owned);
  assert(data2.owned);
  assert(data3.owned);
  assert(data1.data == x1);
  assert(data2.data == nullptr);
  assert(data3.data != x1);
  assert(data3.size == data1.size);
  for (size_t i = 0; i < n1; ++i) {
    assert(data1.data + i != data3.data + i);
    assert(data1.data[i] == data3.data[i]);
  }
  sl_data_delete(&data3);
  assert(data3.data == nullptr);
  return true;
}

bool test_data_slice(const bool verbose) {
  unsigned char x[] = {1, 2, 3, 4, 5, 6};
  const size_t n = SL_ARRAY_LEN(x);
  struct sl_data data = sl_data_view(n, x);
  for (size_t begin = 0; begin < n; ++begin) {
    for (size_t end = begin; end < n; ++end) {
      struct sl_data slice = sl_data_slice(&data, begin, end);
      assert(!slice.owned);
      assert(slice.size == end - begin);
      if (!slice.size) {
        assert(!slice.data);
      }
      for (size_t i = 0; i < slice.size; ++i) {
        assert(slice.data + i == data.data + begin + i);
      }
    }
  }
  return true;
}

bool test_data_slice_past_end(const bool verbose) {
  unsigned char x[] = {1, 2, 3, 4, 5, 6};
  const size_t n = SL_ARRAY_LEN(x);
  struct sl_data data = sl_data_view(n, x);
  struct sl_data slice = sl_data_slice(&data, 0, SIZE_MAX);
  assert(!slice.owned);
  assert(slice.size == data.size);
  for (size_t i = 0; i < slice.size; ++i) {
    assert(slice.data + i == data.data + i);
  }
  return true;
}

bool test_data_find(const bool verbose) {
  unsigned char x1[] = {1, 2, 3, 1, 2, 3};
  unsigned char x2[] = {2, 3};
  const size_t n1 = SL_ARRAY_LEN(x1);
  const size_t n2 = SL_ARRAY_LEN(x2);
  struct sl_data data1 = sl_data_view(n1, x1);
  struct sl_data data2 = sl_data_view(n2, x2);
  struct sl_data empty = {0};
  assert(!sl_data_find(&data2, &data1).data);
  assert(!sl_data_find(&data1, &empty).data);
  assert(!sl_data_find(&empty, &data1).data);
  assert(sl_data_find(&data1, &data1).data == x1);
  assert(sl_data_find(&data2, &data2).data == x2);
  assert(sl_data_find(&data1, &data2).data == x1 + 1);
  return true;
}

bool test_data_iter(const bool verbose) {
  unsigned char x[] = {1, 2, 3, 4, 5, 6};
  const size_t n = SL_ARRAY_LEN(x);
  struct sl_data data = sl_data_view(n, x);
  struct sl_iterator iter = sl_data_iter(&data);
  for (size_t i = 0; i < n; ++i) {
    assert(!iter.is_done(&iter));
    assert(iter.index == i);
    assert(iter.pos == i);
    unsigned char* item = iter.get_item(&iter);
    assert(item == data.data + i);
    assert(item == x + i);
    iter.advance(&iter);
  }
  assert(iter.is_done(&iter));
  return true;
}

SL_TEST_MAIN(test_data_view_one,
             test_data_view_array,
             test_data_create,
             test_data_create_empty,
             test_data_create_from_cstr,
             test_data_create_from_hexadecimal_cstr,
             test_data_copy_view,
             test_data_concat_views,
             test_data_concat_empty,
             test_data_slice,
             test_data_slice_past_end,
             test_data_find,
             test_data_iter)
