#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

size_t* UnionFind_init(size_t n) {
  size_t* parents = malloc(n * sizeof(size_t));
  if (!parents) {
    return NULL;
  }
  for (size_t i = 0; i < n; ++i) {
    parents[i] = SIZE_MAX;
  }
  return parents;
}

size_t UnionFind_find_root(size_t n, size_t parents[static n], size_t index) {
  while (parents[index] != SIZE_MAX) {
    index = parents[index];
  }
  return index;
}

size_t _UnionFind_exchange_root(size_t* parent, size_t new_root) {
  size_t old_root = *parent;
  *parent = new_root;
  return old_root;
}

void _UnionFind_find_replace(size_t n, size_t parents[static n], size_t index, size_t new_root) {
  while (parents[index] != SIZE_MAX) {
    index = _UnionFind_exchange_root(&parents[index], new_root);
  }
  parents[index] = new_root;
}

size_t _UnionFind_find_compress(size_t n, size_t parents[static n], size_t index) {
  size_t root = UnionFind_find_root(n, parents, index);
  while (index != root) {
    index = _UnionFind_exchange_root(&parents[index], root);
  }
  return root;
}

size_t UnionFind_union(size_t n, size_t parents[static n], size_t lhs, size_t rhs) {
  size_t root = _UnionFind_find_compress(n, parents, lhs);
  _UnionFind_find_replace(n, parents, rhs, root);
  return root;
}

int main(void) {
  const size_t num_sets = 10;
  size_t* parents = UnionFind_init(num_sets);
  if (!parents) {
    fprintf(stderr, "error: unable to allocate 'parents' array of size %zu\n", num_sets);
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < num_sets; ++i) {
    assert(parents[i] == SIZE_MAX);
  }

  UnionFind_union(num_sets, parents, 0, 1);
  assert(parents[0] == SIZE_MAX);
  assert(parents[1] == 0);
  assert(parents[2] == SIZE_MAX);
  assert(parents[3] == SIZE_MAX);
  assert(parents[4] == SIZE_MAX);
  assert(parents[5] == SIZE_MAX);
  assert(parents[6] == SIZE_MAX);
  assert(parents[7] == SIZE_MAX);
  assert(parents[8] == SIZE_MAX);
  assert(parents[9] == SIZE_MAX);

  UnionFind_union(num_sets, parents, 4, 5);
  assert(parents[0] == SIZE_MAX);
  assert(parents[1] == 0);
  assert(parents[2] == SIZE_MAX);
  assert(parents[3] == SIZE_MAX);
  assert(parents[4] == SIZE_MAX);
  assert(parents[5] == 4);
  assert(parents[6] == SIZE_MAX);
  assert(parents[7] == SIZE_MAX);
  assert(parents[8] == SIZE_MAX);
  assert(parents[9] == SIZE_MAX);

  UnionFind_union(num_sets, parents, 9, 1);
  assert(parents[0] == 9);
  assert(parents[1] == 9);
  assert(parents[2] == SIZE_MAX);
  assert(parents[3] == SIZE_MAX);
  assert(parents[4] == SIZE_MAX);
  assert(parents[5] == 4);
  assert(parents[6] == SIZE_MAX);
  assert(parents[7] == SIZE_MAX);
  assert(parents[8] == SIZE_MAX);
  assert(parents[9] == SIZE_MAX);

  UnionFind_union(num_sets, parents, 0, 5);
  assert(parents[0] == 9);
  assert(parents[1] == 9);
  assert(parents[2] == SIZE_MAX);
  assert(parents[3] == SIZE_MAX);
  assert(parents[4] == 9);
  assert(parents[5] == 9);
  assert(parents[6] == SIZE_MAX);
  assert(parents[7] == SIZE_MAX);
  assert(parents[8] == SIZE_MAX);
  assert(parents[9] == SIZE_MAX);

  UnionFind_union(num_sets, parents, 8, 9);
  assert(parents[0] == 9);
  assert(parents[1] == 9);
  assert(parents[2] == SIZE_MAX);
  assert(parents[3] == SIZE_MAX);
  assert(parents[4] == 9);
  assert(parents[5] == 9);
  assert(parents[6] == SIZE_MAX);
  assert(parents[7] == SIZE_MAX);
  assert(parents[8] == SIZE_MAX);
  assert(parents[9] == 8);

  UnionFind_union(num_sets, parents, 2, 3);
  assert(parents[0] == 9);
  assert(parents[1] == 9);
  assert(parents[2] == SIZE_MAX);
  assert(parents[3] == 2);
  assert(parents[4] == 9);
  assert(parents[5] == 9);
  assert(parents[6] == SIZE_MAX);
  assert(parents[7] == SIZE_MAX);
  assert(parents[8] == SIZE_MAX);
  assert(parents[9] == 8);

  UnionFind_union(num_sets, parents, 6, 2);
  assert(parents[0] == 9);
  assert(parents[1] == 9);
  assert(parents[2] == 6);
  assert(parents[3] == 2);
  assert(parents[4] == 9);
  assert(parents[5] == 9);
  assert(parents[6] == SIZE_MAX);
  assert(parents[7] == SIZE_MAX);
  assert(parents[8] == SIZE_MAX);
  assert(parents[9] == 8);

  UnionFind_union(num_sets, parents, 7, 6);
  UnionFind_union(num_sets, parents, 8, 7);
  assert(parents[0] == 9);
  assert(parents[1] == 9);
  assert(parents[2] == 6);
  assert(parents[3] == 2);
  assert(parents[4] == 9);
  assert(parents[5] == 9);
  assert(parents[6] == 7);
  assert(parents[7] == 8);
  assert(parents[8] == SIZE_MAX);
  assert(parents[9] == 8);

  for (size_t i = 0; i < num_sets; ++i) {
    assert(UnionFind_find_root(num_sets, parents, i) == 8);
  }

  free(parents);
  return EXIT_SUCCESS;
}
