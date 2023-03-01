#ifndef _SL_HASHMAP_H_INCLUDED
#define _SL_HASHMAP_H_INCLUDED
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "stufflib_data.h"
#include "stufflib_hash.h"
#include "stufflib_macros.h"
#include "stufflib_math.h"
#include "stufflib_memory.h"
#include "stufflib_misc.h"

#ifndef SL_HASHMAP_MAX_LOAD_FACTOR
#define SL_HASHMAP_MAX_LOAD_FACTOR 0.5
#endif
#ifndef SL_HASHMAP_INIT_CAPACITY
#define SL_HASHMAP_INIT_CAPACITY 64
#endif

typedef struct sl_hashmap_slot sl_hashmap_slot;
struct sl_hashmap_slot {
  bool filled;
  sl_data key;
  size_t value;
};

typedef struct sl_hashmap sl_hashmap;
struct sl_hashmap {
  size_t size;
  size_t capacity;
  size_t num_collisions;
  sl_hashmap_slot* slots;
};

sl_hashmap sl_hashmap_create() {
  const size_t capacity = SL_HASHMAP_INIT_CAPACITY;
  assert(capacity);
  return (sl_hashmap){
      .capacity = capacity,
      .slots = sl_alloc(capacity, sizeof(sl_hashmap_slot)),
  };
}

void sl_hashmap_delete_slots(const size_t capacity,
                             sl_hashmap_slot slots[capacity]) {
  for (size_t i = 0; i < capacity; ++i) {
    sl_data_delete(&(slots[i].key));
  }
  sl_free(slots);
}

void sl_hashmap_delete(sl_hashmap map[const static 1]) {
  sl_hashmap_delete_slots(map->capacity, map->slots);
  *map = (sl_hashmap){0};
}

sl_hashmap_slot* sl_hashmap_get(sl_hashmap map[const static 1],
                                sl_data key[const static 1]) {
  size_t index = sl_hash_crc32_bytes(key->size, key->data);
  for (size_t probe = 0;; ++probe) {
    // https://en.wikipedia.org/wiki/Quadratic_probing#Quadratic_function
    // accessed 2023-02-06
    index += (probe + probe * probe) / 2;
    index %= map->capacity;
    sl_hashmap_slot* slot = map->slots + index;
    assert(slot);
    if (!slot->filled || sl_data_compare(&(slot->key), key) == 0) {
      return slot;
    }
    ++(map->num_collisions);
  }
}

void sl_hashmap_set(sl_hashmap map[const static 1],
                    sl_data key[const static 1],
                    const size_t value) {
  *sl_hashmap_get(map, key) = (sl_hashmap_slot){
      .filled = true,
      .key = sl_data_copy(key),
      .value = value,
  };
}

void sl_hashmap_resize(sl_hashmap map[const static 1],
                       const size_t new_capacity) {
  assert(new_capacity);
  assert(new_capacity >= map->size);
  sl_hashmap_slot* const old_slots = map->slots;
  const size_t old_capacity = map->capacity;
  map->slots = sl_alloc(new_capacity, sizeof(sl_hashmap_slot));
  map->capacity = new_capacity;
  for (size_t i = 0; i < old_capacity; ++i) {
    sl_hashmap_slot slot = old_slots[i];
    if (slot.filled) {
      sl_hashmap_set(map, &slot.key, slot.value);
    }
  }
  sl_hashmap_delete_slots(old_capacity, old_slots);
}

double sl_hashmap_load_factor(sl_hashmap map[const static 1]) {
  assert(map->capacity);
  return (double)map->size / (double)map->capacity;
}

bool sl_hashmap_contains(sl_hashmap map[const static 1],
                         sl_data key[const static 1]) {
  sl_hashmap_slot* slot = sl_hashmap_get(map, key);
  return slot->filled && sl_data_compare(&(slot->key), key) == 0;
}

void sl_hashmap_insert(sl_hashmap map[const static 1],
                       sl_data key[const static 1],
                       const size_t value) {
  assert(map->size < map->capacity);
  sl_hashmap_set(map, key, value);
  ++(map->size);
  if (sl_hashmap_load_factor(map) > SL_HASHMAP_MAX_LOAD_FACTOR) {
    const size_t new_capacity = sl_math_next_power_of_two(map->capacity);
    sl_hashmap_resize(map, new_capacity);
  }
}

size_t sl_hashmap_iter_find_next(sl_iterator iter[const static 1],
                                 const size_t begin) {
  const sl_hashmap* map = iter->data;
  size_t i = begin;
  while (i < map->capacity && !(map->slots[i].filled)) {
    ++i;
  }
  return i;
}

void* sl_hashmap_iter_get_item(sl_iterator iter[const static 1]) {
  sl_hashmap* map = iter->data;
  return map->slots + iter->index;
}

void sl_hashmap_iter_advance(sl_iterator iter[const static 1]) {
  iter->index = sl_hashmap_iter_find_next(iter, iter->index + 1);
  iter->pos += 1;
}

bool sl_hashmap_iter_is_done(sl_iterator iter[const static 1]) {
  const sl_hashmap* map = iter->data;
  return iter->index == map->capacity;
}

sl_iterator sl_hashmap_iter(const sl_hashmap map[const static 1]) {
  sl_iterator iter = (sl_iterator){
      .data = (void*)map,
      .get_item = sl_hashmap_iter_get_item,
      .advance = sl_hashmap_iter_advance,
      .is_done = sl_hashmap_iter_is_done,
  };
  iter.index = sl_hashmap_iter_find_next(&iter, 0);
  return iter;
}

#endif  // _SL_HASHMAP_H_INCLUDED
