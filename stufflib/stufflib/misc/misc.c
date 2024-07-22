#include <assert.h>
#include <limits.h>
#include <stdlib.h>

#include "../macros/macros.h"
#include "./misc.h"

size_t sl_misc_parse_lil_endian(const size_t size,
                                const unsigned char data[size]) {
  size_t x = 0;
  for (size_t i = size - 1; i < size; --i) {
    x <<= CHAR_BIT;
    x |= data[i];
  }
  return x;
}

size_t sl_misc_parse_big_endian(const size_t size,
                                const unsigned char data[size]) {
  size_t x = 0;
  for (size_t i = 0; i < size; ++i) {
    x <<= CHAR_BIT;
    x |= data[i];
  }
  return x;
}

unsigned char* sl_misc_encode_lil_endian(const size_t size,
                                         unsigned char dst[size],
                                         const size_t value) {
  size_t tmp = value;
  for (size_t i = 0; i < size; ++i) {
    dst[i] = tmp & 0xff;
    tmp >>= CHAR_BIT;
  }
  return dst;
}

unsigned char* sl_misc_encode_big_endian(const size_t size,
                                         unsigned char dst[size],
                                         const size_t value) {
  size_t tmp = value;
  for (size_t i = size - 1; i < size; --i) {
    dst[i] = tmp & 0xff;
    tmp >>= CHAR_BIT;
  }
  return dst;
}

size_t sl_misc_midpoint(const size_t lo, const size_t hi) {
  return lo + (hi - lo) / 2;
}

size_t sl_misc_vmax_size_t(const size_t n, const size_t v[n]) {
  size_t res = 0;
  for (size_t i = 0; i < n; ++i) {
    res = SL_MAX(res, v[i]);
  }
  return res;
}

void sl_misc_swap(unsigned char a[const static 1],
                  unsigned char b[const static 1],
                  const size_t count) {
  assert(count < SL_MISC_SWAP_MAX_SIZE);
  static unsigned char tmp[SL_MISC_SWAP_MAX_SIZE] = {0};
  memcpy(tmp, a, count);
  memcpy(a, b, count);
  memcpy(b, tmp, count);
}

const char* sl_misc_tmpdir() {
  const char* tmpdir = getenv("SL_TMP_DIR");
  if (!tmpdir) {
    return "/tmp";
  }
  return tmpdir;
}

bool sl_misc_is_zero(const size_t count, unsigned char data[count]) {
  for (size_t i = 0; i < count; ++i) {
    if (data[i]) {
      return false;
    }
  }
  return true;
}

size_t sl_misc_count_nonzero(const size_t size,
                             const size_t count,
                             unsigned char data[count]) {
  size_t n = 0;
  for (size_t i = 0; i < count; ++i) {
    n += !sl_misc_is_zero(size, data + i * size);
  }
  return n;
}
