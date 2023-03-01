#ifndef _SL_ARGS_H_INCLUDED
#define _SL_ARGS_H_INCLUDED
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_macros.h"
#include "stufflib_memory.h"

struct sl_args {
  char* program;
  // pointers to main argv
  char* const** required;
  char* const** optional;
};

struct sl_args sl_args_from_argv(const int argc, char* const argv[argc + 1]) {
  size_t num_required = 0;
  for (size_t i = 1; i < argc; ++i) {
    num_required += argv[i][0] != '-';
  }
  size_t num_optional = argc - 1 - num_required;

  char* const** required = sl_alloc(num_required + 1, sizeof(char*));
  char* const** optional = sl_alloc(num_optional + 1, sizeof(char*));

  {
    char* const** req = required;
    char* const** opt = optional;
    for (size_t i = 1; i < argc; ++i) {
      if (argv[i][0] != '-') {
        *(req++) = argv + i;
      } else {
        *(opt++) = argv + i;
      }
    }
  }

  return (struct sl_args){
      .program = argv[0],
      .required = required,
      .optional = optional,
  };
}

void sl_args_destroy(struct sl_args args[const static 1]) {
  sl_free(args->required);
  sl_free(args->optional);
  *args = (struct sl_args){0};
}

char* const sl_args_get_positional(const struct sl_args args[const static 1],
                                   const size_t pos) {
  for (size_t i = 0; args->required[i]; ++i) {
    if (i == pos) {
      return *args->required[i];
    }
  }
  return nullptr;
}

size_t sl_args_count_positional(const struct sl_args args[const static 1]) {
  size_t count = 0;
  while (args->required[count]) {
    ++count;
  }
  return count;
}

bool sl_args_parse_flag(const struct sl_args args[const static 1],
                        const char arg[const static 1]) {
  for (size_t i = 0; args->optional[i]; ++i) {
    char* const opt = *args->optional[i];
    if (strcmp(opt, arg) == 0) {
      return true;
    }
  }
  return false;
}

size_t sl_args_parse_uint(const struct sl_args args[const static 1],
                          const char arg[const static 1],
                          const int base) {
  const size_t arg_len = strlen(arg);
  if (!arg_len) {
    return 0;
  }
  for (size_t i = 0; args->optional[i]; ++i) {
    char* const opt = *args->optional[i];
    if (strncmp(opt, arg, arg_len) == 0) {
      const char* value = strchr(opt, '=');
      if (value) {
        return strtoull(value + 1, 0, base);
      }
    }
  }
  return 0;
}

#endif  // _SL_ARGS_H_INCLUDED
