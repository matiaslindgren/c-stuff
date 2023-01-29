#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_io.h"
#include "stufflib_sort.h"
#include "stufflib_str.h"

void print_usage(const stufflib_args args[const static 1]) {
  fprintf(stderr, "usage: %s { doubles | strings } path\n", args->program);
}

void print_sorted_doubles(const size_t n, double numbers[const n]) {
  for (size_t i = 0; i < n; ++i) {
    printf("%.f\n", numbers[i]);
  }
}

void print_sorted_lines(const size_t n, char* lines[const n]) {
  for (size_t i = 0; i < n; ++i) {
    printf("%s\n", lines[i]);
  }
}

int main(int argc, char* const argv[argc + 1]) {
  int ok = 0;

  stufflib_args* args = 0;
  char** lines = 0;
  double* numbers = 0;

  args = stufflib_args_from_argv(argc, argv);
  if (stufflib_args_count_positional(args) != 2) {
    print_usage(args);
    goto done;
  }
  const char* sort_as = stufflib_args_get_positional(args, 0);
  const int sort_as_doubles = strcmp(sort_as, "doubles") == 0;
  const int sort_as_strings = strcmp(sort_as, "strings") == 0;
  if (!(sort_as_doubles || sort_as_strings)) {
    print_usage(args);
    goto done;
  }

  const char* path = stufflib_args_get_positional(args, 1);
  lines = stufflib_io_slurp_lines(path, "\n");
  if (!lines) {
    goto done;
  }

  const size_t num_lines = stufflib_str_chunks_count(lines);

  if (sort_as_doubles) {
    numbers = calloc(num_lines, sizeof(double));
    if (!numbers) {
      STUFFLIB_PRINT_ERROR("failed allocating input as array of doubles");
      goto done;
    }
    for (size_t i = 0; i < num_lines; ++i) {
      numbers[i] = strtod(lines[i], 0);
      if (errno == ERANGE) {
        STUFFLIB_PRINT_ERROR("failed converting line '%s' to double", lines[i]);
        free(numbers);
        goto done;
      }
    }
    if (!stufflib_sort_mergesort_double(num_lines, numbers)) {
      STUFFLIB_PRINT_ERROR("failed sorting input as doubles");
      goto done;
    }
    print_sorted_doubles(num_lines, numbers);
  } else {
    if (!stufflib_sort_mergesort_str(num_lines, lines)) {
      STUFFLIB_PRINT_ERROR("failed sorting input as strings");
      goto done;
    }
    print_sorted_lines(num_lines, lines);
  }

  ok = 1;

done:
  if (args) {
    stufflib_args_destroy(args);
  }
  if (lines) {
    stufflib_str_chunks_destroy(lines);
  }
  if (numbers) {
    free(numbers);
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
