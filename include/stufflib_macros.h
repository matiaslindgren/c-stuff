#ifndef _SL_MACROS_H_INCLUDED
#define _SL_MACROS_H_INCLUDED

#define SL_MIN(x, y) ((x) < (y) ? (x) : (y))
#define SL_MAX(x, y) ((x) < (y) ? (y) : (x))

#define SL_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define SL_LOG(level, ...)                       \
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

#define SL_LOG_ERROR(...) SL_LOG("error", __VA_ARGS__)

#define SL_TEST_MAIN(...)                                    \
  int main(int argc, char* const argv[argc + 1]) {           \
    sl_args args = sl_args_from_argv(argc, argv);            \
    const bool verbose = sl_args_parse_flag(&args, "-v");    \
    bool (*tests[])(const bool) = {__VA_ARGS__};             \
    char test_names_str[] = #__VA_ARGS__;                    \
    bool ok = true;                                          \
    const char* test_name = "";                              \
    for (size_t t = 0; ok && t < SL_ARRAY_LEN(tests); ++t) { \
      test_name = strtok(t ? 0 : test_names_str, ", ");      \
      assert(test_name);                                     \
      if (verbose) {                                         \
        printf("%s\n", test_name);                           \
      }                                                      \
      ok = tests[t](verbose);                                \
      if (!ok) {                                             \
        SL_LOG_ERROR("test %s (%zu) failed", test_name, t);  \
      }                                                      \
    }                                                        \
    sl_args_destroy(&args);                                  \
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;                 \
  }

#endif  // _SL_MACROS_H_INCLUDED
