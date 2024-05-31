#include <assert.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_filesystem.h"
#include "stufflib_macros.h"
#include "stufflib_span.h"
#include "stufflib_test_data.h"

bool test_slurp_file(const bool) {
  for (size_t i = 0; i < SL_ARRAY_LEN(sl_test_data_file_paths); ++i) {
    struct sl_span data = sl_fs_slurp_file(sl_test_data_file_paths[i]);
    assert(data.owned);
    assert(data.size == sl_test_data_file_sizes[i]);
    assert(data.data != nullptr);
    sl_span_delete(&data);
  }
  return true;
}

SL_TEST_MAIN(test_slurp_file)
