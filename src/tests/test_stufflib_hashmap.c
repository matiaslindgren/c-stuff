#include <assert.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_data.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"

#define STUFFLIB_HASHMAP_INIT_CAPACITY 2
#include "stufflib_hashmap.h"

bool test_empty(const bool verbose) {
  stufflib_hashmap map = stufflib_hashmap_create();
  assert(map.size == 0);
  assert(map.capacity == 2);
  assert(map.slots != nullptr);

  stufflib_data keys[] = {
      stufflib_data_from_str(""),
      stufflib_data_from_str("hello"),
      stufflib_data_from_str(" "),
      stufflib_data_from_str("there"),
  };
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(keys); ++i) {
    assert(!stufflib_hashmap_contains(&map, keys + i));
    assert(map.size == 0);
    assert(map.capacity == 2);
    assert(map.slots != nullptr);
  }

  stufflib_hashmap_delete(&map);
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(keys); ++i) {
    stufflib_data_delete(keys + i);
  }
  return true;
}

bool test_insert_single_element(const bool verbose) {
  stufflib_hashmap map = stufflib_hashmap_create();
  stufflib_data key_hello = stufflib_data_from_str("hello");

  assert(map.size == 0);
  assert(map.capacity == 2);
  assert(map.slots != nullptr);
  assert(!stufflib_hashmap_contains(&map, &key_hello));

  stufflib_hashmap_insert(&map, &key_hello, 1);
  assert(map.size == 1);
  assert(map.capacity == 2);
  assert(map.slots != nullptr);
  assert(map.slots[0].filled);
  assert(stufflib_data_compare(&(map.slots[0].key), &key_hello) == 0);
  assert(map.slots[0].value == 1);
  assert(stufflib_hashmap_contains(&map, &key_hello));
  assert(stufflib_hashmap_get(&map, &key_hello));
  assert(stufflib_hashmap_get(&map, &key_hello)->value == 1);

  stufflib_hashmap_delete(&map);
  stufflib_data_delete(&key_hello);
  return true;
}

bool test_insert_two_elements_resizes(const bool verbose) {
  stufflib_hashmap map = stufflib_hashmap_create();
  stufflib_data key_hello = stufflib_data_from_str("hello");
  stufflib_data key_there = stufflib_data_from_str("there");

  assert(map.size == 0);
  assert(map.capacity == 2);
  assert(map.slots != nullptr);
  assert(!stufflib_hashmap_contains(&map, &key_hello));

  stufflib_hashmap_insert(&map, &key_hello, 1);
  assert(map.size == 1);
  assert(map.capacity == 2);
  stufflib_hashmap_insert(&map, &key_there, 2);
  assert(map.size == 2);
  assert(map.capacity == 4);

  stufflib_hashmap_delete(&map);
  stufflib_data_delete(&key_hello);
  stufflib_data_delete(&key_there);
  return true;
}

bool test_update_single_element(const bool verbose) {
  stufflib_hashmap map = stufflib_hashmap_create();
  stufflib_data key_hello = stufflib_data_from_str("hello");

  stufflib_hashmap_insert(&map, &key_hello, 1);
  assert(stufflib_hashmap_contains(&map, &key_hello));
  assert(stufflib_hashmap_get(&map, &key_hello));
  assert(stufflib_hashmap_get(&map, &key_hello)->value == 1);

  stufflib_hashmap_get(&map, &key_hello)->value = 10;
  assert(stufflib_hashmap_get(&map, &key_hello));
  assert(stufflib_hashmap_get(&map, &key_hello)->value == 10);

  stufflib_hashmap_delete(&map);
  stufflib_data_delete(&key_hello);
  return true;
}

bool test_multiple_resizes_retain_slots(const bool verbose) {
  stufflib_hashmap map = stufflib_hashmap_create();
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
  const size_t n = STUFFLIB_ARRAY_LEN(keys);
  for (size_t i = 0; i < n; ++i) {
    assert(map.size == i);
    assert(map.capacity == expected_capacities[i]);
    stufflib_data key1 = stufflib_data_from_str(keys[i]);
    assert(!stufflib_hashmap_contains(&map, &key1));
    assert(stufflib_hashmap_get(&map, &key1));
    assert(!(stufflib_hashmap_get(&map, &key1)->filled));
    for (size_t j = 0; j < i; ++j) {
      stufflib_data key2 = stufflib_data_from_str(keys[j]);
      assert(stufflib_hashmap_contains(&map, &key2));
      assert(stufflib_hashmap_get(&map, &key2));
      assert(stufflib_hashmap_get(&map, &key2)->value == values[j]);
      stufflib_data_delete(&key2);
    }
    stufflib_hashmap_insert(&map, &key1, values[i]);
    stufflib_data_delete(&key1);
  }
  assert(map.size == n);
  assert(map.capacity == expected_capacities[n]);
  stufflib_hashmap_delete(&map);
  return true;
}

bool test_slot_iterator(const bool verbose) {
  stufflib_hashmap map = stufflib_hashmap_create();
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
  const size_t n = STUFFLIB_ARRAY_LEN(keys);
  for (size_t i = 0; i < n; ++i) {
    stufflib_data key = stufflib_data_from_str(keys[i]);
    stufflib_hashmap_insert(&map, &key, i);
    stufflib_data_delete(&key);
  }
  stufflib_iterator iter = stufflib_hashmap_iter(&map);
  for (size_t i = 0; i < n; ++i) {
    assert(!iter.is_done(&iter));
    assert(iter.pos == i);
    stufflib_hashmap_slot* slot = iter.get_item(&iter);
    assert(slot);
    assert(iter.index == slot - map.slots);
    assert(slot->filled);
    assert(slot->value >= 0);
    assert(slot->value < n);
    assert(strcmp(keys[slot->value], (char*)(slot->key.data)) == 0);
    iter.advance(&iter);
  }
  assert(iter.is_done(&iter));
  stufflib_hashmap_delete(&map);
  return true;
}

STUFFLIB_TEST_MAIN(test_empty,
                   test_insert_single_element,
                   test_insert_two_elements_resizes,
                   test_update_single_element,
                   test_multiple_resizes_retain_slots,
                   test_slot_iterator)
