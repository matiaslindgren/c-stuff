#ifndef _STUFFLIB_MACROS_H_INCLUDED
#define _STUFFLIB_MACROS_H_INCLUDED

#define STUFFLIB_MIN(x, y) ((x) < (y) ? (x) : (y))
#define STUFFLIB_MAX(x, y) ((x) < (y) ? (y) : (x))

#define STUFFLIB_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define STUFFLIB_LOG(level, ...)                 \
  do {                                           \
    const char* const fname = __FILE__;          \
    const unsigned long lineno = __LINE__;       \
    fprintf(stderr, "{\"level\":\"%s\"", level); \
    fprintf(stderr, ",\"file\":\"%s\"", fname);  \
    fprintf(stderr, ",\"line\":%ld", lineno);    \
    fprintf(stderr, ",\"msg\":\"");              \
    fprintf(stderr, __VA_ARGS__);                \
    fprintf(stderr, "\"}\n");                    \
  } while (false)

#define STUFFLIB_LOG_ERROR(...) STUFFLIB_LOG("error", __VA_ARGS__)

#define STUFFLIB_TEST_MAIN(...)                                    \
  int main(int argc, char* const argv[argc + 1]) {                 \
    stufflib_args args = stufflib_args_from_argv(argc, argv);      \
    const bool verbose = stufflib_args_parse_flag(&args, "-v");    \
    bool (*tests[])(const bool) = {__VA_ARGS__};                   \
    char test_names_str[] = #__VA_ARGS__;                          \
    bool ok = true;                                                \
    const char* test_name = "";                                    \
    for (size_t t = 0; ok && t < STUFFLIB_ARRAY_LEN(tests); ++t) { \
      test_name = strtok(t ? 0 : test_names_str, ", ");            \
      assert(test_name);                                           \
      if (verbose) {                                               \
        printf("%s\n", test_name);                                 \
      }                                                            \
      ok = tests[t](verbose);                                      \
      if (!ok) {                                                   \
        STUFFLIB_LOG_ERROR("test %s (%zu) failed", test_name, t);  \
      }                                                            \
    }                                                              \
    stufflib_args_destroy(&args);                                  \
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;                       \
  }

#endif  // _STUFFLIB_MACROS_H_INCLUDED
