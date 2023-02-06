#include <assert.h>
#include <string.h>

#include "stufflib_hashmap.h"
#include "stufflib_macros.h"
#include "stufflib_test.h"

int test_empty(const int verbose) {
  stufflib_hashmap* map = stufflib_hashmap_init(&(stufflib_hashmap){0}, 0);
  assert(map);
  assert(map->size == 0);
  assert(map->capacity == 0);
  assert(map->nodes == 0);

  const char* keys[] = {
      "",
      "hello",
      " ",
      "there",
  };
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(keys); ++i) {
    assert(!stufflib_hashmap_contains(map, keys[i]));
    assert(map->size == 0);
    assert(map->capacity == 0);
    assert(map->nodes == 0);
  }

  stufflib_hashmap_destroy(map);
  return 1;
}

int test_insert_single_element(const int verbose) {
  stufflib_hashmap* map = stufflib_hashmap_init(&(stufflib_hashmap){0}, 1);
  assert(map);
  assert(map->size == 0);
  assert(map->capacity == 1);
  assert(map->nodes != 0);
  assert(map->nodes->key == 0);
  assert(!stufflib_hashmap_contains(map, "hello"));

  assert(stufflib_hashmap_insert(map, "hello", 1));
  assert(map->size == 1);
  assert(map->capacity == 2);
  assert(map->nodes != 0);
  assert(map->nodes->key != 0);
  assert(strcmp(map->nodes->key, "hello") == 0);
  assert(map->nodes->value == 1);
  assert(stufflib_hashmap_contains(map, "hello"));
  assert(stufflib_hashmap_get(map, "hello"));
  assert(stufflib_hashmap_get(map, "hello")->value == 1);

  stufflib_hashmap_destroy(map);
  return 1;
}

int test_insert_two_elements_resizes(const int verbose) {
  stufflib_hashmap* map = stufflib_hashmap_init(&(stufflib_hashmap){0}, 1);
  assert(map);

  assert(map->size == 0);
  assert(map->capacity == 1);
  assert(map->nodes != 0);
  assert(map->nodes->key == 0);
  assert(!stufflib_hashmap_contains(map, "hello"));

  assert(stufflib_hashmap_insert(map, "hello", 1));
  assert(map->size == 1);
  assert(map->capacity == 2);
  assert(stufflib_hashmap_insert(map, "there", 2));
  assert(map->size == 2);
  assert(map->capacity == 4);

  stufflib_hashmap_destroy(map);
  return 1;
}

int test_update_single_element(const int verbose) {
  stufflib_hashmap* map = stufflib_hashmap_init(&(stufflib_hashmap){0}, 1);
  assert(map);

  assert(stufflib_hashmap_insert(map, "hello", 1));
  assert(stufflib_hashmap_contains(map, "hello"));
  assert(stufflib_hashmap_get(map, "hello"));
  assert(stufflib_hashmap_get(map, "hello")->value == 1);

  stufflib_hashmap_update(map, "hello", 10);
  assert(stufflib_hashmap_get(map, "hello"));
  assert(stufflib_hashmap_get(map, "hello")->value == 10);

  stufflib_hashmap_destroy(map);
  return 1;
}

int test_multiple_resizes_retain_nodes(const int verbose) {
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
    assert(!stufflib_hashmap_contains(map, keys[i]));
    assert(stufflib_hashmap_get(map, keys[i]));
    assert(!stufflib_hashmap_get(map, keys[i])->key);
    for (size_t j = 0; j < i; ++j) {
      assert(stufflib_hashmap_contains(map, keys[j]));
      assert(stufflib_hashmap_get(map, keys[j]));
      assert(stufflib_hashmap_get(map, keys[j])->value == values[j]);
    }
    assert(stufflib_hashmap_insert(map, keys[i], values[i]));
  }
  assert(map->size == n);
  assert(map->capacity == expected_capacities[n]);
  stufflib_hashmap_destroy(map);
  return 1;
}

STUFFLIB_TEST_MAIN(test_empty,
                   test_insert_single_element,
                   test_insert_two_elements_resizes,
                   test_update_single_element,
                   test_multiple_resizes_retain_nodes);
