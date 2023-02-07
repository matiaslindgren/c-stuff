#ifndef _STUFFLIB_MACROS_H_INCLUDED
#define _STUFFLIB_MACROS_H_INCLUDED

#define STUFFLIB_MIN(x, y) ((x) < (y) ? (x) : (y))
#define STUFFLIB_MAX(x, y) ((x) < (y) ? (y) : (x))

#define STUFFLIB_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define STUFFLIB_ONES(n) ((1 << ((n)*CHAR_BIT)) - 1)

#define STUFFLIB_PRINT_ERROR(msg, ...)                   \
  do {                                                   \
    const char* const fname = __FILE__;                  \
    const unsigned long lineno = __LINE__;               \
    fprintf(stderr, "error (%s@L%ld): ", fname, lineno); \
    fprintf(stderr, (msg), ##__VA_ARGS__);               \
    fprintf(stderr, "\n");                               \
  } while (0)

#define STUFFLIB_TEST_MAIN(...)                                         \
  int main(int argc, char* const argv[argc + 1]) {                      \
    stufflib_args* args = stufflib_args_from_argv(argc, argv);          \
    const int verbose = stufflib_args_parse_flag(args, "-v");           \
    int (*tests[])(const int) = {__VA_ARGS__};                          \
    char** test_names = stufflib_str_split(#__VA_ARGS__, ", ");         \
    assert(test_names);                                                 \
    int ok = 1;                                                         \
    for (size_t t = 0; ok && t < STUFFLIB_ARRAY_LEN(tests); ++t) {      \
      if (verbose) {                                                    \
        printf("%s\n", test_names[t]);                                  \
      }                                                                 \
      ok = tests[t](verbose);                                           \
      if (!ok) {                                                        \
        STUFFLIB_PRINT_ERROR("test %s (%zu) failed", test_names[t], t); \
      }                                                                 \
    }                                                                   \
    stufflib_args_destroy(args);                                        \
    stufflib_str_chunks_destroy(test_names);                            \
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;                            \
  }

#endif  // _STUFFLIB_MACROS_H_INCLUDED
