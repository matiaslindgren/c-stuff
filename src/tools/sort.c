#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_io.h"
#include "stufflib_sort.h"

void print_usage(const stufflib_args args[const static 1]) {
  fprintf(stderr, "usage: %s { numeric | ascii } path\n", args->program);
}

int compare_as_doubles(const void* lhs, const void* rhs) {
  const char* const lhs_str = ((const char**)lhs)[0];
  const char* const rhs_str = ((const char**)rhs)[0];
  const double lhs_num = strtod(lhs_str, 0);
  assert(errno != ERANGE);
  const double rhs_num = strtod(rhs_str, 0);
  assert(errno != ERANGE);
  return (lhs_num > rhs_num) - (lhs_num < rhs_num);
}

char** sort_doubles(const size_t count, char* lines[count]) {
  return stufflib_sort_quicksort((void*)lines,
                                 count,
                                 sizeof(char*),
                                 compare_as_doubles);
}

int main(int argc, char* const argv[argc + 1]) {
  bool ok = false;

  char** lines = nullptr;

  stufflib_args args = stufflib_args_from_argv(argc, argv);
  if (stufflib_args_count_positional(&args) != 2) {
    print_usage(&args);
    goto done;
  }
  const char* sort_as = stufflib_args_get_positional(&args, 0);
  const bool sort_as_numeric = strcmp(sort_as, "numeric") == 0;
  const bool sort_as_ascii = strcmp(sort_as, "ascii") == 0;
  if (!(sort_as_numeric || sort_as_ascii)) {
    print_usage(&args);
    goto done;
  }

  const char* path = stufflib_args_get_positional(&args, 1);
  lines = stufflib_io_slurp_lines(path, "\n");
  if (!lines) {
    goto done;
  }

  const size_t num_lines = stufflib_str_chunks_count(lines);

  if (sort_as_numeric) {
    if (!sort_doubles(num_lines, lines)) {
      STUFFLIB_LOG_ERROR("failed sorting input as numeric");
      goto done;
    }
  } else {
    if (!stufflib_sort_quicksort_str(num_lines, lines)) {
      STUFFLIB_LOG_ERROR("failed sorting input as ascii");
      goto done;
    }
  }

  const bool reverse = stufflib_args_parse_flag(&args, "--reverse");
  for (size_t i = 0; i < num_lines; ++i) {
    const size_t index = reverse ? num_lines - (i + 1) : i;
    printf("%s\n", lines[index]);
  }

  ok = true;

done:
  stufflib_args_destroy(&args);
  if (lines) {
    stufflib_str_chunks_destroy(lines);
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
