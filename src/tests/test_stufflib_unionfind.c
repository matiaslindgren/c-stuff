#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_macros.h"
#include "stufflib_unionfind.h"

int _check_roots(const struct sl_unionfind uf,
                 size_t expected_roots[static 1]) {
  for (size_t i = 0; i < uf.count; ++i) {
    const size_t root = sl_unionfind_find_root(&uf, i);
    if (root != expected_roots[i]) {
      return false;
    }
  }
  return true;
}

bool test_union_until_single_set(const bool verbose) {
  struct sl_unionfind uf = {0};
  if (!sl_unionfind_init(&uf, 10)) {
    SL_LOG_ERROR("unable to allocate union find structure");
    return false;
  }

  size_t expected_roots[10] = {0};
  for (size_t i = 0; i < uf.count; ++i) {
    expected_roots[i] = i;
  }

  assert(_check_roots(uf, expected_roots));

  sl_unionfind_union(&uf, 0, 1);
  expected_roots[1] = 0;
  assert(_check_roots(uf, expected_roots));

  sl_unionfind_union(&uf, 4, 5);
  expected_roots[5] = 4;
  assert(_check_roots(uf, expected_roots));

  sl_unionfind_union(&uf, 9, 1);
  expected_roots[0] = 9;
  expected_roots[1] = 9;
  assert(_check_roots(uf, expected_roots));

  sl_unionfind_union(&uf, 0, 5);
  expected_roots[4] = 9;
  expected_roots[5] = 9;
  assert(_check_roots(uf, expected_roots));

  sl_unionfind_union(&uf, 8, 9);
  expected_roots[0] = 8;
  expected_roots[1] = 8;
  expected_roots[4] = 8;
  expected_roots[5] = 8;
  expected_roots[9] = 8;
  assert(_check_roots(uf, expected_roots));

  sl_unionfind_union(&uf, 2, 3);
  expected_roots[3] = 2;
  assert(_check_roots(uf, expected_roots));

  sl_unionfind_union(&uf, 6, 2);
  expected_roots[2] = 6;
  expected_roots[3] = 6;
  assert(_check_roots(uf, expected_roots));

  sl_unionfind_union(&uf, 7, 6);
  expected_roots[2] = 7;
  expected_roots[3] = 7;
  expected_roots[6] = 7;
  assert(_check_roots(uf, expected_roots));

  sl_unionfind_union(&uf, 8, 7);
  for (size_t i = 0; i < uf.count; ++i) {
    expected_roots[i] = 8;
  }
  assert(_check_roots(uf, expected_roots));

  sl_unionfind_destroy(&uf);
  return true;
}

bool test_union_self(const bool verbose) {
  struct sl_unionfind uf = {0};
  if (!sl_unionfind_init(&uf, 3)) {
    SL_LOG_ERROR("unable to allocate union find structure");
    return false;
  }

  size_t expected_roots[3] = {0};
  for (size_t i = 0; i < uf.count; ++i) {
    expected_roots[i] = i;
  }

  assert(_check_roots(uf, expected_roots));

  for (size_t i = 0; i < uf.count; ++i) {
    sl_unionfind_union(&uf, i, i);
    assert(_check_roots(uf, expected_roots));
  }

  sl_unionfind_union(&uf, 0, 1);
  expected_roots[1] = 0;
  assert(_check_roots(uf, expected_roots));

  for (size_t i = 0; i < uf.count; ++i) {
    sl_unionfind_union(&uf, i, i);
    assert(_check_roots(uf, expected_roots));
  }

  sl_unionfind_union(&uf, 0, 1);
  assert(_check_roots(uf, expected_roots));
  sl_unionfind_union(&uf, 1, 1);
  assert(_check_roots(uf, expected_roots));
  sl_unionfind_union(&uf, 1, 0);
  assert(_check_roots(uf, expected_roots));

  sl_unionfind_destroy(&uf);
  return true;
}

SL_TEST_MAIN(test_union_until_single_set, test_union_self)
