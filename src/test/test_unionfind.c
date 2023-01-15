#include "stufflib_unionfind.h"

int main(int argc, char* const argv[argc + 1]) {
  const size_t num_sets = 10;
  size_t* parents = stufflib_unionfind_init(num_sets);
  if (!parents) {
    fprintf(stderr,
            "error: unable to allocate 'parents' array of size %zu\n",
            num_sets);
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < num_sets; ++i) {
    assert(parents[i] == SIZE_MAX);
  }

  stufflib_unionfind_union(num_sets, parents, 0, 1);
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

  stufflib_unionfind_union(num_sets, parents, 4, 5);
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

  stufflib_unionfind_union(num_sets, parents, 9, 1);
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

  stufflib_unionfind_union(num_sets, parents, 0, 5);
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

  stufflib_unionfind_union(num_sets, parents, 8, 9);
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

  stufflib_unionfind_union(num_sets, parents, 2, 3);
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

  stufflib_unionfind_union(num_sets, parents, 6, 2);
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

  stufflib_unionfind_union(num_sets, parents, 7, 6);
  stufflib_unionfind_union(num_sets, parents, 8, 7);
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
    assert(stufflib_unionfind_find_root(num_sets, parents, i) == 8);
  }

  free(parents);
  return EXIT_SUCCESS;
}
