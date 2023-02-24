#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_data.h"
#include "stufflib_macros.h"

bool test_view_one(const bool verbose) {
  unsigned char x = 1;
  stufflib_data data = stufflib_data_view(1, &x);
  assert(data.size == 1);
  assert(!data.owned);
  assert(data.data == &x);
  stufflib_data_delete(&data);
  assert(data.size == 0);
  assert(!data.owned);
  assert(data.data == nullptr);
  return true;
}

bool test_view_array(const bool verbose) {
  unsigned char x[] = {1, 2, 3, 4};
  const size_t n = STUFFLIB_ARRAY_LEN(x);
  stufflib_data data = stufflib_data_view(n, x);
  assert(data.size == n);
  assert(!data.owned);
  assert(data.data == x);
  for (size_t i = 0; i < n; ++i) {
    assert(data.data + i == x + i);
  }
  stufflib_data_delete(&data);
  assert(data.size == 0);
  assert(!data.owned);
  assert(data.data == nullptr);
  return true;
}

bool test_create(const bool verbose) {
  const size_t n = 10;
  stufflib_data data = stufflib_data_create(n);
  assert(data.size == n);
  assert(data.owned);
  assert(data.data);
  for (size_t i = 0; i < n; ++i) {
    assert(data.data[i] == 0);
  }
  stufflib_data_delete(&data);
  assert(data.size == 0);
  assert(!data.owned);
  assert(data.data == nullptr);
  return true;
}

bool test_create_empty(const bool verbose) {
  stufflib_data data = stufflib_data_create(0);
  assert(data.size == 0);
  assert(data.owned);
  assert(!data.data);
  stufflib_data_delete(&data);
  assert(data.size == 0);
  assert(!data.owned);
  assert(data.data == nullptr);
  return true;
}

bool test_copy_view(const bool verbose) {
  unsigned char x[] = {1, 2, 3, 4};
  const size_t n = STUFFLIB_ARRAY_LEN(x);
  stufflib_data data1 = stufflib_data_view(n, x);
  stufflib_data data2 = stufflib_data_copy(&data1);
  assert(!data1.owned);
  assert(data1.data == x);
  assert(data2.owned);
  assert(data2.data != x);
  for (size_t i = 0; i < n; ++i) {
    assert(data1.data + i == x + i);
    assert(data2.data + i != x + i);
    assert(data2.data[i] == x[i]);
  }
  stufflib_data_delete(&data2);
  return true;
}

bool test_concat_views(const bool verbose) {
  unsigned char x1[] = {1, 2, 3, 4};
  unsigned char x2[] = {6, 7, 8, 9, 10, 11};
  const size_t n1 = STUFFLIB_ARRAY_LEN(x1);
  const size_t n2 = STUFFLIB_ARRAY_LEN(x2);
  stufflib_data data1 = stufflib_data_view(n1, x1);
  stufflib_data data2 = stufflib_data_view(n2, x2);
  stufflib_data data3 = stufflib_data_concat(&data1, &data2);
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
  stufflib_data_delete(&data3);
  return true;
}

bool test_concat_empty(const bool verbose) {
  unsigned char x1[] = {1, 2, 3, 4};
  const size_t n1 = STUFFLIB_ARRAY_LEN(x1);
  stufflib_data data1 = stufflib_data_view(n1, x1);
  stufflib_data data2 = stufflib_data_create(0);
  stufflib_data data3 = stufflib_data_concat(&data1, &data2);
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
  stufflib_data_delete(&data3);
  return true;
}

bool test_slice(const bool verbose) {
  unsigned char x[] = {1, 2, 3, 4, 5, 6};
  const size_t n = STUFFLIB_ARRAY_LEN(x);
  stufflib_data data = stufflib_data_view(n, x);
  for (size_t begin = 0; begin < n; ++begin) {
    for (size_t end = begin; end < n; ++end) {
      stufflib_data slice = stufflib_data_slice(&data, begin, end);
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

bool test_slice_past_end(const bool verbose) {
  unsigned char x[] = {1, 2, 3, 4, 5, 6};
  const size_t n = STUFFLIB_ARRAY_LEN(x);
  stufflib_data data = stufflib_data_view(n, x);
  stufflib_data slice = stufflib_data_slice(&data, 0, SIZE_MAX);
  assert(!slice.owned);
  assert(slice.size == data.size);
  for (size_t i = 0; i < slice.size; ++i) {
    assert(slice.data + i == data.data + i);
  }
  return true;
}

bool test_iter(const bool verbose) {
  unsigned char x[] = {1, 2, 3, 4, 5, 6};
  const size_t n = STUFFLIB_ARRAY_LEN(x);
  stufflib_data data = stufflib_data_view(n, x);
  stufflib_iterator iter = stufflib_data_iter(&data);
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

STUFFLIB_TEST_MAIN(test_view_one,
                   test_view_array,
                   test_create,
                   test_create_empty,
                   test_copy_view,
                   test_concat_views,
                   test_concat_empty,
                   test_slice,
                   test_slice_past_end,
                   test_iter)
