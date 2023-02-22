#ifndef _STUFFLIB_UNIONFIND_H_INCLUDED
#define _STUFFLIB_UNIONFIND_H_INCLUDED
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_memory.h"

typedef struct stufflib_unionfind stufflib_unionfind;
struct stufflib_unionfind {
  size_t count;
  size_t* parents;
};

#define _STUFFLIB_UNIONFIND_ROOT SIZE_MAX

stufflib_unionfind* stufflib_unionfind_init(stufflib_unionfind uf[static 1],
                                            const size_t count) {
  size_t* parents = stufflib_alloc(count, sizeof(size_t));
  for (size_t i = 0; i < count; ++i) {
    parents[i] = _STUFFLIB_UNIONFIND_ROOT;
  }
  uf->count = count;
  uf->parents = parents;
  return uf;
}

void stufflib_unionfind_destroy(stufflib_unionfind uf[static 1]) {
  free(uf->parents);
  *uf = (stufflib_unionfind){0};
}

size_t stufflib_unionfind_find_root(const stufflib_unionfind uf[const static 1],
                                    size_t index) {
  while (uf->parents[index] != _STUFFLIB_UNIONFIND_ROOT) {
    index = uf->parents[index];
  }
  return index;
}

void _stufflib_unionfind_set_root(const stufflib_unionfind uf[const static 1],
                                  const size_t begin,
                                  const size_t root) {
  for (size_t parent = begin; parent != _STUFFLIB_UNIONFIND_ROOT;) {
    const size_t next = uf->parents[parent];
    if (parent != root) {
      uf->parents[parent] = root;
    }
    parent = next;
  }
}

void stufflib_unionfind_union(const stufflib_unionfind uf[const static 1],
                              const size_t lhs,
                              const size_t rhs) {
  _stufflib_unionfind_set_root(uf, rhs, stufflib_unionfind_find_root(uf, lhs));
}

#undef _STUFFLIB_UNIONFIND_ROOT

#endif  // _STUFFLIB_UNIONFIND_H_INCLUDED
