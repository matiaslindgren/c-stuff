#ifndef _STUFFLIB_ARGS_H_INCLUDED
#define _STUFFLIB_ARGS_H_INCLUDED
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_macros.h"

typedef struct stufflib_args stufflib_args;
struct stufflib_args {
  char* program;
  // pointers to main argv
  char* const** required;
  char* const** optional;
};

stufflib_args* stufflib_args_from_argv(const int argc,
                                       char* const argv[argc + 1]) {
  size_t num_required = 0;
  for (size_t i = 1; i < argc; ++i) {
    num_required += argv[i][0] != '-';
  }
  size_t num_optional = argc - 1 - num_required;

  stufflib_args* args = calloc(1, sizeof(stufflib_args));
  char* const** required = calloc(num_required + 1, sizeof(char*));
  char* const** optional = calloc(num_optional + 1, sizeof(char*));
  if (!args || !required || !optional) {
    STUFFLIB_PRINT_ERROR("failed allocating memory during args parse");
    assert(0);
    return 0;
  }

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

  *args = (stufflib_args){
      .program = argv[0],
      .required = required,
      .optional = optional,
  };
  return args;
}

void stufflib_args_destroy(stufflib_args args[const static 1]) {
  if (args) {
    free(args->required);
    free(args->optional);
  }
  free(args);
}

char* const stufflib_args_get_positional(
    const stufflib_args args[const static 1],
    const size_t pos) {
  for (size_t i = 0; args->required[i]; ++i) {
    if (i == pos) {
      return *args->required[i];
    }
  }
  return 0;
}

size_t stufflib_args_count_positional(
    const stufflib_args args[const static 1]) {
  size_t count = 0;
  while (args->required[count]) {
    ++count;
  }
  return count;
}

int stufflib_args_parse_flag(const stufflib_args args[const static 1],
                             const char arg[const static 1]) {
  for (size_t i = 0; args->optional[i]; ++i) {
    char* const opt = *args->optional[i];
    if (strcmp(opt, arg) == 0) {
      return 1;
    }
  }
  return 0;
}

size_t stufflib_args_parse_uint(const stufflib_args args[const static 1],
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

#endif  // _STUFFLIB_ARGS_H_INCLUDED