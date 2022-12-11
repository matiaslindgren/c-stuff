#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_argv.h"
#include "stufflib_math.h"

void _check_factorization(int verbose, size_t x, size_t* factors) {
  assert(factors);
  if (verbose) {
    printf("%zd factors:\n", x);
    for (size_t i = 0; factors[i] != 0; ++i) {
      printf("  %zu: %zu\n", i, factors[i]);
    }
  }
}

int main(int argc, char* const argv[argc + 1]) {
  int verbose = stufflib_argv_parse_flag(argc, argv, "-v");
  {
    size_t primes[] = {2, 3, 5, 7, 11, 13, 17};
    for (size_t i = 0; i < sizeof(primes) / sizeof(primes[0]); ++i) {
      size_t* f = stufflib_math_factorize(primes[i]);
      _check_factorization(verbose, primes[i], f);
      assert(f[0] == primes[i]);
      assert(f[1] == 0);
      free(f);
    }
  }
  {
    const size_t n = 4;
    size_t* f = stufflib_math_factorize(n);
    _check_factorization(verbose, n, f);
    assert(f[0] == 2);
    assert(f[1] == 2);
    assert(f[2] == 0);
    free(f);
  }
  {
    const size_t n = 25;
    size_t* f = stufflib_math_factorize(n);
    _check_factorization(verbose, n, f);
    assert(f[0] == 5);
    assert(f[1] == 5);
    assert(f[2] == 0);
    free(f);
  }
  {
    const size_t n = 30;
    size_t* f = stufflib_math_factorize(n);
    _check_factorization(verbose, n, f);
    assert(f[0] == 2);
    assert(f[1] == 3);
    assert(f[2] == 5);
    assert(f[3] == 0);
    free(f);
  }
  {
    const size_t n = 864;
    size_t* f = stufflib_math_factorize(n);
    _check_factorization(verbose, n, f);
    assert(f[0] == 2);
    assert(f[1] == 2);
    assert(f[2] == 2);
    assert(f[3] == 2);
    assert(f[4] == 2);
    assert(f[5] == 3);
    assert(f[6] == 3);
    assert(f[7] == 3);
    assert(f[8] == 0);
    free(f);
  }
  {
    const size_t n = 2022;
    size_t* f = stufflib_math_factorize(n);
    _check_factorization(verbose, n, f);
    assert(f[0] == 2);
    assert(f[1] == 3);
    assert(f[2] == 337);
    assert(f[3] == 0);
    free(f);
  }
  {
    const size_t n = 202212;
    size_t* f = stufflib_math_factorize(n);
    _check_factorization(verbose, n, f);
    assert(f[0] == 2);
    assert(f[1] == 2);
    assert(f[2] == 3);
    assert(f[3] == 3);
    assert(f[4] == 41);
    assert(f[5] == 137);
    assert(f[6] == 0);
    free(f);
  }
  {
    const size_t n = 20221210;
    size_t* f = stufflib_math_factorize(n);
    _check_factorization(verbose, n, f);
    assert(f[0] == 2);
    assert(f[1] == 5);
    assert(f[2] == 101);
    assert(f[3] == 20021);
    assert(f[4] == 0);
    free(f);
  }
  return EXIT_SUCCESS;
}
