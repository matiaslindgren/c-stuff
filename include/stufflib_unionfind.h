#ifndef _STUFFLIB_UNIONFIND_H_INCLUDED
#define _STUFFLIB_UNIONFIND_H_INCLUDED
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

size_t* stufflib_unionfind_init(const size_t n) {
  size_t* parents = calloc(n, sizeof(size_t));
  if (!parents) {
    return 0;
  }
  for (size_t i = 0; i < n; ++i) {
    parents[i] = SIZE_MAX;
  }
  return parents;
}

size_t stufflib_unionfind_find_root(const size_t n,
                                    size_t parents[n],
                                    size_t index) {
  while (parents[index] != SIZE_MAX) {
    index = parents[index];
  }
  return index;
}

size_t _stufflib_unionfind_exchange_root(size_t parent[static 1],
                                         const size_t new_root) {
  size_t old_root = parent[0];
  parent[0] = new_root;
  return old_root;
}

void _stufflib_unionfind_find_replace(const size_t n,
                                      size_t parents[const n],
                                      size_t index,
                                      const size_t new_root) {
  while (parents[index] != SIZE_MAX) {
    index = _stufflib_unionfind_exchange_root(&parents[index], new_root);
  }
  parents[index] = new_root;
}

size_t _stufflib_unionfind_find_compress(const size_t n,
                                         size_t parents[const n],
                                         size_t index) {
  const size_t root = stufflib_unionfind_find_root(n, parents, index);
  while (index != root) {
    index = _stufflib_unionfind_exchange_root(&parents[index], root);
  }
  return root;
}

size_t stufflib_unionfind_union(const size_t n,
                                size_t parents[const n],
                                const size_t lhs,
                                const size_t rhs) {
  size_t root = _stufflib_unionfind_find_compress(n, parents, lhs);
  _stufflib_unionfind_find_replace(n, parents, rhs, root);
  return root;
}
#endif  // _STUFFLIB_UNIONFIND_H_INCLUDED
