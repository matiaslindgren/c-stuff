#include <stdio.h>
#include <stdlib.h>

#include "stufflib_text.h"

int main(int argc, char* const argv[argc + 1]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s text_path\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char* text_path = argv[1];
  struct stufflib_text* text = stufflib_text_from_file(text_path);
  if (!text) {
    goto error;
  }

  if (stufflib_text_pretty_fprint(stdout, text) < 0) {
    goto error;
  }
  printf("\n");

  if (stufflib_text_fprint(stdout, text) < 0) {
    goto error;
  }
  printf("\n");

  struct stufflib_text* lhs = text;
  struct stufflib_text* rhs = stufflib_text_split_after(lhs, 10);
  stufflib_text_pretty_fprint(stdout, lhs);
  printf("\n");
  stufflib_text_pretty_fprint(stdout, rhs);
  printf("\n");

  text = stufflib_text_concat(lhs, rhs);
  stufflib_text_pretty_fprint(stdout, text);
  printf("\n");

  stufflib_text_destroy(text);

  return EXIT_SUCCESS;

error:
  if (text) {
    stufflib_text_destroy(text);
  }
  return EXIT_FAILURE;
}
