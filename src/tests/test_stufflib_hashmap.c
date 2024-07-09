#include <assert.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_hashmap.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_span.h"

bool test_empty(const bool) {
  struct sl_hashmap map = sl_hashmap_create(2);
  SL_ASSERT_EQ_LL(map.size, 0);
  SL_ASSERT_EQ_LL(map.capacity, 2);
  assert(map.slots != nullptr);

  struct sl_span keys[] = {
      sl_span_from_str("hello"),
      sl_span_from_str(" "),
      sl_span_from_str("there"),
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(keys); ++i) {
    assert(!sl_hashmap_contains(&map, keys + i));
    SL_ASSERT_EQ_LL(map.size, 0);
    SL_ASSERT_EQ_LL(map.capacity, 2);
    assert(map.slots != nullptr);
  }

  sl_hashmap_delete(&map);
  for (size_t i = 0; i < SL_ARRAY_LEN(keys); ++i) {
    sl_span_delete(keys + i);
  }
  return true;
}

bool test_insert_single_int(const bool) {
  for (int64_t value = -10; value <= 10; ++value) {
    struct sl_hashmap map = sl_hashmap_create(2);
    struct sl_span key_hello = sl_span_from_str("hello");

    SL_ASSERT_EQ_LL(map.size, 0);
    SL_ASSERT_EQ_LL(map.capacity, 2);
    assert(map.slots != nullptr);
    assert(!sl_hashmap_contains(&map, &key_hello));

    sl_hashmap_insert(&map, &key_hello, sl_hashmap_type_int64, &value);
    SL_ASSERT_EQ_LL(map.size, 1);
    SL_ASSERT_EQ_LL(map.capacity, 2);
    assert(map.slots != nullptr);
    assert(map.slots[0].type == sl_hashmap_type_int64);
    SL_ASSERT_EQ_LL(sl_span_compare(&(map.slots[0].key), &key_hello), 0);
    SL_ASSERT_EQ_LL(map.slots[0].value.int64, value);
    assert(sl_hashmap_contains(&map, &key_hello));
    assert(sl_hashmap_get(&map, &key_hello));
    SL_ASSERT_EQ_LL(sl_hashmap_get(&map, &key_hello)->value.int64, value);

    sl_hashmap_delete(&map);
    sl_span_delete(&key_hello);
  }
  for (uint64_t value = 0; value <= 20; ++value) {
    struct sl_hashmap map = sl_hashmap_create(2);
    struct sl_span key = sl_span_from_str("hello");

    SL_ASSERT_EQ_LL(map.size, 0);
    SL_ASSERT_EQ_LL(map.capacity, 2);
    assert(map.slots != nullptr);
    assert(!sl_hashmap_contains(&map, &key));

    sl_hashmap_insert(&map, &key, sl_hashmap_type_uint64, &value);
    SL_ASSERT_EQ_LL(map.size, 1);
    SL_ASSERT_EQ_LL(map.capacity, 2);
    assert(map.slots != nullptr);
    assert(map.slots[0].type == sl_hashmap_type_uint64);
    SL_ASSERT_EQ_LL(sl_span_compare(&(map.slots[0].key), &key), 0);
    SL_ASSERT_EQ_LL(map.slots[0].value.uint64, value);
    assert(sl_hashmap_contains(&map, &key));
    assert(sl_hashmap_get(&map, &key));
    SL_ASSERT_EQ_LL(sl_hashmap_get(&map, &key)->value.uint64, value);

    sl_hashmap_delete(&map);
    sl_span_delete(&key);
  }
  return true;
}

bool test_insert_single_pointer(const bool) {
  struct sl_hashmap map = sl_hashmap_create(2);
  struct sl_span key = sl_span_from_str("hello");
  struct sl_span value = sl_span_from_str("there");

  SL_ASSERT_EQ_LL(map.size, 0);
  SL_ASSERT_EQ_LL(map.capacity, 2);
  assert(map.slots != nullptr);
  assert(!sl_hashmap_contains(&map, &key));

  sl_hashmap_insert(&map, &key, sl_hashmap_type_any, &value);
  SL_ASSERT_EQ_LL(map.size, 1);
  SL_ASSERT_EQ_LL(map.capacity, 2);
  assert(map.slots != nullptr);
  assert(map.slots[0].type == sl_hashmap_type_any);
  SL_ASSERT_EQ_LL(sl_span_compare(&(map.slots[0].key), &key), 0);
  SL_ASSERT_EQ_PTR(map.slots[0].value.any, &value);
  assert(sl_hashmap_contains(&map, &key));
  assert(sl_hashmap_get(&map, &key));
  SL_ASSERT_EQ_PTR(sl_hashmap_get(&map, &key)->value.any, &value);

  sl_hashmap_delete(&map);
  sl_span_delete(&key);
  sl_span_delete(&value);

  return true;
}

bool test_insert_two_elements_resizes(const bool) {
  struct sl_hashmap map = sl_hashmap_create(2);
  struct sl_span key_hello = sl_span_from_str("hello");
  struct sl_span key_there = sl_span_from_str("there");

  SL_ASSERT_EQ_LL(map.size, 0);
  SL_ASSERT_EQ_LL(map.capacity, 2);
  assert(map.slots != nullptr);
  assert(!sl_hashmap_contains(&map, &key_hello));

  sl_hashmap_insert(&map, &key_hello, sl_hashmap_type_int64, &((int64_t){1}));
  SL_ASSERT_EQ_LL(map.size, 1);
  SL_ASSERT_EQ_LL(map.capacity, 2);
  sl_hashmap_insert(&map, &key_there, sl_hashmap_type_int64, &((int64_t){2}));
  SL_ASSERT_EQ_LL(map.size, 2);
  SL_ASSERT_EQ_LL(map.capacity, 4);

  sl_hashmap_delete(&map);
  sl_span_delete(&key_hello);
  sl_span_delete(&key_there);
  return true;
}

bool test_update_single_element(const bool) {
  struct sl_hashmap map = sl_hashmap_create(2);
  struct sl_span key = sl_span_from_str("hello");

  {
    uint64_t value = 0x123456789;
    sl_hashmap_insert(&map, &key, sl_hashmap_type_uint64, &value);
    assert(sl_hashmap_contains(&map, &key));
    assert(sl_hashmap_get(&map, &key));
    SL_ASSERT_EQ_LL(sl_hashmap_get(&map, &key)->value.uint64, value);
  }

  {
    uint64_t value = 0x987654321;
    sl_hashmap_get(&map, &key)->value.uint64 = value;
    assert(sl_hashmap_get(&map, &key));
    SL_ASSERT_EQ_LL(sl_hashmap_get(&map, &key)->value.uint64, value);
    assert(sl_hashmap_get(&map, &key)->type == sl_hashmap_type_uint64);
  }

  sl_hashmap_delete(&map);
  sl_span_delete(&key);
  return true;
}

bool test_multiple_resizes_retain_slots(const bool) {
  struct sl_hashmap map = sl_hashmap_create(2);
  const char* keys[] = {
      "ten",
      "eleven",
      "twelve",
      "thirteen",
      "fourteen",
      "fifteen",
      "sixteen",
      "seventeen",
      "eighteen",
      "nineteen",
  };
  int64_t values[] = {
      10,
      11,
      12,
      13,
      14,
      15,
      16,
      17,
      18,
      19,
  };
  const size_t expected_capacities[] = {
      2,
      2,
      4,
      8,
      8,
      16,
      16,
      16,
      16,
      32,
      32,
  };
  const size_t n = SL_ARRAY_LEN(keys);
  for (size_t i = 0; i < n; ++i) {
    SL_ASSERT_EQ_LL(map.size, i);
    SL_ASSERT_EQ_LL(map.capacity, expected_capacities[i]);
    struct sl_span key1 = sl_span_from_str(keys[i]);
    assert(!sl_hashmap_contains(&map, &key1));
    assert(sl_hashmap_get(&map, &key1));
    assert(sl_hashmap_get(&map, &key1)->type == sl_hashmap_type_empty);
    for (size_t j = 0; j < i; ++j) {
      struct sl_span key2 = sl_span_from_str(keys[j]);
      assert(sl_hashmap_contains(&map, &key2));
      assert(sl_hashmap_get(&map, &key2));
      SL_ASSERT_EQ_LL(sl_hashmap_get(&map, &key2)->value.int64, values[j]);
      sl_span_delete(&key2);
    }
    sl_hashmap_insert(&map, &key1, sl_hashmap_type_int64, values + i);
    sl_span_delete(&key1);
  }
  SL_ASSERT_EQ_LL(map.size, n);
  SL_ASSERT_EQ_LL(map.capacity, expected_capacities[n]);
  sl_hashmap_delete(&map);
  return true;
}

bool test_slot_iterator(const bool) {
  struct sl_hashmap map = sl_hashmap_create(2);
  const char* keys[] = {
      "ten",
      "eleven",
      "twelve",
      "thirteen",
      "fourteen",
      "fifteen",
      "sixteen",
      "seventeen",
      "eighteen",
      "nineteen",
  };
  uint64_t values[SL_ARRAY_LEN(keys)] = {};
  const size_t n = SL_ARRAY_LEN(keys);
  for (uint64_t i = 0; i < n; ++i) {
    values[i] = i;
    struct sl_span key = sl_span_from_str(keys[i]);
    sl_hashmap_insert(&map, &key, sl_hashmap_type_uint64, values + i);
    sl_span_delete(&key);
  }
  struct sl_iterator iter = sl_hashmap_iter(&map);
  for (size_t i = 0; i < n; ++i) {
    assert(!sl_hashmap_iter_is_done(&iter));
    SL_ASSERT_EQ_LL(iter.pos, i);
    struct sl_hashmap_slot* slot = sl_hashmap_iter_get(&iter);
    assert(slot);
    SL_ASSERT_EQ_LL((long)iter.index, slot - map.slots);
    assert(slot->type == sl_hashmap_type_uint64);
    assert(slot->value.uint64 >= 0);
    assert(slot->value.uint64 < n);
    SL_ASSERT_EQ_LL(
        memcmp(keys[slot->value.uint64], slot->key.data, slot->key.size),
        0);
    sl_hashmap_iter_advance(&iter);
  }
  assert(sl_hashmap_iter_is_done(&iter));
  sl_hashmap_delete(&map);
  return true;
}

SL_TEST_MAIN(test_empty,
             test_insert_single_int,
             test_insert_single_pointer,
             test_insert_two_elements_resizes,
             test_update_single_element,
             test_multiple_resizes_retain_slots,
             test_slot_iterator)
