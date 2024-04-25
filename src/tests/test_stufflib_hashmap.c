#include <assert.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_span.h"

#define SL_HASHMAP_INIT_CAPACITY 2
#include "stufflib_hashmap.h"

bool test_empty(const bool verbose) {
  struct sl_hashmap map = sl_hashmap_create();
  assert(map.size == 0);
  assert(map.capacity == 2);
  assert(map.slots != nullptr);

  struct sl_span keys[] = {
      sl_span_from_str("hello"),
      sl_span_from_str(" "),
      sl_span_from_str("there"),
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(keys); ++i) {
    assert(!sl_hashmap_contains(&map, keys + i));
    assert(map.size == 0);
    assert(map.capacity == 2);
    assert(map.slots != nullptr);
  }

  sl_hashmap_delete(&map);
  for (size_t i = 0; i < SL_ARRAY_LEN(keys); ++i) {
    sl_span_delete(keys + i);
  }
  return true;
}

bool test_insert_single_element(const bool verbose) {
  struct sl_hashmap map = sl_hashmap_create();
  struct sl_span key_hello = sl_span_from_str("hello");

  assert(map.size == 0);
  assert(map.capacity == 2);
  assert(map.slots != nullptr);
  assert(!sl_hashmap_contains(&map, &key_hello));

  sl_hashmap_insert(&map, &key_hello, 1);
  assert(map.size == 1);
  assert(map.capacity == 2);
  assert(map.slots != nullptr);
  assert(map.slots[0].filled);
  assert(sl_span_compare(&(map.slots[0].key), &key_hello) == 0);
  assert(map.slots[0].value == 1);
  assert(sl_hashmap_contains(&map, &key_hello));
  assert(sl_hashmap_get(&map, &key_hello));
  assert(sl_hashmap_get(&map, &key_hello)->value == 1);

  sl_hashmap_delete(&map);
  sl_span_delete(&key_hello);
  return true;
}

bool test_insert_two_elements_resizes(const bool verbose) {
  struct sl_hashmap map = sl_hashmap_create();
  struct sl_span key_hello = sl_span_from_str("hello");
  struct sl_span key_there = sl_span_from_str("there");

  assert(map.size == 0);
  assert(map.capacity == 2);
  assert(map.slots != nullptr);
  assert(!sl_hashmap_contains(&map, &key_hello));

  sl_hashmap_insert(&map, &key_hello, 1);
  assert(map.size == 1);
  assert(map.capacity == 2);
  sl_hashmap_insert(&map, &key_there, 2);
  assert(map.size == 2);
  assert(map.capacity == 4);

  sl_hashmap_delete(&map);
  sl_span_delete(&key_hello);
  sl_span_delete(&key_there);
  return true;
}

bool test_update_single_element(const bool verbose) {
  struct sl_hashmap map = sl_hashmap_create();
  struct sl_span key_hello = sl_span_from_str("hello");

  sl_hashmap_insert(&map, &key_hello, 1);
  assert(sl_hashmap_contains(&map, &key_hello));
  assert(sl_hashmap_get(&map, &key_hello));
  assert(sl_hashmap_get(&map, &key_hello)->value == 1);

  sl_hashmap_get(&map, &key_hello)->value = 10;
  assert(sl_hashmap_get(&map, &key_hello));
  assert(sl_hashmap_get(&map, &key_hello)->value == 10);

  sl_hashmap_delete(&map);
  sl_span_delete(&key_hello);
  return true;
}

bool test_multiple_resizes_retain_slots(const bool verbose) {
  struct sl_hashmap map = sl_hashmap_create();
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
  const size_t values[] = {
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
    assert(map.size == i);
    assert(map.capacity == expected_capacities[i]);
    struct sl_span key1 = sl_span_from_str(keys[i]);
    assert(!sl_hashmap_contains(&map, &key1));
    assert(sl_hashmap_get(&map, &key1));
    assert(!(sl_hashmap_get(&map, &key1)->filled));
    for (size_t j = 0; j < i; ++j) {
      struct sl_span key2 = sl_span_from_str(keys[j]);
      assert(sl_hashmap_contains(&map, &key2));
      assert(sl_hashmap_get(&map, &key2));
      assert(sl_hashmap_get(&map, &key2)->value == values[j]);
      sl_span_delete(&key2);
    }
    sl_hashmap_insert(&map, &key1, values[i]);
    sl_span_delete(&key1);
  }
  assert(map.size == n);
  assert(map.capacity == expected_capacities[n]);
  sl_hashmap_delete(&map);
  return true;
}

bool test_slot_iterator(const bool verbose) {
  struct sl_hashmap map = sl_hashmap_create();
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
  const size_t n = SL_ARRAY_LEN(keys);
  for (size_t i = 0; i < n; ++i) {
    struct sl_span key = sl_span_from_str(keys[i]);
    sl_hashmap_insert(&map, &key, i);
    sl_span_delete(&key);
  }
  struct sl_iterator iter = sl_hashmap_iter(&map);
  for (size_t i = 0; i < n; ++i) {
    assert(!iter.is_done(&iter));
    assert(iter.pos == i);
    struct sl_hashmap_slot* slot = iter.get_item(&iter);
    assert(slot);
    assert(iter.index == slot - map.slots);
    assert(slot->filled);
    assert(slot->value >= 0);
    assert(slot->value < n);
    assert(memcmp(keys[slot->value], slot->key.data, slot->key.size) == 0);
    iter.advance(&iter);
  }
  assert(iter.is_done(&iter));
  sl_hashmap_delete(&map);
  return true;
}

SL_TEST_MAIN(test_empty,
             test_insert_single_element,
             test_insert_two_elements_resizes,
             test_update_single_element,
             test_multiple_resizes_retain_slots,
             test_slot_iterator)
