#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib.h"

int _is_prime(size_t x) {
  for (size_t i = 2; i * i < x; ++i) {
    if (x % i == 0) {
      return 0;
    }
  }
  return 1;
}

size_t _next_prime(size_t x) {
  for (size_t p = x + 1;; ++p) {
    if (_is_prime(p)) {
      return p;
    }
  }
}

size_t* wheel_factorization(size_t n) {
  assert(n > 1);

  size_t capacity = 4;
  size_t* factors = malloc(capacity * sizeof(size_t));
  if (!factors) {
    return NULL;
  }

  size_t num_factors = 0;
  size_t k = _next_prime(1);
  while (k <= n) {
    if (n % k == 0) {
      if (num_factors >= capacity) {
        capacity *= 2;
        {
          size_t* resized = realloc(factors, capacity * sizeof(size_t));
          if (!resized) {
            goto error;
          }
          factors = resized;
        }
      }
      factors[num_factors] = k;
      ++num_factors;
      n /= k;
    } else {
      k = _next_prime(k);
    }
  }

  {
    size_t* resized = realloc(factors, (num_factors + 1) * sizeof(size_t));
    if (!resized) {
      goto error;
    }
    factors = resized;
  }
  factors[num_factors] = 0;

  return factors;

error:
  free(factors);
  return NULL;
}

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
  int verbose = stufflib_parse_argv_flag(argc, argv, "-v");
  {
    size_t primes[] = {2, 3, 5, 7, 11, 13, 17};
    for (size_t i = 0; i < sizeof(primes) / sizeof(primes[0]); ++i) {
      size_t* f = wheel_factorization(primes[i]);
      _check_factorization(verbose, primes[i], f);
      assert(f[0] == primes[i]);
      assert(f[1] == 0);
      free(f);
    }
  }
  {
    const size_t n = 4;
    size_t* f = wheel_factorization(n);
    _check_factorization(verbose, n, f);
    assert(f[0] == 2);
    assert(f[1] == 2);
    assert(f[2] == 0);
    free(f);
  }
  {
    const size_t n = 25;
    size_t* f = wheel_factorization(n);
    _check_factorization(verbose, n, f);
    assert(f[0] == 5);
    assert(f[1] == 5);
    assert(f[2] == 0);
    free(f);
  }
  {
    const size_t n = 30;
    size_t* f = wheel_factorization(n);
    _check_factorization(verbose, n, f);
    assert(f[0] == 2);
    assert(f[1] == 3);
    assert(f[2] == 5);
    assert(f[3] == 0);
    free(f);
  }
  {
    const size_t n = 864;
    size_t* f = wheel_factorization(n);
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
    size_t* f = wheel_factorization(n);
    _check_factorization(verbose, n, f);
    assert(f[0] == 2);
    assert(f[1] == 3);
    assert(f[2] == 337);
    assert(f[3] == 0);
    free(f);
  }
  {
    const size_t n = 202212;
    size_t* f = wheel_factorization(n);
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
    size_t* f = wheel_factorization(n);
    _check_factorization(verbose, n, f);
    assert(f[0] == 2);
    assert(f[1] == 5);
    assert(f[2] == 101);
    assert(f[3] == 20021);
    assert(f[4] == 0);
    free(f);
  }
  return 0;
}
