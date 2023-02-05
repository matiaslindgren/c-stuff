#include <assert.h>
#include <string.h>

#include "stufflib_hashmap.h"
#include "stufflib_macros.h"
#include "stufflib_test.h"

int test_empty(const int verbose) {
  stufflib_hashmap map = {0};
  stufflib_hashmap_init(&map, 0);
  assert(map.size == 0);
  assert(map.capacity == 0);
  assert(map.nodes == 0);

  const char* keys[] = {
      "",
      "hello",
      " ",
      "there",
  };
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(keys); ++i) {
    assert(!stufflib_hashmap_contains(&map, keys[i]));
    assert(map.size == 0);
    assert(map.capacity == 0);
    assert(map.nodes == 0);
  }

  stufflib_hashmap_destroy(&map);
  return 1;
}

int test_insert_single_element(const int verbose) {
  stufflib_hashmap map = {0};
  stufflib_hashmap_init(&map, 1);
  assert(map.size == 0);
  assert(map.capacity == 1);
  assert(map.nodes != 0);
  assert(map.nodes->key == 0);
  assert(!stufflib_hashmap_contains(&map, "hello"));

  assert(stufflib_hashmap_insert(&map, "hello", 1));
  assert(map.size == 1);
  assert(map.capacity == 1);
  assert(map.nodes != 0);
  assert(map.nodes->key != 0);
  assert(strcmp(map.nodes->key, "hello") == 0);
  assert(map.nodes->value == 1);
  assert(stufflib_hashmap_contains(&map, "hello"));
  assert(stufflib_hashmap_get(&map, "hello"));
  assert(stufflib_hashmap_get(&map, "hello")->value == 1);

  stufflib_hashmap_destroy(&map);
  return 1;
}

int test_update_single_element(const int verbose) {
  stufflib_hashmap map = {0};
  stufflib_hashmap_init(&map, 1);

  assert(stufflib_hashmap_insert(&map, "hello", 1));
  assert(stufflib_hashmap_contains(&map, "hello"));
  assert(stufflib_hashmap_get(&map, "hello"));
  assert(stufflib_hashmap_get(&map, "hello")->value == 1);

  stufflib_hashmap_update(&map, "hello", 10);
  assert(stufflib_hashmap_get(&map, "hello"));
  assert(stufflib_hashmap_get(&map, "hello")->value == 10);

  stufflib_hashmap_destroy(&map);
  return 1;
}

int test_single_collision(const int verbose) {
  stufflib_hashmap map = {0};
  stufflib_hashmap_init(&map, 1);
  assert(stufflib_hashmap_insert(&map, "hello", 1));

  stufflib_hashmap_destroy(&map);
  return 1;
}

STUFFLIB_TEST_MAIN(test_empty,
                   test_insert_single_element,
                   test_update_single_element);
