#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/macros/macros.h>
#include <stufflib/testing/testing.h>
#include <stufflib/unionfind/unionfind.h>

int sl_uf_assert_roots_ok(const struct sl_unionfind uf, size_t expected_roots[static 1]) {
  for (size_t i = 0; i < uf.count; ++i) {
    const size_t root = sl_unionfind_find_root(&uf, i);
    if (root != expected_roots[i]) {
      return false;
    }
  }
  return true;
}

SL_TEST(test_union_until_single_set) {
  (void)verbose;
  struct sl_unionfind uf = {0};
  if (!sl_unionfind_init(ctx, &uf, 10)) {
    SL_LOG_ERROR("unable to allocate union find structure");
    return false;
  }

  size_t expected_roots[10] = {0};
  for (size_t i = 0; i < uf.count; ++i) {
    expected_roots[i] = i;
  }

  SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));

  sl_unionfind_union(&uf, 0, 1);
  expected_roots[1] = 0;
  SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));

  sl_unionfind_union(&uf, 4, 5);
  expected_roots[5] = 4;
  SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));

  sl_unionfind_union(&uf, 9, 1);
  expected_roots[0] = 9;
  expected_roots[1] = 9;
  SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));

  sl_unionfind_union(&uf, 0, 5);
  expected_roots[4] = 9;
  expected_roots[5] = 9;
  SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));

  sl_unionfind_union(&uf, 8, 9);
  expected_roots[0] = 8;
  expected_roots[1] = 8;
  expected_roots[4] = 8;
  expected_roots[5] = 8;
  expected_roots[9] = 8;
  SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));

  sl_unionfind_union(&uf, 2, 3);
  expected_roots[3] = 2;
  SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));

  sl_unionfind_union(&uf, 6, 2);
  expected_roots[2] = 6;
  expected_roots[3] = 6;
  SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));

  sl_unionfind_union(&uf, 7, 6);
  expected_roots[2] = 7;
  expected_roots[3] = 7;
  expected_roots[6] = 7;
  SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));

  sl_unionfind_union(&uf, 8, 7);
  for (size_t i = 0; i < uf.count; ++i) {
    expected_roots[i] = 8;
  }
  SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));

  sl_unionfind_destroy(&uf);
  return true;
}

SL_TEST(test_union_self) {
  (void)verbose;
  struct sl_unionfind uf = {0};
  if (!sl_unionfind_init(ctx, &uf, 3)) {
    SL_LOG_ERROR("unable to allocate union find structure");
    return false;
  }

  size_t expected_roots[3] = {0};
  for (size_t i = 0; i < uf.count; ++i) {
    expected_roots[i] = i;
  }

  SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));

  for (size_t i = 0; i < uf.count; ++i) {
    sl_unionfind_union(&uf, i, i);
    SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));
  }

  sl_unionfind_union(&uf, 0, 1);
  expected_roots[1] = 0;
  SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));

  for (size_t i = 0; i < uf.count; ++i) {
    sl_unionfind_union(&uf, i, i);
    SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));
  }

  sl_unionfind_union(&uf, 0, 1);
  SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));
  sl_unionfind_union(&uf, 1, 1);
  SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));
  sl_unionfind_union(&uf, 1, 0);
  SL_ASSERT_TRUE(sl_uf_assert_roots_ok(uf, expected_roots));

  sl_unionfind_destroy(&uf);
  return true;
}

SL_TEST_MAIN()
