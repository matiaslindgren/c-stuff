#include <stdio.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_text.h"

int read_concat_and_print(const stufflib_args args[const static 1]) {
  int ok = 1;
  stufflib_text* text = 0;

  for (size_t i = 1;; ++i) {
    const char* path = stufflib_args_get_positional(args, i);
    if (!path) {
      break;
    }
    stufflib_text* next = stufflib_text_from_file(path);
    if (!next) {
      ok = 0;
      goto done;
    }
    if (text) {
      text = stufflib_text_concat(text, next);
    } else {
      text = next;
    }
  }

  if (!text) {
    ok = 0;
    goto done;
  }

  if (stufflib_text_fprint(stdout, text, "") < 0) {
    ok = 0;
    goto done;
  }

done:
  if (text) {
    stufflib_text_destroy(text);
  }
  return ok;
}

void print_usage(const stufflib_args args[const static 1]) {
  fprintf(stderr,
          ("usage:\n"
           "  %s concat path [paths...]\n"),
          args->program);
}

int main(int argc, char* const argv[argc + 1]) {
  stufflib_args* args = stufflib_args_from_argv(argc, argv);
  int ok = 1;

  const char* command = stufflib_args_get_positional(args, 0);
  if (strcmp(command, "concat") == 0) {
    ok = read_concat_and_print(args);
  } else {
    ok = 0;
    STUFFLIB_PRINT_ERROR("unknown command %s", command);
    print_usage(args);
    goto done;
  }

done:
  stufflib_args_destroy(args);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
