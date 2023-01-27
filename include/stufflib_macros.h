#ifndef _STUFFLIB_MACROS_H_INCLUDED
#define _STUFFLIB_MACROS_H_INCLUDED

#define STUFFLIB_MIN(x, y) ((x) < (y) ? (x) : (y))
#define STUFFLIB_MAX(x, y) ((x) < (y) ? (y) : (x))

#define STUFFLIB_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define STUFFLIB_ONES(n) ((1 << ((n)*CHAR_BIT)) - 1)

#define STUFFLIB_PRINT_ERROR(msg, ...)                       \
  do {                                                       \
    fprintf(stderr, "error (%s@L%d): ", __FILE__, __LINE__); \
    fprintf(stderr, (msg), ##__VA_ARGS__);                   \
    fprintf(stderr, "\n");                                   \
  } while (0)

#define STUFFLIB_TEST_MAIN(...)                                         \
  int main(int argc, char* const argv[argc + 1]) {                      \
    const int verbose = stufflib_argv_parse_flag(argc, argv, "-v");     \
    int (*tests[])(const int) = {__VA_ARGS__};                          \
    char** test_names = stufflib_str_split(#__VA_ARGS__, ", ");         \
    assert(test_names);                                                 \
    for (size_t t = 0; t < STUFFLIB_ARRAY_LEN(tests); ++t) {            \
      if (verbose) {                                                    \
        printf("%s\n", test_names[t]);                                  \
      }                                                                 \
      if (!tests[t](verbose)) {                                         \
        STUFFLIB_PRINT_ERROR("test %s (%zu) failed", test_names[t], t); \
        stufflib_str_split_destroy(test_names);                         \
        return EXIT_FAILURE;                                            \
      }                                                                 \
    }                                                                   \
    stufflib_str_split_destroy(test_names);                             \
    return EXIT_SUCCESS;                                                \
  }

#endif  // _STUFFLIB_MACROS_H_INCLUDED
