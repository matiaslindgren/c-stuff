#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./args.h"

struct sl_args;

bool sl_args_is_flag(const struct sl_args args[const static 1], const int i) {
  return i < args->argc && args->argv[i][0] == '-';
}

int sl_args_count_positional(const struct sl_args args[const static 1]) {
  int p = 0;
  for (int i = 1; i < args->argc; ++i) {
    p += !sl_args_is_flag(args, i);
  }
  return p;
}

int sl_args_count_optional(const struct sl_args args[const static 1]) {
  return args->argc - sl_args_count_positional(args) - 1;
}

char* sl_args_get_positional(const struct sl_args args[const static 1],
                             const int pos) {
  for (int i = 1, p = 0; i < args->argc; ++i) {
    if (!sl_args_is_flag(args, i)) {
      if (p++ == pos) {
        return args->argv[i];
      }
    }
  }
  return nullptr;
}

char* sl_args_find_optional(const struct sl_args args[const static 1],
                            const char arg[const static 1]) {
  const size_t arg_len = strlen(arg);
  if (arg_len > 0) {
    for (int i = 1; i < args->argc; ++i) {
      if (sl_args_is_flag(args, i)) {
        char* const opt = args->argv[i];
        if (strncmp(opt, arg, arg_len) == 0) {
          return opt;
        }
      }
    }
  }
  return nullptr;
}

bool sl_args_parse_flag(const struct sl_args args[const static 1],
                        const char arg[const static 1]) {
  return sl_args_find_optional(args, arg) != nullptr;
}

unsigned long long sl_args_parse_ull(const struct sl_args args[const static 1],
                                     const char arg[const static 1],
                                     const int base) {
  char* const opt = sl_args_find_optional(args, arg);
  if (opt != nullptr) {
    const char* value = strchr(opt, '=');
    if (value) {
      return strtoull(value + 1, 0, base);
    }
  }
  return 0;
}
