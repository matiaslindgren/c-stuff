#ifndef _STUFFLIB_HASHMAP_H_INCLUDED
#define _STUFFLIB_HASHMAP_H_INCLUDED
#include <assert.h>
#include <stdint.h>

#include "stufflib_hash.h"
#include "stufflib_macros.h"
#include "stufflib_misc.h"

typedef struct stufflib_hashmap_node stufflib_hashmap_node;
struct stufflib_hashmap_node {
  const char* key;
  size_t value;
};

typedef struct stufflib_hashmap stufflib_hashmap;
struct stufflib_hashmap {
  double load_factor;
  size_t size;
  size_t capacity;
  stufflib_hashmap_node* nodes;
};

void stufflib_hashmap_destroy(stufflib_hashmap map[static 1]) {
  assert(map);
  free(map->nodes);
  map->size = 0;
  map->capacity = 0;
}

int stufflib_hashmap_resize(stufflib_hashmap map[static 1],
                            const size_t capacity) {
  if (capacity) {
    stufflib_hashmap_node* new_nodes =
        calloc(capacity, sizeof(stufflib_hashmap_node));
    if (!new_nodes) {
      STUFFLIB_PRINT_ERROR("failed allocating %zu hashmap nodes", capacity);
      return 0;
    }
    if (map->size) {
      assert(map->nodes);
      memcpy(new_nodes, map->nodes, map->size * sizeof(stufflib_hashmap_node));
      free(map->nodes);
    }
    map->nodes = new_nodes;
    map->capacity = capacity;
  }
  return 1;
}

stufflib_hashmap* stufflib_hashmap_init(stufflib_hashmap map[const static 1],
                                        const size_t capacity) {
  *map = (stufflib_hashmap){.load_factor = 0.5};
  if (!stufflib_hashmap_resize(map, capacity)) {
    STUFFLIB_PRINT_ERROR("failed allocating hashmap with capacity %zu",
                         capacity);
    return 0;
  }
  return map;
}

stufflib_hashmap_node* stufflib_hashmap_get(stufflib_hashmap map[static 1],
                                            const char key[const static 1]) {
  if (!map->capacity) {
    return 0;
  }
  size_t index = stufflib_hash_crc32_str(key);
  for (size_t probe = 0;; ++probe) {
    index += probe + probe * probe;
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
    // collision, continue
  }
}

int stufflib_hashmap_contains(stufflib_hashmap map[const static 1],
                              const char key[const static 1]) {
  stufflib_hashmap_node* node = stufflib_hashmap_get(map, key);
  return node && node->key != 0 && strcmp(node->key, key) == 0;
}

int stufflib_hashmap_insert(stufflib_hashmap map[static 1],
                            const char key[const static 1],
                            const size_t value) {
  if ((double)map->size / (double)map->capacity > map->load_factor) {
    const size_t new_capacity = (double)(map->capacity + 1) / map->load_factor;
    if (!stufflib_hashmap_resize(map, new_capacity)) {
      STUFFLIB_PRINT_ERROR("failed resizing hashmap to capacity %zu",
                           new_capacity);
      return 0;
    }
    printf("resize %zu/%zu\n", map->size, map->capacity);
  }
  stufflib_hashmap_node* node = stufflib_hashmap_get(map, key);
  assert(node);
  *node = (stufflib_hashmap_node){
      .key = key,
      .value = value,
  };
  ++map->size;
  return 1;
}

void stufflib_hashmap_update(stufflib_hashmap map[static 1],
                             const char key[const static 1],
                             const size_t value) {
  assert(stufflib_hashmap_contains(map, key));
  stufflib_hashmap_get(map, key)->value = value;
}

#endif  // _STUFFLIB_HASHMAP_H_INCLUDED
