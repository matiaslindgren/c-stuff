#ifndef SL_MISC_H_INCLUDED
#define SL_MISC_H_INCLUDED

#ifndef SL_MISC_SWAP_MAX_SIZE
  #define SL_MISC_SWAP_MAX_SIZE 1024
#endif

size_t sl_misc_parse_lil_endian(const size_t size,
                                const unsigned char data[size]);

size_t sl_misc_parse_big_endian(const size_t size,
                                const unsigned char data[size]);

unsigned char* sl_misc_encode_lil_endian(const size_t size,
                                         unsigned char dst[size],
                                         const size_t value);

unsigned char* sl_misc_encode_big_endian(const size_t size,
                                         unsigned char dst[size],
                                         const size_t value);

size_t sl_misc_midpoint(const size_t lo, const size_t hi);

size_t sl_misc_vmax_size_t(const size_t n, const size_t v[n]);

void sl_misc_swap(unsigned char a[const static 1],
                  unsigned char b[const static 1],
                  const size_t count);

const char* sl_misc_tmpdir();

bool sl_misc_is_zero(const size_t count, unsigned char data[count]);

size_t sl_misc_count_nonzero(const size_t size,
                             const size_t count,
                             unsigned char data[count]);

#endif  // SL_MISC_H_INCLUDED
