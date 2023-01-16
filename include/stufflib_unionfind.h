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

int stufflib_unionfind_init(stufflib_unionfind uf[static 1],
                            const size_t count) {
  size_t* parents = calloc(count, sizeof(size_t));
  if (!parents) {
    return 0;
  }
  for (size_t i = 0; i < count; ++i) {
    // TODO use null pointers instead of sentinels?
    parents[i] = SIZE_MAX;
  }
  *uf = (stufflib_unionfind){
      .count = count,
      .parents = parents,
  };
  return 1;
}

void stufflib_unionfind_destroy(stufflib_unionfind uf[static 1]) {
  free(uf->parents);
  *uf = (stufflib_unionfind){0};
}

size_t stufflib_unionfind_find_root(const stufflib_unionfind uf[const static 1],
                                    size_t index) {
  while (uf->parents[index] != SIZE_MAX) {
    index = uf->parents[index];
  }
  return index;
}

size_t _stufflib_unionfind_exchange_root(size_t parent[static 1],
                                         const size_t new_root) {
  size_t old_root = parent[0];
  parent[0] = new_root;
  return old_root;
}

void _stufflib_unionfind_find_replace(
    const stufflib_unionfind uf[const static 1],
    size_t index,
    const size_t new_root) {
  while (uf->parents[index] != SIZE_MAX) {
    index = _stufflib_unionfind_exchange_root(uf->parents + index, new_root);
  }
  uf->parents[index] = new_root;
}

size_t _stufflib_unionfind_find_compress(
    const stufflib_unionfind uf[const static 1],
    size_t index) {
  const size_t root = stufflib_unionfind_find_root(uf, index);
  while (index != root) {
    index = _stufflib_unionfind_exchange_root(uf->parents + index, root);
  }
  return root;
}

size_t stufflib_unionfind_union(const stufflib_unionfind uf[const static 1],
                                const size_t lhs,
                                const size_t rhs) {
  size_t root = _stufflib_unionfind_find_compress(uf, lhs);
  _stufflib_unionfind_find_replace(uf, rhs, root);
  return root;
}
#endif  // _STUFFLIB_UNIONFIND_H_INCLUDED