#include <assert.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_data.h"
#include "stufflib_hashmap.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"

bool test_empty(const bool verbose) {
  stufflib_hashmap* map = stufflib_hashmap_init(&(stufflib_hashmap){0}, 0);
  assert(map);
  assert(map->size == 0);
  assert(map->capacity == 0);
  assert(map->nodes == 0);

  stufflib_data keys[] = {
      stufflib_data_view(1, (unsigned char*)""),
      stufflib_data_view(6, (unsigned char*)"hello"),
      stufflib_data_view(2, (unsigned char*)" "),
      stufflib_data_view(6, (unsigned char*)"there"),
  };
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(keys); ++i) {
    assert(!stufflib_hashmap_contains(map, keys + i));
    assert(map->size == 0);
    assert(map->capacity == 0);
    assert(map->nodes == 0);
  }

  stufflib_hashmap_destroy(map);
  return true;
}

bool test_insert_single_element(const bool verbose) {
  stufflib_hashmap* map = stufflib_hashmap_init(&(stufflib_hashmap){0}, 1);
  stufflib_data key_hello = stufflib_data_view(6, (unsigned char*)"hello");

  assert(map);
  assert(map->size == 0);
  assert(map->capacity == 1);
  assert(map->nodes != 0);
  assert(!map->nodes[0].has_value);

  assert(!stufflib_hashmap_contains(map, &key_hello));

  assert(stufflib_hashmap_insert(map, &key_hello, 1));
  assert(map->size == 1);
  assert(map->capacity == 2);
  assert(map->nodes != 0);
  assert(map->nodes[0].has_value);
  assert(stufflib_data_compare(&(map->nodes[0].key), &key_hello) == 0);
  assert(map->nodes[0].value == 1);
  assert(stufflib_hashmap_contains(map, &key_hello));
  assert(stufflib_hashmap_get(map, &key_hello));
  assert(stufflib_hashmap_get(map, &key_hello)->value == 1);

  stufflib_hashmap_destroy(map);
  return true;
}

bool test_insert_two_elements_resizes(const bool verbose) {
  stufflib_hashmap* map = stufflib_hashmap_init(&(stufflib_hashmap){0}, 1);
  stufflib_data key_hello = stufflib_data_view(6, (unsigned char*)"hello");
  stufflib_data key_there = stufflib_data_view(6, (unsigned char*)"there");

  assert(map);
  assert(map->size == 0);
  assert(map->capacity == 1);
  assert(map->nodes != 0);
  assert(!map->nodes[0].has_value);
  assert(!stufflib_hashmap_contains(map, &key_hello));

  assert(stufflib_hashmap_insert(map, &key_hello, 1));
  assert(map->size == 1);
  assert(map->capacity == 2);
  assert(stufflib_hashmap_insert(map, &key_there, 2));
  assert(map->size == 2);
  assert(map->capacity == 4);

  stufflib_hashmap_destroy(map);
  return true;
}

bool test_update_single_element(const bool verbose) {
  stufflib_hashmap* map = stufflib_hashmap_init(&(stufflib_hashmap){0}, 1);
  stufflib_data key_hello = stufflib_data_view(6, (unsigned char*)"hello");

  assert(map);

  assert(stufflib_hashmap_insert(map, &key_hello, 1));
  assert(stufflib_hashmap_contains(map, &key_hello));
  assert(stufflib_hashmap_get(map, &key_hello));
  assert(stufflib_hashmap_get(map, &key_hello)->value == 1);

  stufflib_hashmap_get(map, &key_hello)->value = 10;
  assert(stufflib_hashmap_get(map, &key_hello));
  assert(stufflib_hashmap_get(map, &key_hello)->value == 10);

  stufflib_hashmap_destroy(map);
  return true;
}

bool test_multiple_resizes_retain_nodes(const bool verbose) {
  stufflib_hashmap* map = stufflib_hashmap_init(&(stufflib_hashmap){0}, 1);
  assert(map);
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
      1,
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
    assert(map->size == i);
    assert(map->capacity == expected_capacities[i]);
    stufflib_data key1 =
        stufflib_data_view(strlen(keys[i]) + 1, (unsigned char*)(keys[i]));
    assert(!stufflib_hashmap_contains(map, &key1));
    assert(stufflib_hashmap_get(map, &key1));
    assert(!(stufflib_hashmap_get(map, &key1)->has_value));
    for (size_t j = 0; j < i; ++j) {
      stufflib_data key2 =
          stufflib_data_view(strlen(keys[j]) + 1, (unsigned char*)(keys[j]));
      assert(stufflib_hashmap_contains(map, &key2));
      assert(stufflib_hashmap_get(map, &key2));
      assert(stufflib_hashmap_get(map, &key2)->value == values[j]);
    }
    assert(stufflib_hashmap_insert(map, &key1, values[i]));
  }
  assert(map->size == n);
  assert(map->capacity == expected_capacities[n]);
  stufflib_hashmap_destroy(map);
  return true;
}

bool test_node_iterator(const bool verbose) {
  stufflib_hashmap* map = stufflib_hashmap_init(&(stufflib_hashmap){0}, 1);
  assert(map);
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
    stufflib_data key =
        stufflib_data_view(strlen(keys[i]) + 1, (unsigned char*)(keys[i]));
    stufflib_hashmap_insert(map, &key, i);
  }
  stufflib_iterator iter = stufflib_hashmap_iter(map);
  for (size_t i = 0; i < n; ++i) {
    assert(!iter.is_done(&iter));
    assert(iter.pos == i);
    stufflib_hashmap_node* node = iter.get_item(&iter);
    assert(node);
    assert(iter.index == node - map->nodes);
    assert(node->has_value);
    assert(node->value >= 0);
    assert(node->value < n);
    assert((void*)(keys[node->value]) == (void*)(node->key.data));
    assert(strcmp(keys[node->value], (char*)(node->key.data)) == 0);
    iter.advance(&iter);
  }
  assert(iter.is_done(&iter));
  stufflib_hashmap_destroy(map);
  return true;
}

STUFFLIB_TEST_MAIN(test_empty,
                   test_insert_single_element,
                   test_insert_two_elements_resizes,
                   test_update_single_element,
                   test_multiple_resizes_retain_nodes,
                   test_node_iterator)
