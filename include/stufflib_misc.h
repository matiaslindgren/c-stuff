#ifndef SL_MISC_H_INCLUDED
#define SL_MISC_H_INCLUDED
#include <assert.h>
#include <limits.h>
#include <stdlib.h>

#include "stufflib_macros.h"

#ifndef SL_MISC_SWAP_MAX_SIZE
  #define SL_MISC_SWAP_MAX_SIZE 1024
#endif

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
  unsigned char tmp[SL_MISC_SWAP_MAX_SIZE] = {0};
  memcpy(tmp, a, count);
  memcpy(a, b, count);
  memcpy(b, tmp, count);
}

static inline const char* sl_misc_tmpdir() {
  const char* tmpdir = getenv("SL_TMP_DIR");
  if (!tmpdir) {
    return "/tmp";
  }
  return tmpdir;
}

bool sl_misc_is_zero(size_t count, unsigned char data[count]) {
  for (size_t i = 0; i < count; ++i) {
    if (data[i]) {
      return false;
    }
  }
  return true;
}

bool sl_misc_format_path(const size_t size,
                         char buffer[const size],
                         const char path[const static 1],
                         const char name[const static 1],
                         const char suffix[const static 1]) {
  if (snprintf(buffer, size, "%s/%s.%s", path, name, suffix) >= 5) {
    return true;
  }
  SL_LOG_ERROR("failed formatting %s/%s.%s", path, name, suffix);
  return false;
}

#endif  // SL_MISC_H_INCLUDED
