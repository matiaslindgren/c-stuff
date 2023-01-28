#include <stdio.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_text.h"

int main(int argc, char* const argv[argc + 1]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s text_path\n", argv[0]);
    return EXIT_FAILURE;
  }

  stufflib_args* args = stufflib_args_from_argv(argc, argv);
  const char* text_path = stufflib_args_get_positional(args, 0);
  stufflib_args_destroy(args);
  args = 0;

  stufflib_text* text = stufflib_text_from_file(text_path);
  if (!text) {
    goto error;
  }

  {
    printf("contents in %s:\n", text_path);
    printf("----------------------------------------\n");
    if (stufflib_text_fprint(stdout, text, "\n--\n") < 0) {
      goto error;
    }
    printf("\n\n");
  }

  {
    stufflib_text* tmp = stufflib_text_split(text, "\n\n");
    if (!tmp) {
      goto error;
    }
    const size_t count = stufflib_text_count(tmp);
    printf("%zu sections in %s:\n", count, text_path);
    if (stufflib_text_fprint(stdout, tmp, "\n---\n") < 0) {
      goto error;
    }
    stufflib_text_destroy(tmp);
    printf("\n\n");
  }

  {
    stufflib_text* tmp = stufflib_text_split(text, "\n");
    if (!tmp) {
      goto error;
    }
    const size_t count = stufflib_text_count(tmp);
    printf("%zu lines in %s:\n", count, text_path);
    if (stufflib_text_fprint(stdout, tmp, "\n---\n") < 0) {
      goto error;
    }
    stufflib_text_destroy(tmp);
    printf("\n\n");
  }

  stufflib_text_destroy(text);

  return EXIT_SUCCESS;

error:
  if (text) {
    stufflib_text_destroy(text);
  }
  return EXIT_FAILURE;
}
