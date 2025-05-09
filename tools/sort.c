#include "stufflib/sort/sort.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib/args/args.h"
#include "stufflib/filesystem/filesystem.h"
#include "stufflib/io/io.h"
#include "stufflib/string/string.h"
#include "stufflib/tokenizer/tokenizer.h"

void print_usage(const struct sl_args args[const static 1]) {
  fprintf(stderr, "usage: %s { numeric | ascii } path\n", args->argv[0]);
}

int compare_as_doubles(const void* lhs_data, const void* rhs_data) {
  const char* const* lhs = lhs_data;
  const char* const* rhs = rhs_data;
  const double lhs_num = strtod(lhs[0], 0);
  assert(errno != ERANGE);
  const double rhs_num = strtod(rhs[0], 0);
  assert(errno != ERANGE);
  return (lhs_num > rhs_num) - (lhs_num < rhs_num);
}

char** sort_doubles(const size_t count, char* lines[count]) {
  return sl_sort_quicksort(lines, count, sizeof(char*), compare_as_doubles);
}

int main(int argc, char* const argv[argc + 1]) {
  bool is_done = false;

  unsigned char reader_buffer_data[1024 << 6] = {0};
  struct sl_span reader_buffer =
      sl_span_view(SL_ARRAY_LEN(reader_buffer_data), reader_buffer_data);

  struct sl_string content = {0};
  char** lines = nullptr;
  size_t num_lines = 0;

  struct sl_args args = {.argc = argc, .argv = argv};
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
  content = sl_fs_read_file_utf8(path, &reader_buffer);
  struct sl_span newline = sl_span_view(1, (unsigned char[]){'\n'});

  struct sl_tokenizer newline_tokenizer =
      sl_tokenizer_create(&(content.utf8_data), &newline);
  for (struct sl_iterator iter = sl_tokenizer_iter(&newline_tokenizer);
       !sl_tokenizer_iter_is_done(&iter);
       sl_tokenizer_iter_advance(&iter)) {
    // TODO less awful
    struct sl_span* token = sl_tokenizer_iter_get(&iter);
    assert(token->size);
    if (token->data[0] == 0) {
      continue;
    }
    struct sl_string line = sl_string_from_utf8(token);
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

  is_done = true;

done:
  sl_string_destroy(&content);
  for (size_t i = 0; i < num_lines; ++i) {
    sl_free(lines[i]);
  }
  sl_free(lines);
  return is_done ? EXIT_SUCCESS : EXIT_FAILURE;
}
