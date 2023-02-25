#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_io.h"
#include "stufflib_sort.h"
#include "stufflib_string.h"
#include "stufflib_tokenizer.h"

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

  stufflib_string content = (stufflib_string){0};
  char** lines = nullptr;
  size_t num_lines = 0;

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
  content = stufflib_string_from_file(path);
  stufflib_data newline = stufflib_data_view(1, (unsigned char[]){'\n'});

  stufflib_tokenizer newline_tokenizer =
      stufflib_tokenizer_create(&(content.utf8_data), &newline);
  for (stufflib_iterator iter = stufflib_tokenizer_iter(&newline_tokenizer);
       !iter.is_done(&iter);
       iter.advance(&iter)) {
    // TODO less awful
    stufflib_data* token = iter.get_item(&iter);
    assert(token->size);
    if (token->data[0] == 0) {
      continue;
    }
    stufflib_string line = stufflib_string_from_utf8(token);
    lines = stufflib_realloc(lines, num_lines, num_lines + 1, sizeof(char*));
    lines[num_lines++] = (char*)line.utf8_data.data;
  }

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
  stufflib_string_delete(&content);
  for (size_t i = 0; i < num_lines; ++i) {
    stufflib_free(lines[i]);
  }
  stufflib_free(lines);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
