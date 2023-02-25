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

#ifndef STUFFLIB_HASHMAP_MAX_LOAD_FACTOR
#define STUFFLIB_HASHMAP_MAX_LOAD_FACTOR 0.5
#endif
#ifndef STUFFLIB_HASHMAP_INIT_CAPACITY
#define STUFFLIB_HASHMAP_INIT_CAPACITY 64
#endif

typedef struct stufflib_hashmap_slot stufflib_hashmap_slot;
struct stufflib_hashmap_slot {
  bool filled;
  stufflib_data key;
  size_t value;
};

typedef struct stufflib_hashmap stufflib_hashmap;
struct stufflib_hashmap {
  size_t size;
  size_t capacity;
  size_t num_collisions;
  stufflib_hashmap_slot* slots;
};

stufflib_hashmap stufflib_hashmap_create() {
  const size_t capacity = STUFFLIB_HASHMAP_INIT_CAPACITY;
  assert(capacity);
  return (stufflib_hashmap){
      .capacity = capacity,
      .slots = stufflib_alloc(capacity, sizeof(stufflib_hashmap_slot)),
  };
}

void stufflib_hashmap_delete_slots(const size_t capacity,
                                   stufflib_hashmap_slot slots[capacity]) {
  for (size_t i = 0; i < capacity; ++i) {
    stufflib_data_delete(&(slots[i].key));
  }
  stufflib_free(slots);
}

void stufflib_hashmap_delete(stufflib_hashmap map[const static 1]) {
  stufflib_hashmap_delete_slots(map->capacity, map->slots);
  *map = (stufflib_hashmap){0};
}

stufflib_hashmap_slot* stufflib_hashmap_get(
    stufflib_hashmap map[const static 1],
    stufflib_data key[const static 1]) {
  size_t index = stufflib_hash_crc32_bytes(key->size, key->data);
  for (size_t probe = 0;; ++probe) {
    // https://en.wikipedia.org/wiki/Quadratic_probing#Quadratic_function
    // accessed 2023-02-06
    index += (probe + probe * probe) / 2;
    index %= map->capacity;
    stufflib_hashmap_slot* slot = map->slots + index;
    assert(slot);
    if (!slot->filled || stufflib_data_compare(&(slot->key), key) == 0) {
      return slot;
    }
    ++(map->num_collisions);
  }
}

void stufflib_hashmap_set(stufflib_hashmap map[const static 1],
                          stufflib_data key[const static 1],
                          const size_t value) {
  *stufflib_hashmap_get(map, key) = (stufflib_hashmap_slot){
      .filled = true,
      .key = stufflib_data_copy(key),
      .value = value,
  };
}

void stufflib_hashmap_resize(stufflib_hashmap map[const static 1],
                             const size_t new_capacity) {
  assert(new_capacity);
  assert(new_capacity >= map->size);
  stufflib_hashmap_slot* const old_slots = map->slots;
  const size_t old_capacity = map->capacity;
  map->slots = stufflib_alloc(new_capacity, sizeof(stufflib_hashmap_slot));
  map->capacity = new_capacity;
  for (size_t i = 0; i < old_capacity; ++i) {
    stufflib_hashmap_slot slot = old_slots[i];
    if (slot.filled) {
      stufflib_hashmap_set(map, &slot.key, slot.value);
    }
  }
  stufflib_hashmap_delete_slots(old_capacity, old_slots);
}

double stufflib_hashmap_load_factor(stufflib_hashmap map[const static 1]) {
  assert(map->capacity);
  return (double)map->size / (double)map->capacity;
}

bool stufflib_hashmap_contains(stufflib_hashmap map[const static 1],
                               stufflib_data key[const static 1]) {
  stufflib_hashmap_slot* slot = stufflib_hashmap_get(map, key);
  return slot->filled && stufflib_data_compare(&(slot->key), key) == 0;
}

void stufflib_hashmap_insert(stufflib_hashmap map[const static 1],
                             stufflib_data key[const static 1],
                             const size_t value) {
  assert(map->size < map->capacity);
  stufflib_hashmap_set(map, key, value);
  ++(map->size);
  if (stufflib_hashmap_load_factor(map) > STUFFLIB_HASHMAP_MAX_LOAD_FACTOR) {
    const size_t new_capacity = stufflib_math_next_power_of_two(map->capacity);
    stufflib_hashmap_resize(map, new_capacity);
  }
}

size_t stufflib_hashmap_iter_find_next(stufflib_iterator iter[const static 1],
                                       const size_t begin) {
  const stufflib_hashmap* map = iter->data;
  size_t i = begin;
  while (i < map->capacity && !(map->slots[i].filled)) {
    ++i;
  }
  return i;
}

void* stufflib_hashmap_iter_get_item(stufflib_iterator iter[const static 1]) {
  stufflib_hashmap* map = iter->data;
  return map->slots + iter->index;
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
