#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stufflib/context/context.h>
#include <stufflib/hash/hash.h>
#include <stufflib/hashmap/hashmap.h>
#include <stufflib/iterator/iterator.h>
#include <stufflib/math/math.h>
#include <stufflib/memory/memory.h>
#include <stufflib/span/span.h>

struct sl_hashmap sl_hashmap_create(struct sl_context ctx[static 1], size_t init_capacity) {
  return (struct sl_hashmap){
      .capacity = init_capacity,
      .slots    = sl_alloc(ctx, init_capacity, sizeof(struct sl_hashmap_slot)),
  };
}

void sl_hashmap_destroy_slots(const size_t capacity, struct sl_hashmap_slot slots[capacity]) {
  for (size_t i = 0; i < capacity; ++i) {
    sl_span_destroy(&(slots[i].key));
  }
  sl_free(slots);
}

void sl_hashmap_destroy(struct sl_hashmap map[const static 1]) {
  sl_hashmap_destroy_slots(map->capacity, map->slots);
  *map = (struct sl_hashmap){0};
}

struct sl_hashmap_slot* sl_hashmap_find_slot(
    struct sl_context ctx[static 1],
    struct sl_hashmap map[const static 1],
    struct sl_span key[const static 1],
    const size_t hash
) {
  if (!map->capacity) {
    SL_ERROR(ctx, "hashmap has zero capacity");
    return nullptr;
  }
  // https://en.wikipedia.org/wiki/Quadratic_probing#Quadratic_function
  // accessed 2023-02-06
  for (size_t index = hash, probe = 0; probe < map->capacity; ++probe) {
    index                        = (index + (probe + probe * probe) / 2) % map->capacity;
    struct sl_hashmap_slot* slot = map->slots + index;
    if (slot->type == sl_hashmap_type_empty || sl_span_compare(&(slot->key), key) == 0) {
      return slot;
    }
  }
  SL_ERROR(ctx, "hashmap is full");
  return nullptr;
}

struct sl_hashmap_slot* sl_hashmap_get(
    struct sl_context ctx[static 1],
    struct sl_hashmap map[const static 1],
    struct sl_span key[const static 1]
) {
  return sl_hashmap_find_slot(ctx, map, key, sl_hash_crc32_bytes(key->size, key->data));
}

bool sl_hashmap_write(
    struct sl_context ctx[static 1],
    struct sl_hashmap map[const static 1],
    struct sl_span key[const static 1],
    const size_t hash,
    enum sl_hashmap_type type,
    void* value
) {
  struct sl_hashmap_slot* slot = sl_hashmap_find_slot(ctx, map, key, hash);
  if (!slot) {
    return false;
  }
  slot->key  = sl_span_copy(ctx, key);
  slot->hash = hash;
  slot->type = type;
  switch (type) {
    case sl_hashmap_type_empty: {
    } break;
    case sl_hashmap_type_any: {
      slot->value.any = value;
    } break;
    case sl_hashmap_type_int64: {
      slot->value.int64 = ((int64_t*)value)[0];
    } break;
    case sl_hashmap_type_uint64: {
      slot->value.uint64 = ((uint64_t*)value)[0];
    } break;
  }
  return true;
}

bool sl_hashmap_set(
    struct sl_context ctx[static 1],
    struct sl_hashmap map[const static 1],
    struct sl_span key[const static 1],
    enum sl_hashmap_type type,
    void* value
) {
  return sl_hashmap_write(ctx, map, key, sl_hash_crc32_bytes(key->size, key->data), type, value);
}

bool sl_hashmap_resize(
    struct sl_context ctx[static 1],
    struct sl_hashmap map[const static 1],
    const size_t new_capacity
) {
  assert(new_capacity);
  assert(new_capacity >= map->size);
  struct sl_hashmap_slot* const old_slots = map->slots;
  const size_t old_capacity               = map->capacity;
  map->slots    = sl_alloc(ctx, new_capacity, sizeof(struct sl_hashmap_slot));
  map->capacity = new_capacity;
  for (size_t i = 0; i < old_capacity; ++i) {
    struct sl_hashmap_slot slot = old_slots[i];
    if (!sl_hashmap_write(ctx, map, &(slot.key), slot.hash, slot.type, &(slot.value))) {
      sl_hashmap_destroy_slots(old_capacity, old_slots);
      return false;
    }
  }
  sl_hashmap_destroy_slots(old_capacity, old_slots);
  return true;
}

double sl_hashmap_load_factor(struct sl_hashmap map[const static 1]) {
  return map->capacity ? (double)map->size / (double)map->capacity : 1;
}

bool sl_hashmap_contains(
    struct sl_context ctx[static 1],
    struct sl_hashmap map[const static 1],
    struct sl_span key[const static 1]
) {
  struct sl_hashmap_slot* const slot = sl_hashmap_get(ctx, map, key);
  return slot && slot->type != sl_hashmap_type_empty && sl_span_compare(&(slot->key), key) == 0;
}

bool sl_hashmap_insert(
    struct sl_context ctx[static 1],
    struct sl_hashmap map[const static 1],
    struct sl_span key[const static 1],
    enum sl_hashmap_type type,
    void* value
) {
  if (map->size >= map->capacity) {
    SL_ERROR(ctx, "hashmap is full, cannot insert");
    return false;
  }
  if (!sl_hashmap_set(ctx, map, key, type, value)) {
    return false;
  }
  ++(map->size);
  if (sl_hashmap_load_factor(map) > SL_HASHMAP_MAX_LOAD_FACTOR) {
    const size_t new_capacity = sl_math_next_power_of_two(map->capacity);
    return sl_hashmap_resize(ctx, map, new_capacity);
  }
  return true;
}

size_t sl_hashmap_iter_find_next(struct sl_iterator iter[const static 1], const size_t begin) {
  struct sl_hashmap* map = iter->data;
  size_t i               = begin;
  while (i < map->capacity && map->slots[i].type == sl_hashmap_type_empty) {
    ++i;
  }
  return i;
}

void* sl_hashmap_iter_get(struct sl_iterator iter[const static 1]) {
  struct sl_hashmap* map = iter->data;
  return map->slots + iter->index;
}

void sl_hashmap_iter_advance(struct sl_iterator iter[const static 1]) {
  iter->index = sl_hashmap_iter_find_next(iter, iter->index + 1);
  iter->pos += 1;
}

bool sl_hashmap_iter_is_done(struct sl_iterator iter[const static 1]) {
  struct sl_hashmap* map = iter->data;
  return iter->index == map->capacity;
}

struct sl_iterator sl_hashmap_iter(struct sl_hashmap map[const static 1]) {
  struct sl_iterator iter = {.data = (void*)map};
  iter.index              = sl_hashmap_iter_find_next(&iter, 0);
  return iter;
}
