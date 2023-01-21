#ifndef _STUFFLIB_UNIONFIND_H_INCLUDED
#define _STUFFLIB_UNIONFIND_H_INCLUDED
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct stufflib_unionfind stufflib_unionfind;
struct stufflib_unionfind {
  size_t count;
  size_t* parents;
};

#define _STUFFLIB_UNIONFIND_ROOT SIZE_MAX

stufflib_unionfind* stufflib_unionfind_init(stufflib_unionfind uf[static 1],
                                            const size_t count) {
  size_t* parents = calloc(count, sizeof(size_t));
  if (!parents) {
    *uf = (stufflib_unionfind){0};
    return uf;
  }
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
  for (size_t parent = begin;
       parent != root && parent != _STUFFLIB_UNIONFIND_ROOT;) {
    const size_t next = uf->parents[parent];
    uf->parents[parent] = root;
    parent = next;
  }
}

size_t stufflib_unionfind_union(const stufflib_unionfind uf[const static 1],
                                const size_t lhs,
                                const size_t rhs) {
  const size_t root = stufflib_unionfind_find_root(uf, lhs);
  _stufflib_unionfind_set_root(uf, rhs, root);
  return root;
}

#undef _STUFFLIB_UNIONFIND_ROOT

#endif  // _STUFFLIB_UNIONFIND_H_INCLUDED
