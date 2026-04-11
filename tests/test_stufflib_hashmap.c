#include <string.h>
#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/hashmap/hashmap.h>
#include <stufflib/iterator/iterator.h>
#include <stufflib/macros/macros.h>
#include <stufflib/span/span.h>
#include <stufflib/testing/testing.h>

SL_TEST(test_empty) {
  (void)verbose;
  struct sl_hashmap map = sl_hashmap_create(ctx, 2);
  SL_ASSERT_EQ_LL(map.size, 0);
  SL_ASSERT_EQ_LL(map.capacity, 2);
  SL_ASSERT_TRUE(map.slots != nullptr);

  struct sl_span keys[] = {
      sl_span_from_str(ctx, "hello"),
      sl_span_from_str(ctx, " "),
      sl_span_from_str(ctx, "there"),
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(keys); ++i) {
    SL_ASSERT_TRUE(!sl_hashmap_contains(ctx, &map, keys + i));
    SL_ASSERT_EQ_LL(map.size, 0);
    SL_ASSERT_EQ_LL(map.capacity, 2);
    SL_ASSERT_TRUE(map.slots != nullptr);
  }

  sl_hashmap_destroy(&map);
  for (size_t i = 0; i < SL_ARRAY_LEN(keys); ++i) {
    sl_span_destroy(keys + i);
  }
  return true;
}

SL_TEST(test_insert_single_int) {
  (void)verbose;
  for (int64_t value = -10; value <= 10; ++value) {
    struct sl_hashmap map    = sl_hashmap_create(ctx, 2);
    struct sl_span key_hello = sl_span_from_str(ctx, "hello");

    SL_ASSERT_EQ_LL(map.size, 0);
    SL_ASSERT_EQ_LL(map.capacity, 2);
    SL_ASSERT_TRUE(map.slots != nullptr);
    SL_ASSERT_TRUE(!sl_hashmap_contains(ctx, &map, &key_hello));

    SL_ASSERT_TRUE(sl_hashmap_insert(ctx, &map, &key_hello, sl_hashmap_type_int64, &value));
    SL_ASSERT_EQ_LL(map.size, 1);
    SL_ASSERT_EQ_LL(map.capacity, 2);
    SL_ASSERT_TRUE(map.slots != nullptr);
    SL_ASSERT_TRUE(map.slots[0].type == sl_hashmap_type_int64);
    SL_ASSERT_EQ_LL(sl_span_compare(&(map.slots[0].key), &key_hello), 0);
    SL_ASSERT_EQ_LL(map.slots[0].value.int64, value);
    SL_ASSERT_TRUE(sl_hashmap_contains(ctx, &map, &key_hello));
    SL_ASSERT_TRUE(sl_hashmap_get(ctx, &map, &key_hello));
    SL_ASSERT_EQ_LL(sl_hashmap_get(ctx, &map, &key_hello)->value.int64, value);

    sl_hashmap_destroy(&map);
    sl_span_destroy(&key_hello);
  }
  for (uint64_t value = 0; value <= 20; ++value) {
    struct sl_hashmap map = sl_hashmap_create(ctx, 2);
    struct sl_span key    = sl_span_from_str(ctx, "hello");

    SL_ASSERT_EQ_LL(map.size, 0);
    SL_ASSERT_EQ_LL(map.capacity, 2);
    SL_ASSERT_TRUE(map.slots != nullptr);
    SL_ASSERT_TRUE(!sl_hashmap_contains(ctx, &map, &key));

    SL_ASSERT_TRUE(sl_hashmap_insert(ctx, &map, &key, sl_hashmap_type_uint64, &value));
    SL_ASSERT_EQ_LL(map.size, 1);
    SL_ASSERT_EQ_LL(map.capacity, 2);
    SL_ASSERT_TRUE(map.slots != nullptr);
    SL_ASSERT_TRUE(map.slots[0].type == sl_hashmap_type_uint64);
    SL_ASSERT_EQ_LL(sl_span_compare(&(map.slots[0].key), &key), 0);
    SL_ASSERT_EQ_LL(map.slots[0].value.uint64, value);
    SL_ASSERT_TRUE(sl_hashmap_contains(ctx, &map, &key));
    SL_ASSERT_TRUE(sl_hashmap_get(ctx, &map, &key));
    SL_ASSERT_EQ_LL(sl_hashmap_get(ctx, &map, &key)->value.uint64, value);

    sl_hashmap_destroy(&map);
    sl_span_destroy(&key);
  }
  return true;
}

SL_TEST(test_insert_single_pointer) {
  (void)verbose;
  struct sl_hashmap map = sl_hashmap_create(ctx, 2);
  struct sl_span key    = sl_span_from_str(ctx, "hello");
  struct sl_span value  = sl_span_from_str(ctx, "there");

  SL_ASSERT_EQ_LL(map.size, 0);
  SL_ASSERT_EQ_LL(map.capacity, 2);
  SL_ASSERT_TRUE(map.slots != nullptr);
  SL_ASSERT_TRUE(!sl_hashmap_contains(ctx, &map, &key));

  SL_ASSERT_TRUE(sl_hashmap_insert(ctx, &map, &key, sl_hashmap_type_any, &value));
  SL_ASSERT_EQ_LL(map.size, 1);
  SL_ASSERT_EQ_LL(map.capacity, 2);
  SL_ASSERT_TRUE(map.slots != nullptr);
  SL_ASSERT_TRUE(map.slots[0].type == sl_hashmap_type_any);
  SL_ASSERT_EQ_LL(sl_span_compare(&(map.slots[0].key), &key), 0);
  SL_ASSERT_EQ_PTR(map.slots[0].value.any, &value);
  SL_ASSERT_TRUE(sl_hashmap_contains(ctx, &map, &key));
  SL_ASSERT_TRUE(sl_hashmap_get(ctx, &map, &key));
  SL_ASSERT_EQ_PTR(sl_hashmap_get(ctx, &map, &key)->value.any, &value);

  sl_hashmap_destroy(&map);
  sl_span_destroy(&key);
  sl_span_destroy(&value);

  return true;
}

SL_TEST(test_insert_two_elements_resizes) {
  (void)verbose;
  struct sl_hashmap map    = sl_hashmap_create(ctx, 2);
  struct sl_span key_hello = sl_span_from_str(ctx, "hello");
  struct sl_span key_there = sl_span_from_str(ctx, "there");

  SL_ASSERT_EQ_LL(map.size, 0);
  SL_ASSERT_EQ_LL(map.capacity, 2);
  SL_ASSERT_TRUE(map.slots != nullptr);
  SL_ASSERT_TRUE(!sl_hashmap_contains(ctx, &map, &key_hello));

  SL_ASSERT_TRUE(sl_hashmap_insert(ctx, &map, &key_hello, sl_hashmap_type_int64, &((int64_t){1})));
  SL_ASSERT_EQ_LL(map.size, 1);
  SL_ASSERT_EQ_LL(map.capacity, 2);
  SL_ASSERT_TRUE(sl_hashmap_insert(ctx, &map, &key_there, sl_hashmap_type_int64, &((int64_t){2})));
  SL_ASSERT_EQ_LL(map.size, 2);
  SL_ASSERT_EQ_LL(map.capacity, 4);

  sl_hashmap_destroy(&map);
  sl_span_destroy(&key_hello);
  sl_span_destroy(&key_there);
  return true;
}

SL_TEST(test_update_single_element) {
  (void)verbose;
  struct sl_hashmap map = sl_hashmap_create(ctx, 2);
  struct sl_span key    = sl_span_from_str(ctx, "hello");

  {
    uint64_t value = 0x123456789;
    SL_ASSERT_TRUE(sl_hashmap_insert(ctx, &map, &key, sl_hashmap_type_uint64, &value));
    SL_ASSERT_TRUE(sl_hashmap_contains(ctx, &map, &key));
    SL_ASSERT_TRUE(sl_hashmap_get(ctx, &map, &key));
    SL_ASSERT_EQ_LL(sl_hashmap_get(ctx, &map, &key)->value.uint64, value);
  }

  {
    uint64_t value                = 0x987654321;
    struct sl_hashmap_slot* slot2 = sl_hashmap_get(ctx, &map, &key);
    SL_ASSERT_TRUE(slot2);
    slot2->value.uint64 = value;
    SL_ASSERT_TRUE(sl_hashmap_get(ctx, &map, &key));
    SL_ASSERT_EQ_LL(sl_hashmap_get(ctx, &map, &key)->value.uint64, value);
    SL_ASSERT_TRUE(sl_hashmap_get(ctx, &map, &key)->type == sl_hashmap_type_uint64);
  }

  sl_hashmap_destroy(&map);
  sl_span_destroy(&key);
  return true;
}

SL_TEST(test_multiple_resizes_retain_slots) {
  (void)verbose;
  struct sl_hashmap map = sl_hashmap_create(ctx, 2);
  const char* keys[]    = {
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
    struct sl_span key1 = sl_span_from_str(ctx, keys[i]);
    SL_ASSERT_TRUE(!sl_hashmap_contains(ctx, &map, &key1));
    SL_ASSERT_TRUE(sl_hashmap_get(ctx, &map, &key1));
    SL_ASSERT_TRUE(sl_hashmap_get(ctx, &map, &key1)->type == sl_hashmap_type_empty);
    for (size_t j = 0; j < i; ++j) {
      struct sl_span key2 = sl_span_from_str(ctx, keys[j]);
      SL_ASSERT_TRUE(sl_hashmap_contains(ctx, &map, &key2));
      SL_ASSERT_TRUE(sl_hashmap_get(ctx, &map, &key2));
      SL_ASSERT_EQ_LL(sl_hashmap_get(ctx, &map, &key2)->value.int64, values[j]);
      sl_span_destroy(&key2);
    }
    SL_ASSERT_TRUE(sl_hashmap_insert(ctx, &map, &key1, sl_hashmap_type_int64, values + i));
    sl_span_destroy(&key1);
  }
  SL_ASSERT_EQ_LL(map.size, n);
  SL_ASSERT_EQ_LL(map.capacity, expected_capacities[n]);
  sl_hashmap_destroy(&map);
  return true;
}

SL_TEST(test_slot_iterator) {
  (void)verbose;
  struct sl_hashmap map = sl_hashmap_create(ctx, 2);
  const char* keys[]    = {
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
  const size_t n                      = SL_ARRAY_LEN(keys);
  for (uint64_t i = 0; i < n; ++i) {
    values[i]          = i;
    struct sl_span key = sl_span_from_str(ctx, keys[i]);
    SL_ASSERT_TRUE(sl_hashmap_insert(ctx, &map, &key, sl_hashmap_type_uint64, values + i));
    sl_span_destroy(&key);
  }
  struct sl_iterator iter = sl_hashmap_iter(&map);
  for (size_t i = 0; i < n; ++i) {
    SL_ASSERT_TRUE(!sl_hashmap_iter_is_done(&iter));
    SL_ASSERT_EQ_LL(iter.pos, i);
    struct sl_hashmap_slot* slot = sl_hashmap_iter_get(&iter);
    SL_ASSERT_TRUE(slot);
    SL_ASSERT_EQ_LL((long)iter.index, slot - map.slots);
    SL_ASSERT_TRUE(slot->type == sl_hashmap_type_uint64);
    SL_ASSERT_TRUE(slot->value.uint64 >= 0);
    SL_ASSERT_TRUE(slot->value.uint64 < n);
    SL_ASSERT_EQ_LL(memcmp(keys[slot->value.uint64], slot->key.data, slot->key.size), 0);
    sl_hashmap_iter_advance(&iter);
  }
  SL_ASSERT_TRUE(sl_hashmap_iter_is_done(&iter));
  sl_hashmap_destroy(&map);
  return true;
}

SL_TEST_MAIN()
