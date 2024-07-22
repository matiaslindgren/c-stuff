#ifndef SL_HASHMAP_H_INCLUDED
#define SL_HASHMAP_H_INCLUDED

#include "../iterator/iterator.h"
#include "../span/span.h"

enum sl_hashmap_type {
  sl_hashmap_type_empty = 0,
  sl_hashmap_type_any,
  sl_hashmap_type_int64,
  sl_hashmap_type_uint64,
};

struct sl_hashmap_slot {
  struct sl_span key;
  size_t hash;
  enum sl_hashmap_type type;
  union {
    void* any;
    int64_t int64;
    uint64_t uint64;
  } value;
};

struct sl_hashmap {
  size_t size;
  size_t capacity;
  struct sl_hashmap_slot* slots;
};

struct sl_hashmap sl_hashmap_create(size_t init_capacity);

void sl_hashmap_destroy_slots(const size_t capacity,
                              struct sl_hashmap_slot slots[capacity]);

void sl_hashmap_destroy(struct sl_hashmap map[const static 1]);

struct sl_hashmap_slot* sl_hashmap_find_slot(
    struct sl_hashmap map[const static 1],
    struct sl_span key[const static 1],
    const size_t hash);

struct sl_hashmap_slot* sl_hashmap_get(struct sl_hashmap map[const static 1],
                                       struct sl_span key[const static 1]);

void sl_hashmap_write(struct sl_hashmap map[const static 1],
                      struct sl_span key[const static 1],
                      const size_t hash,
                      enum sl_hashmap_type type,
                      void* value);

void sl_hashmap_set(struct sl_hashmap map[const static 1],
                    struct sl_span key[const static 1],
                    enum sl_hashmap_type type,
                    void* value);

void sl_hashmap_resize(struct sl_hashmap map[const static 1],
                       const size_t new_capacity);

double sl_hashmap_load_factor(struct sl_hashmap map[const static 1]);

bool sl_hashmap_contains(struct sl_hashmap map[const static 1],
                         struct sl_span key[const static 1]);

void sl_hashmap_insert(struct sl_hashmap map[const static 1],
                       struct sl_span key[const static 1],
                       enum sl_hashmap_type type,
                       void* value);

size_t sl_hashmap_iter_find_next(struct sl_iterator iter[const static 1],
                                 const size_t begin);

void* sl_hashmap_iter_get(struct sl_iterator iter[const static 1]);

void sl_hashmap_iter_advance(struct sl_iterator iter[const static 1]);

bool sl_hashmap_iter_is_done(struct sl_iterator iter[const static 1]);

struct sl_iterator sl_hashmap_iter(struct sl_hashmap map[const static 1]);

#endif  // SL_HASHMAP_H_INCLUDED
