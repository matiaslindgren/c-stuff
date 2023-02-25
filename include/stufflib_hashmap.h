#ifndef _STUFFLIB_HASHMAP_H_INCLUDED
#define _STUFFLIB_HASHMAP_H_INCLUDED
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "stufflib_data.h"
#include "stufflib_hash.h"
#include "stufflib_macros.h"
#include "stufflib_math.h"
#include "stufflib_memory.h"
#include "stufflib_misc.h"

#define STUFFLIB_HASHMAP_MAX_LOAD_FACTOR 0.5

typedef struct stufflib_hashmap_node stufflib_hashmap_node;
struct stufflib_hashmap_node {
  bool has_value;
  stufflib_data key;
  size_t value;
};

typedef struct stufflib_hashmap stufflib_hashmap;
struct stufflib_hashmap {
  size_t size;
  size_t capacity;
  size_t num_collisions;
  stufflib_hashmap_node* nodes;
};

// TODO less awful
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
  stufflib_free(map->nodes);
  *map = (stufflib_hashmap){0};
}

double stufflib_hashmap_load_factor(stufflib_hashmap map[const static 1]) {
  assert(map->capacity);
  return (double)map->size / (double)map->capacity;
}

stufflib_hashmap_node* stufflib_hashmap_get(
    stufflib_hashmap map[const static 1],
    stufflib_data key[const static 1]) {
  if (!map->capacity) {
    return nullptr;
  }
  size_t index = stufflib_hash_crc32_bytes(key->size, key->data);
  for (size_t probe = 0;; ++probe) {
    // https://en.wikipedia.org/wiki/Quadratic_probing#Quadratic_function
    // accessed 2023-02-06
    index += (probe + probe * probe) / 2;
    index %= map->capacity;
    stufflib_hashmap_node* node = map->nodes + index;
    assert(node);
    if (!node->has_value || stufflib_data_compare(&(node->key), key) == 0) {
      return node;
    }
    ++(map->num_collisions);
  }
}

void stufflib_hashmap_set(stufflib_hashmap map[const static 1],
                          stufflib_data key[const static 1],
                          const size_t value) {
  *stufflib_hashmap_get(map, key) = (stufflib_hashmap_node){
      .has_value = true,
      .key = *key,
      .value = value,
  };
}

bool stufflib_hashmap_resize(stufflib_hashmap map[const static 1],
                             const size_t new_capacity) {
  assert(new_capacity > 0);
  assert(new_capacity >= map->size);
  stufflib_hashmap_node* const old_nodes = map->nodes;
  const size_t old_capacity = map->capacity;
  _realloc_nodes(map, new_capacity);
  for (size_t i = 0; i < old_capacity; ++i) {
    if (old_nodes[i].has_value) {
      stufflib_data key = old_nodes[i].key;
      const size_t value = old_nodes[i].value;
      stufflib_hashmap_set(map, &key, value);
    }
  }
  stufflib_free(old_nodes);
  return true;
}

bool stufflib_hashmap_contains(stufflib_hashmap map[const static 1],
                               stufflib_data key[const static 1]) {
  stufflib_hashmap_node* node = stufflib_hashmap_get(map, key);
  return node != nullptr && node->has_value &&
         stufflib_data_compare(&(node->key), key) == 0;
}

bool stufflib_hashmap_insert(stufflib_hashmap map[const static 1],
                             stufflib_data key[const static 1],
                             const size_t value) {
  assert(map->size < map->capacity);
  stufflib_hashmap_set(map, key, value);
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

size_t stufflib_hashmap_iter_find_next(stufflib_iterator iter[const static 1],
                                       const size_t begin) {
  const stufflib_hashmap* map = iter->data;
  size_t i = begin;
  while (i < map->capacity && !(map->nodes[i].has_value)) {
    ++i;
  }
  return i;
}

void* stufflib_hashmap_iter_get_item(stufflib_iterator iter[const static 1]) {
  stufflib_hashmap* map = iter->data;
  return map->nodes + iter->index;
}

void stufflib_hashmap_iter_advance(stufflib_iterator iter[const static 1]) {
  iter->index = stufflib_hashmap_iter_find_next(iter, iter->index + 1);
  iter->pos += 1;
}

bool stufflib_hashmap_iter_is_done(stufflib_iterator iter[const static 1]) {
  const stufflib_hashmap* map = iter->data;
  return iter->index == map->capacity;
}

stufflib_iterator stufflib_hashmap_iter(
    const stufflib_hashmap map[const static 1]) {
  stufflib_iterator iter = (stufflib_iterator){
      .data = (void*)map,
      .get_item = stufflib_hashmap_iter_get_item,
      .advance = stufflib_hashmap_iter_advance,
      .is_done = stufflib_hashmap_iter_is_done,
  };
  iter.index = stufflib_hashmap_iter_find_next(&iter, 0);
  return iter;
}

#endif  // _STUFFLIB_HASHMAP_H_INCLUDED
