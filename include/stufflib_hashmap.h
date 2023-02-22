#ifndef _STUFFLIB_HASHMAP_H_INCLUDED
#define _STUFFLIB_HASHMAP_H_INCLUDED
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "stufflib_hash.h"
#include "stufflib_macros.h"
#include "stufflib_math.h"
#include "stufflib_memory.h"
#include "stufflib_misc.h"

#define STUFFLIB_HASHMAP_MAX_LOAD_FACTOR 0.5

typedef struct stufflib_hashmap_node stufflib_hashmap_node;
struct stufflib_hashmap_node {
  const char* key;
  size_t value;
};

typedef struct stufflib_hashmap stufflib_hashmap;
struct stufflib_hashmap {
  size_t size;
  size_t capacity;
  stufflib_hashmap_node* nodes;
};

static size_t stufflib_hashmap_num_collisions = 0;

static inline void _realloc_nodes(stufflib_hashmap map[const static 1],
                                  const size_t new_capacity) {
  stufflib_hashmap_node* new_nodes =
      stufflib_alloc(new_capacity, sizeof(stufflib_hashmap_node));
  map->nodes = new_nodes;
  map->capacity = new_capacity;
}

stufflib_hashmap* stufflib_hashmap_init(stufflib_hashmap map[const static 1],
                                        const size_t capacity) {
  *map = (stufflib_hashmap){0};
  if (capacity) {
    _realloc_nodes(map, capacity);
  }
  return map;
}

void stufflib_hashmap_destroy(stufflib_hashmap map[const static 1]) {
  assert(map);
  free(map->nodes);
  *map = (stufflib_hashmap){0};
}

double stufflib_hashmap_load_factor(stufflib_hashmap map[const static 1]) {
  assert(map->capacity);
  return (double)map->size / (double)map->capacity;
}

stufflib_hashmap_node* stufflib_hashmap_get(
    stufflib_hashmap map[const static 1],
    const char key[const static 1]) {
  if (!map->capacity) {
    return nullptr;
  }
  size_t index = stufflib_hash_crc32_str(key);
  for (size_t probe = 0;; ++probe) {
    // https://en.wikipedia.org/wiki/Quadratic_probing#Quadratic_function
    // accessed 2023-02-06
    index += (probe + probe * probe) / 2;
    index %= map->capacity;
    stufflib_hashmap_node* node = map->nodes + index;
    assert(node);
    if (!node->key) {
      // empty slot
      return node;
    }
    if (strcmp(node->key, key) == 0) {
      // match
      return node;
    }
    ++stufflib_hashmap_num_collisions;
  }
}

bool stufflib_hashmap_resize(stufflib_hashmap map[const static 1],
                             const size_t new_capacity) {
  assert(new_capacity > 0);
  assert(new_capacity >= map->size);
  stufflib_hashmap_node* const old_nodes = map->nodes;
  const size_t old_capacity = map->capacity;
  _realloc_nodes(map, new_capacity);
  for (size_t i = 0; i < old_capacity; ++i) {
    const char* key = old_nodes[i].key;
    if (key) {
      const size_t value = old_nodes[i].value;
      *stufflib_hashmap_get(map, key) = (stufflib_hashmap_node){
          .key = key,
          .value = value,
      };
    }
  }
  free(old_nodes);
  return true;
}

bool stufflib_hashmap_contains(stufflib_hashmap map[const static 1],
                               const char key[const static 1]) {
  stufflib_hashmap_node* node = stufflib_hashmap_get(map, key);
  return node != nullptr && node->key != 0 && strcmp(node->key, key) == 0;
}

bool stufflib_hashmap_insert(stufflib_hashmap map[const static 1],
                             const char key[const static 1],
                             const size_t value) {
  assert(map->size < map->capacity);
  *stufflib_hashmap_get(map, key) = (stufflib_hashmap_node){
      .key = key,
      .value = value,
  };
  ++map->size;
  if (stufflib_hashmap_load_factor(map) > STUFFLIB_HASHMAP_MAX_LOAD_FACTOR) {
    const size_t new_capacity = stufflib_math_next_power_of_two(map->capacity);
    if (!stufflib_hashmap_resize(map, new_capacity)) {
      STUFFLIB_LOG_ERROR("failed resizing hashmap to capacity %zu",
                         new_capacity);
      return false;
    }
  }
  return true;
}

void stufflib_hashmap_update(stufflib_hashmap map[const static 1],
                             const char key[const static 1],
                             const size_t value) {
  assert(stufflib_hashmap_contains(map, key));
  stufflib_hashmap_get(map, key)->value = value;
}

#endif  // _STUFFLIB_HASHMAP_H_INCLUDED
