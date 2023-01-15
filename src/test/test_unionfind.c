#include "stufflib_unionfind.h"

int main(int argc, char* const argv[argc + 1]) {
  stufflib_unionfind uf = stufflib_unionfind_new(10);
  if (!uf.size) {
    fprintf(stderr, "error: unable to allocate union find structure\n");
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < uf.size; ++i) {
    assert(uf.parents[i] == SIZE_MAX);
  }

  stufflib_unionfind_union(uf, 0, 1);
  assert(uf.parents[0] == SIZE_MAX);
  assert(uf.parents[1] == 0);
  assert(uf.parents[2] == SIZE_MAX);
  assert(uf.parents[3] == SIZE_MAX);
  assert(uf.parents[4] == SIZE_MAX);
  assert(uf.parents[5] == SIZE_MAX);
  assert(uf.parents[6] == SIZE_MAX);
  assert(uf.parents[7] == SIZE_MAX);
  assert(uf.parents[8] == SIZE_MAX);
  assert(uf.parents[9] == SIZE_MAX);

  stufflib_unionfind_union(uf, 4, 5);
  assert(uf.parents[0] == SIZE_MAX);
  assert(uf.parents[1] == 0);
  assert(uf.parents[2] == SIZE_MAX);
  assert(uf.parents[3] == SIZE_MAX);
  assert(uf.parents[4] == SIZE_MAX);
  assert(uf.parents[5] == 4);
  assert(uf.parents[6] == SIZE_MAX);
  assert(uf.parents[7] == SIZE_MAX);
  assert(uf.parents[8] == SIZE_MAX);
  assert(uf.parents[9] == SIZE_MAX);

  stufflib_unionfind_union(uf, 9, 1);
  assert(uf.parents[0] == 9);
  assert(uf.parents[1] == 9);
  assert(uf.parents[2] == SIZE_MAX);
  assert(uf.parents[3] == SIZE_MAX);
  assert(uf.parents[4] == SIZE_MAX);
  assert(uf.parents[5] == 4);
  assert(uf.parents[6] == SIZE_MAX);
  assert(uf.parents[7] == SIZE_MAX);
  assert(uf.parents[8] == SIZE_MAX);
  assert(uf.parents[9] == SIZE_MAX);

  stufflib_unionfind_union(uf, 0, 5);
  assert(uf.parents[0] == 9);
  assert(uf.parents[1] == 9);
  assert(uf.parents[2] == SIZE_MAX);
  assert(uf.parents[3] == SIZE_MAX);
  assert(uf.parents[4] == 9);
  assert(uf.parents[5] == 9);
  assert(uf.parents[6] == SIZE_MAX);
  assert(uf.parents[7] == SIZE_MAX);
  assert(uf.parents[8] == SIZE_MAX);
  assert(uf.parents[9] == SIZE_MAX);

  stufflib_unionfind_union(uf, 8, 9);
  assert(uf.parents[0] == 9);
  assert(uf.parents[1] == 9);
  assert(uf.parents[2] == SIZE_MAX);
  assert(uf.parents[3] == SIZE_MAX);
  assert(uf.parents[4] == 9);
  assert(uf.parents[5] == 9);
  assert(uf.parents[6] == SIZE_MAX);
  assert(uf.parents[7] == SIZE_MAX);
  assert(uf.parents[8] == SIZE_MAX);
  assert(uf.parents[9] == 8);

  stufflib_unionfind_union(uf, 2, 3);
  assert(uf.parents[0] == 9);
  assert(uf.parents[1] == 9);
  assert(uf.parents[2] == SIZE_MAX);
  assert(uf.parents[3] == 2);
  assert(uf.parents[4] == 9);
  assert(uf.parents[5] == 9);
  assert(uf.parents[6] == SIZE_MAX);
  assert(uf.parents[7] == SIZE_MAX);
  assert(uf.parents[8] == SIZE_MAX);
  assert(uf.parents[9] == 8);

  stufflib_unionfind_union(uf, 6, 2);
  assert(uf.parents[0] == 9);
  assert(uf.parents[1] == 9);
  assert(uf.parents[2] == 6);
  assert(uf.parents[3] == 2);
  assert(uf.parents[4] == 9);
  assert(uf.parents[5] == 9);
  assert(uf.parents[6] == SIZE_MAX);
  assert(uf.parents[7] == SIZE_MAX);
  assert(uf.parents[8] == SIZE_MAX);
  assert(uf.parents[9] == 8);

  stufflib_unionfind_union(uf, 7, 6);
  stufflib_unionfind_union(uf, 8, 7);
  assert(uf.parents[0] == 9);
  assert(uf.parents[1] == 9);
  assert(uf.parents[2] == 6);
  assert(uf.parents[3] == 2);
  assert(uf.parents[4] == 9);
  assert(uf.parents[5] == 9);
  assert(uf.parents[6] == 7);
  assert(uf.parents[7] == 8);
  assert(uf.parents[8] == SIZE_MAX);
  assert(uf.parents[9] == 8);

  for (size_t i = 0; i < uf.size; ++i) {
    assert(stufflib_unionfind_find_root(uf, i) == 8);
  }

  stufflib_unionfind_destroy(&uf);
  return EXIT_SUCCESS;
}
