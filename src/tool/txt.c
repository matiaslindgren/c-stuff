#include <stdio.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_text.h"

int concat(const stufflib_args args[const static 1]) {
  if (stufflib_args_count_positional(args) < 2) {
    STUFFLIB_PRINT_ERROR("too few arguments to concat");
    return 0;
  }

  int ok = 0;
  stufflib_text* text = 0;

  for (size_t i = 1;; ++i) {
    const char* path = stufflib_args_get_positional(args, i);
    if (!path) {
      break;
    }
    stufflib_text* next = stufflib_text_from_file(path);
    if (!next) {
      goto done;
    }
    if (text) {
      text = stufflib_text_concat(text, next);
    } else {
      text = next;
    }
  }

  if (!text) {
    goto done;
  }

  if (stufflib_text_fprint(stdout, text, "") < 0) {
    goto done;
  }

  ok = 1;

done:
  if (text) {
    stufflib_text_destroy(text);
  }
  return ok;
}

int replace(const stufflib_args args[const static 1]) {
  if (stufflib_args_count_positional(args) != 4) {
    STUFFLIB_PRINT_ERROR("too few arguments to replace");
    return 0;
  }

  int ok = 0;

  const char* old_str = stufflib_args_get_positional(args, 1);
  const char* new_str = stufflib_args_get_positional(args, 2);
  const char* path = stufflib_args_get_positional(args, 3);

  stufflib_text* text = stufflib_text_from_file(path);
  if (!text) {
    goto done;
  }
  stufflib_text* replaced = stufflib_text_replace(text, old_str, new_str);
  stufflib_text_destroy(text);
  text = replaced;
  if (!replaced) {
    goto done;
  }

  if (stufflib_text_fprint(stdout, text, "") < 0) {
    goto done;
  }

  ok = 1;

done:
  if (text) {
    stufflib_text_destroy(text);
  }
  return ok;
}

void print_usage(const stufflib_args args[const static 1]) {
  fprintf(stderr,
          ("usage:\n"
           "  %s concat path [paths...]\n"
           "  %s replace old_str new_str path\n"),
          args->program,
          args->program);
}

int main(int argc, char* const argv[argc + 1]) {
  stufflib_args* args = stufflib_args_from_argv(argc, argv);
  int ok = 0;
  const char* command = stufflib_args_get_positional(args, 0);
  if (command) {
    if (strcmp(command, "concat") == 0) {
      ok = concat(args);
    } else if (strcmp(command, "replace") == 0) {
      ok = replace(args);
    } else {
      STUFFLIB_PRINT_ERROR("unknown command %s", command);
    }
  }
  if (!ok) {
    print_usage(args);
  }
  stufflib_args_destroy(args);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
