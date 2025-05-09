#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib/args/args.h"
#include "stufflib/filesystem/filesystem.h"
#include "stufflib/hashmap/hashmap.h"
#include "stufflib/iterator/iterator.h"
#include "stufflib/span/span.h"
#include "stufflib/string/string.h"
#include "stufflib/tokenizer/tokenizer.h"

static unsigned char reader_buffer_data[1024 << 6] = {0};
static struct sl_span reader_buffer = {0};

bool concat(const struct sl_args args[const static 1]) {
  {
    const int args_count = sl_args_count_positional(args) - 1;
    const int expected_count = 1;
    if (args_count < expected_count) {
      SL_LOG_ERROR("concat takes %d or more arguments, not %d",
                   expected_count,
                   args_count);
      return false;
    }
  }

  bool is_done = false;
  struct sl_string result = {0};

  for (int i = 1;; ++i) {
    char* path = sl_args_get_positional(args, i);
    if (!path) {
      break;
    }
    struct sl_string content = sl_fs_read_file_utf8(path, &reader_buffer);
    const bool read_ok = content.length > 0;
    if (read_ok) {
      sl_string_extend(&result, &content);
    }
    sl_string_destroy(&content);
    if (!read_ok) {
      goto done;
    }
  }

  if (!sl_string_fprint(stdout, &result)) {
    goto done;
  }

  is_done = true;

done:
  sl_string_destroy(&result);
  return is_done;
}

bool count(const struct sl_args args[const static 1]) {
  {
    const int args_count = sl_args_count_positional(args) - 1;
    const int expected_count = 2;
    if (args_count != expected_count) {
      SL_LOG_ERROR("count takes %d arguments, not %d",
                   expected_count,
                   args_count);
      return false;
    }
  }

  char* pattern_str = sl_args_get_positional(args, 1);
  if (strlen(pattern_str) == 0) {
    SL_LOG_ERROR("pattern to count cannot be empty");
    return false;
  }

  bool is_done = false;

  char* path = sl_args_get_positional(args, 2);
  struct sl_string content = sl_fs_read_file_utf8(path, &reader_buffer);

  struct sl_span pattern =
      sl_span_view(strlen(pattern_str), (unsigned char*)pattern_str);
  struct sl_tokenizer pattern_tokenizer =
      sl_tokenizer_create(&(content.utf8_data), &pattern);

  size_t n = 0;
  for (struct sl_iterator iter = sl_tokenizer_iter(&pattern_tokenizer);
       !sl_tokenizer_iter_is_done(&iter);
       sl_tokenizer_iter_advance(&iter)) {
    ++n;
  }
  if (n) {
    --n;
  }
  if (printf("%zu\n", n) < 0) {
    goto done;
  }

  is_done = true;

done:
  sl_string_destroy(&content);
  return is_done;
}

bool slicelines(const struct sl_args args[const static 1]) {
  {
    const int args_count = sl_args_count_positional(args) - 1;
    const int expected_count = 3;
    if (args_count != expected_count) {
      SL_LOG_ERROR("slicelines takes %d arguments, not %d",
                   expected_count,
                   args_count);
      return false;
    }
  }

  const size_t begin = strtoull(sl_args_get_positional(args, 1), 0, 10);
  const size_t count = strtoull(sl_args_get_positional(args, 2), 0, 10);
  char* path = sl_args_get_positional(args, 3);

  bool is_done = false;

  struct sl_string content = sl_fs_read_file_utf8(path, &reader_buffer);
  if (!content.length) {
    goto done;
  }

  struct sl_span newline = sl_span_view(1, (unsigned char[]){'\n'});
  struct sl_tokenizer newline_tokenizer =
      sl_tokenizer_create(&(content.utf8_data), &newline);

  size_t lineno = 1;
  size_t n_printed = 0;
  for (struct sl_iterator iter = sl_tokenizer_iter(&newline_tokenizer);
       !sl_tokenizer_iter_is_done(&iter);
       sl_tokenizer_iter_advance(&iter)) {
    if (begin <= lineno && n_printed < count) {
      struct sl_string line = sl_string_from_utf8(sl_tokenizer_iter_get(&iter));
      bool write_ok = sl_string_fprint(stdout, &line);
      ++n_printed;
      sl_string_destroy(&line);
      if (!write_ok) {
        goto done;
      }
      if (printf("\n") < 0) {
        goto done;
      }
    }
    ++lineno;
  }

  is_done = true;

done:
  sl_string_destroy(&content);
  return is_done;
}

bool replace(const struct sl_args args[const static 1]) {
  {
    const int args_count = sl_args_count_positional(args) - 1;
    const int expected_count = 3;
    if (args_count != expected_count) {
      SL_LOG_ERROR("replace takes %d arguments, not %d",
                   expected_count,
                   args_count);
      return false;
    }
  }

  char* pattern_str = sl_args_get_positional(args, 1);
  char* replacement_str = sl_args_get_positional(args, 2);
  char* path = sl_args_get_positional(args, 3);

  if (strlen(pattern_str) == 0) {
    // TODO allow empty pattern str to split at all codepoints
    SL_LOG_ERROR("pattern to replace cannot be empty");
    return false;
  }

  bool is_done = false;

  struct sl_string content = sl_fs_read_file_utf8(path, &reader_buffer);
  if (!content.length) {
    goto done;
  }

  struct sl_span pattern = sl_span_from_str(pattern_str);
  if (sl_span_is_hexadecimal_str(&pattern)) {
    struct sl_span hex_pattern = sl_span_parse_hex(&pattern);
    sl_span_destroy(&pattern);
    pattern = hex_pattern;
  }

  struct sl_span replacement = sl_span_from_str(replacement_str);
  if (sl_span_is_hexadecimal_str(&replacement)) {
    struct sl_span hex_replacement = sl_span_parse_hex(&replacement);
    sl_span_destroy(&replacement);
    replacement = hex_replacement;
  }

  struct sl_tokenizer pattern_tokenizer =
      sl_tokenizer_create(&(content.utf8_data), &pattern);

  struct sl_iterator iter = sl_tokenizer_iter(&pattern_tokenizer);
  while (!sl_tokenizer_iter_is_done(&iter)) {
    {
      struct sl_string line = sl_string_from_utf8(sl_tokenizer_iter_get(&iter));
      const bool write_ok = sl_string_fprint(stdout, &line);
      sl_string_destroy(&line);
      if (!write_ok) {
        goto done;
      }
    }
    sl_tokenizer_iter_advance(&iter);
    if (!sl_tokenizer_iter_is_done(&iter) && replacement.size) {
      struct sl_string repl = sl_string_from_utf8(&replacement);
      const bool write_ok = sl_string_fprint(stdout, &repl);
      sl_string_destroy(&repl);
      if (!write_ok) {
        goto done;
      }
    }
  }

  is_done = true;

done:
  sl_string_destroy(&content);
  sl_span_destroy(&pattern);
  sl_span_destroy(&replacement);
  return is_done;
}

bool linefreq(const struct sl_args args[const static 1]) {
  {
    const int args_count = sl_args_count_positional(args) - 1;
    const int expected_count = 1;
    if (args_count != expected_count) {
      SL_LOG_ERROR("linefreq takes %d arguments, not %d",
                   expected_count,
                   args_count);
      return false;
    }
  }

  char* path = sl_args_get_positional(args, 1);

  bool is_done = false;

  struct sl_hashmap freq = sl_hashmap_create(1024);
  struct sl_string content = sl_fs_read_file_utf8(path, &reader_buffer);
  if (!content.length) {
    goto done;
  }

  struct sl_span newline = sl_span_view(1, (unsigned char[]){'\n'});
  struct sl_tokenizer newline_tokenizer =
      sl_tokenizer_create(&(content.utf8_data), &newline);

  for (struct sl_iterator line_iter = sl_tokenizer_iter(&newline_tokenizer);
       !sl_tokenizer_iter_is_done(&line_iter);
       sl_tokenizer_iter_advance(&line_iter)) {
    struct sl_span* line = sl_tokenizer_iter_get(&line_iter);
    if (!(line->size)) {
      continue;
    }
    if (!sl_hashmap_contains(&freq, line)) {
      sl_hashmap_insert(&freq, line, sl_hashmap_type_uint64, &((uint64_t){0}));
    }
    sl_hashmap_get(&freq, line)->value.uint64 += 1;
  }

  for (struct sl_iterator freq_iter = sl_hashmap_iter(&freq);
       !sl_hashmap_iter_is_done(&freq_iter);
       sl_hashmap_iter_advance(&freq_iter)) {
    struct sl_hashmap_slot* slot = sl_hashmap_iter_get(&freq_iter);
    if (printf("%" PRIu64 " ", slot->value.uint64) < 0) {
      goto done;
    }
    struct sl_string key_str = sl_string_from_utf8(&(slot->key));
    bool write_ok = sl_string_fprint(stdout, &key_str);
    sl_string_destroy(&key_str);
    if (!write_ok || (printf("\n") < 0)) {
      goto done;
    }
  }

  is_done = true;

done:
  sl_string_destroy(&content);
  sl_hashmap_destroy(&freq);
  return is_done;
}

void print_usage(const struct sl_args args[const static 1]) {
  fprintf(stderr,
          ("usage:"
           "\n"
           "  %s concat path [paths...]"
           "\n"
           "  %s count pattern path"
           "\n"
           "  %s slicelines begin end path"
           "\n"
           "  %s replace pattern replacement path"
           "\n"
           "  %s linefreq path"
           "\n"),
          args->argv[0],
          args->argv[0],
          args->argv[0],
          args->argv[0],
          args->argv[0]);
}

int main(int argc, char* const argv[argc + 1]) {
  struct sl_args args = {.argc = argc, .argv = argv};
  reader_buffer =
      sl_span_view(SL_ARRAY_LEN(reader_buffer_data), reader_buffer_data);
  bool ok = false;
  char* command = sl_args_get_positional(&args, 0);
  if (command) {
    if (strcmp(command, "concat") == 0) {
      ok = concat(&args);
    } else if (strcmp(command, "count") == 0) {
      ok = count(&args);
    } else if (strcmp(command, "slicelines") == 0) {
      ok = slicelines(&args);
    } else if (strcmp(command, "replace") == 0) {
      ok = replace(&args);
    } else if (strcmp(command, "linefreq") == 0) {
      ok = linefreq(&args);
    } else {
      SL_LOG_ERROR("unknown command %s", command);
    }
  }
  if (!ok) {
    print_usage(&args);
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
