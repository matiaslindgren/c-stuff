#ifndef SL_MISC_H_INCLUDED
#define SL_MISC_H_INCLUDED

#ifndef SL_MISC_SWAP_MAX_SIZE
  #define SL_MISC_SWAP_MAX_SIZE 1'024
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stufflib/macros/macros.h>

size_t sl_misc_parse_lil_endian(size_t size, const unsigned char data[size]);
size_t sl_misc_parse_big_endian(size_t size, const unsigned char data[size]);
unsigned char* sl_misc_encode_lil_endian(size_t size, unsigned char dst[size], size_t value);
unsigned char* sl_misc_encode_big_endian(size_t size, unsigned char dst[size], size_t value);
size_t sl_misc_vmax_size_t(size_t n, const size_t v[n]);
void sl_misc_swap(unsigned char a[const static 1], unsigned char b[const static 1], size_t count);
bool sl_misc_is_zero(size_t count, unsigned char data[count]);
size_t sl_misc_count_nonzero(size_t size, size_t count, unsigned char data[count]);

static inline size_t sl_misc_midpoint(const size_t lo, const size_t hi) {
  return lo + ((hi - lo) / 2);
}

static inline const char* sl_misc_tmpdir(void) {
  const char* tmpdir = getenv("SL_TMP_DIR");
  if (!tmpdir) {
    return "/tmp";
  }
  return tmpdir;
}

#endif  // SL_MISC_H_INCLUDED
