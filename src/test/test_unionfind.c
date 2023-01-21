#include "stufflib_argv.h"
#include "stufflib_macros.h"
#include "stufflib_misc.h"
#include "stufflib_unionfind.h"

int _check_roots(const stufflib_unionfind uf, size_t expected_roots[static 1]) {
  for (size_t i = 0; i < uf.count; ++i) {
    const size_t root = stufflib_unionfind_find_root(&uf, i);
    if (root != expected_roots[i]) {
      return 0;
    }
  }
  return 1;
}

int test_union_until_single_set(const int verbose) {
  stufflib_unionfind uf = {0};
  if (!stufflib_unionfind_init(&uf, 10)) {
    STUFFLIB_PRINT_ERROR("unable to allocate union find structure");
    return 0;
  }

  size_t expected_roots[10] = {0};
  for (size_t i = 0; i < uf.count; ++i) {
    expected_roots[i] = i;
  }

  assert(_check_roots(uf, expected_roots));

  stufflib_unionfind_union(&uf, 0, 1);
  expected_roots[1] = 0;
  assert(_check_roots(uf, expected_roots));

  stufflib_unionfind_union(&uf, 4, 5);
  expected_roots[5] = 4;
  assert(_check_roots(uf, expected_roots));

  stufflib_unionfind_union(&uf, 9, 1);
  expected_roots[0] = 9;
  expected_roots[1] = 9;
  assert(_check_roots(uf, expected_roots));

  stufflib_unionfind_union(&uf, 0, 5);
  expected_roots[4] = 9;
  expected_roots[5] = 9;
  assert(_check_roots(uf, expected_roots));

  stufflib_unionfind_union(&uf, 8, 9);
  expected_roots[0] = 8;
  expected_roots[1] = 8;
  expected_roots[4] = 8;
  expected_roots[5] = 8;
  expected_roots[9] = 8;
  assert(_check_roots(uf, expected_roots));

  stufflib_unionfind_union(&uf, 2, 3);
  expected_roots[3] = 2;
  assert(_check_roots(uf, expected_roots));

  stufflib_unionfind_union(&uf, 6, 2);
  expected_roots[2] = 6;
  expected_roots[3] = 6;
  assert(_check_roots(uf, expected_roots));

  stufflib_unionfind_union(&uf, 7, 6);
  expected_roots[2] = 7;
  expected_roots[3] = 7;
  expected_roots[6] = 7;
  assert(_check_roots(uf, expected_roots));

  stufflib_unionfind_union(&uf, 8, 7);
  for (size_t i = 0; i < uf.count; ++i) {
    expected_roots[i] = 8;
  }
  assert(_check_roots(uf, expected_roots));

  stufflib_unionfind_destroy(&uf);
  return 1;
}

int test_union_self(const int verbose) {
  stufflib_unionfind uf = {0};
  if (!stufflib_unionfind_init(&uf, 3)) {
    STUFFLIB_PRINT_ERROR("unable to allocate union find structure");
    return 0;
  }

  size_t expected_roots[3] = {0};
  for (size_t i = 0; i < uf.count; ++i) {
    expected_roots[i] = i;
  }

  assert(_check_roots(uf, expected_roots));

  for (size_t i = 0; i < uf.count; ++i) {
    stufflib_unionfind_union(&uf, i, i);
    assert(_check_roots(uf, expected_roots));
  }

  stufflib_unionfind_union(&uf, 0, 1);
  expected_roots[1] = 0;
  assert(_check_roots(uf, expected_roots));

  for (size_t i = 0; i < uf.count; ++i) {
    stufflib_unionfind_union(&uf, i, i);
    assert(_check_roots(uf, expected_roots));
  }

  stufflib_unionfind_union(&uf, 0, 1);
  assert(_check_roots(uf, expected_roots));
  stufflib_unionfind_union(&uf, 1, 1);
  assert(_check_roots(uf, expected_roots));
  stufflib_unionfind_union(&uf, 1, 0);
  assert(_check_roots(uf, expected_roots));

  stufflib_unionfind_destroy(&uf);
  return 1;
}

typedef int test_function(const int);

int main(int argc, char* const argv[argc + 1]) {
  const int verbose = stufflib_argv_parse_flag(argc, argv, "-v");

  test_function* tests[] = {
      test_union_until_single_set,
      test_union_self,
  };
  const char* test_names[] = {
      "test_union_until_single_set",
      "test_union_self",
  };

  for (size_t t = 0; t < STUFFLIB_ARRAY_LEN(tests); ++t) {
    if (verbose) {
      printf("\n%s\n", test_names[t]);
    }
    const int ok = tests[t](verbose);
    if (!ok) {
      STUFFLIB_PRINT_ERROR("test %s (test idx: %zu) failed", test_names[t], t);
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
