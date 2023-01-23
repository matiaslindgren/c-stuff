#ifndef _STUFFLIB_ARGV_H_INCLUDED
#define _STUFFLIB_ARGV_H_INCLUDED
#include <stdlib.h>
#include <string.h>

int stufflib_argv_parse_flag(int argc,
                             char* const argv[argc + 1],
                             const char* arg) {
  for (size_t i = 1; i < argc; ++i) {
    if (strcmp(argv[i], arg) == 0) {
      return 1;
    }
  }
  return 0;
}

size_t stufflib_argv_parse_uint(int argc,
                                char* const argv[argc + 1],
                                const char* arg,
                                const size_t default_) {
  const size_t arg_len = strlen(arg);
  if (!arg_len) {
    return default_;
  }
  for (size_t i = 1; i < argc; ++i) {
    if (strncmp(argv[i], arg, arg_len) == 0) {
      const char* value = strchr(argv[i], '=');
      if (value) {
        return strtoull(value + 1, 0, 10);
      }
    }
  }
  return default_;
}

#endif  // _STUFFLIB_ARGV_H_INCLUDED
