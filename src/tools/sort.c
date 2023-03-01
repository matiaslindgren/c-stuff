#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_io.h"
#include "stufflib_sort.h"
#include "stufflib_string.h"
#include "stufflib_tokenizer.h"

void print_usage(const sl_args args[const static 1]) {
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
  return sl_sort_quicksort((void*)lines,
                           count,
                           sizeof(char*),
                           compare_as_doubles);
}

int main(int argc, char* const argv[argc + 1]) {
  bool ok = false;

  sl_string content = (sl_string){0};
  char** lines = nullptr;
  size_t num_lines = 0;

  sl_args args = sl_args_from_argv(argc, argv);
  if (sl_args_count_positional(&args) != 2) {
    print_usage(&args);
    goto done;
  }
  const char* sort_as = sl_args_get_positional(&args, 0);
  const bool sort_as_numeric = strcmp(sort_as, "numeric") == 0;
  const bool sort_as_ascii = strcmp(sort_as, "ascii") == 0;
  if (!(sort_as_numeric || sort_as_ascii)) {
    print_usage(&args);
    goto done;
  }

  const char* path = sl_args_get_positional(&args, 1);
  content = sl_string_from_file(path);
  sl_data newline = sl_data_view(1, (unsigned char[]){'\n'});

  sl_tokenizer newline_tokenizer =
      sl_tokenizer_create(&(content.utf8_data), &newline);
  for (sl_iterator iter = sl_tokenizer_iter(&newline_tokenizer);
       !iter.is_done(&iter);
       iter.advance(&iter)) {
    // TODO less awful
    sl_data* token = iter.get_item(&iter);
    assert(token->size);
    if (token->data[0] == 0) {
      continue;
    }
    sl_string line = sl_string_from_utf8(token);
    lines = sl_realloc(lines, num_lines, num_lines + 1, sizeof(char*));
    lines[num_lines++] = (char*)line.utf8_data.data;
  }

  if (sort_as_numeric) {
    if (!sort_doubles(num_lines, lines)) {
      SL_LOG_ERROR("failed sorting input as numeric");
      goto done;
    }
  } else {
    if (!sl_sort_quicksort_str(num_lines, lines)) {
      SL_LOG_ERROR("failed sorting input as ascii");
      goto done;
    }
  }

  const bool reverse = sl_args_parse_flag(&args, "--reverse");
  for (size_t i = 0; i < num_lines; ++i) {
    const size_t index = reverse ? num_lines - (i + 1) : i;
    printf("%s\n", lines[index]);
  }

  ok = true;

done:
  sl_args_destroy(&args);
  sl_string_delete(&content);
  for (size_t i = 0; i < num_lines; ++i) {
    sl_free(lines[i]);
  }
  sl_free(lines);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
