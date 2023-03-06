#ifndef SL_UNIONFIND_H_INCLUDED
#define SL_UNIONFIND_H_INCLUDED
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_memory.h"

struct sl_unionfind {
  size_t count;
  size_t* parents;
};

#define SL_UNIONFIND_ROOT SIZE_MAX

struct sl_unionfind* sl_unionfind_init(struct sl_unionfind uf[static 1],
                                       const size_t count) {
  size_t* parents = sl_alloc(count, sizeof(size_t));
  for (size_t i = 0; i < count; ++i) {
    parents[i] = SL_UNIONFIND_ROOT;
  }
  uf->count = count;
  uf->parents = parents;
  return uf;
}

void sl_unionfind_destroy(struct sl_unionfind uf[static 1]) {
  sl_free(uf->parents);
  *uf = (struct sl_unionfind){0};
}

size_t sl_unionfind_find_root(const struct sl_unionfind uf[const static 1],
                              size_t index) {
  while (uf->parents[index] != SL_UNIONFIND_ROOT) {
    index = uf->parents[index];
  }
  return index;
}

void _sl_unionfind_set_root(const struct sl_unionfind uf[const static 1],
                            const size_t begin,
                            const size_t root) {
  for (size_t parent = begin; parent != SL_UNIONFIND_ROOT;) {
    const size_t next = uf->parents[parent];
    if (parent != root) {
      uf->parents[parent] = root;
    }
    parent = next;
  }
}

void sl_unionfind_union(const struct sl_unionfind uf[const static 1],
                        const size_t lhs,
                        const size_t rhs) {
  _sl_unionfind_set_root(uf, rhs, sl_unionfind_find_root(uf, lhs));
}

#undef SL_UNIONFIND_ROOT

#endif  // SL_UNIONFIND_H_INCLUDED
