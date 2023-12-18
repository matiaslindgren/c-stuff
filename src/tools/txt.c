#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_data.h"
#include "stufflib_hashmap.h"
#include "stufflib_iterator.h"
#include "stufflib_string.h"
#include "stufflib_tokenizer.h"

bool concat(const struct sl_args args[const static 1]) {
  {
    const size_t args_count = sl_args_count_positional(args) - 1;
    const size_t expected_count = 1;
    if (args_count < expected_count) {
      SL_LOG_ERROR("concat takes %zu or more arguments, not %zu",
                   expected_count,
                   args_count);
      return false;
    }
  }

  bool is_done = false;
  struct sl_string result = {0};

  for (size_t i = 1;; ++i) {
    const char* path = sl_args_get_positional(args, i);
    if (!path) {
      break;
    }
    struct sl_string content = sl_string_from_file(path);
    const bool read_ok = content.length > 0;
    if (read_ok) {
      sl_string_extend(&result, &content);
    }
    sl_string_delete(&content);
    if (!read_ok) {
      goto done;
    }
  }

  if (!sl_string_fprint(stdout, &result)) {
    goto done;
  }

  is_done = true;

done:
  sl_string_delete(&result);
  return is_done;
}

bool count(const struct sl_args args[const static 1]) {
  {
    const size_t args_count = sl_args_count_positional(args) - 1;
    const size_t expected_count = 2;
    if (args_count != expected_count) {
      SL_LOG_ERROR("count takes %zu arguments, not %zu",
                   expected_count,
                   args_count);
      return false;
    }
  }

  const char* pattern_str = sl_args_get_positional(args, 1);
  if (strlen(pattern_str) == 0) {
    SL_LOG_ERROR("pattern to count cannot be empty");
    return false;
  }

  bool is_done = false;

  const char* path = sl_args_get_positional(args, 2);
  struct sl_string content = sl_string_from_file(path);

  struct sl_data pattern =
      sl_data_view(strlen(pattern_str), (unsigned char*)pattern_str);
  struct sl_tokenizer pattern_tokenizer =
      sl_tokenizer_create(&(content.utf8_data), &pattern);

  size_t n = 0;
  for (struct sl_iterator iter = sl_tokenizer_iter(&pattern_tokenizer);
       !iter.is_done(&iter);
       iter.advance(&iter)) {
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
  sl_string_delete(&content);
  return is_done;
}

bool slicelines(const struct sl_args args[const static 1]) {
  {
    const size_t args_count = sl_args_count_positional(args) - 1;
    const size_t expected_count = 3;
    if (args_count != expected_count) {
      SL_LOG_ERROR("slicelines takes %zu arguments, not %zu",
                   expected_count,
                   args_count);
      return false;
    }
  }

  const size_t begin = strtoull(sl_args_get_positional(args, 1), 0, 10);
  const size_t end = strtoull(sl_args_get_positional(args, 2), 0, 10);
  const char* path = sl_args_get_positional(args, 3);

  bool is_done = false;

  struct sl_string content = sl_string_from_file(path);
  if (!content.length) {
    goto done;
  }

  const struct sl_data newline = sl_data_view(1, (unsigned char[]){'\n'});
  struct sl_tokenizer newline_tokenizer =
      sl_tokenizer_create(&(content.utf8_data), &newline);

  size_t lineno = 1;
  for (struct sl_iterator iter = sl_tokenizer_iter(&newline_tokenizer);
       !iter.is_done(&iter);
       iter.advance(&iter)) {
    if (begin <= lineno && lineno <= end) {
      struct sl_string line = sl_string_from_utf8(iter.get_item(&iter));
      bool write_ok = sl_string_fprint(stdout, &line);
      sl_string_delete(&line);
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
  sl_string_delete(&content);
  return is_done;
}

bool replace(const struct sl_args args[const static 1]) {
  {
    const size_t args_count = sl_args_count_positional(args) - 1;
    const size_t expected_count = 3;
    if (args_count != expected_count) {
      SL_LOG_ERROR("replace takes %zu arguments, not %zu",
                   expected_count,
                   args_count);
      return false;
    }
  }

  const char* pattern_str = sl_args_get_positional(args, 1);
  const char* replacement_str = sl_args_get_positional(args, 2);
  const char* path = sl_args_get_positional(args, 3);

  if (strlen(pattern_str) == 0) {
    // TODO allow empty pattern str to split at all codepoints
    SL_LOG_ERROR("pattern to replace cannot be empty");
    return false;
  }

  bool is_done = false;

  struct sl_string content = sl_string_from_file(path);
  if (!content.length) {
    goto done;
  }

  struct sl_data pattern = sl_data_from_str(pattern_str);
  if (sl_data_is_hexadecimal_str(&pattern)) {
    struct sl_data hex_pattern = sl_data_parse_hex(&pattern);
    sl_data_delete(&pattern);
    pattern = hex_pattern;
  }

  struct sl_data replacement = sl_data_from_str(replacement_str);
  if (sl_data_is_hexadecimal_str(&replacement)) {
    struct sl_data hex_replacement = sl_data_parse_hex(&replacement);
    sl_data_delete(&replacement);
    replacement = hex_replacement;
  }

  struct sl_tokenizer pattern_tokenizer =
      sl_tokenizer_create(&(content.utf8_data), &pattern);

  struct sl_iterator iter = sl_tokenizer_iter(&pattern_tokenizer);
  while (!iter.is_done(&iter)) {
    struct sl_string line = sl_string_from_utf8(iter.get_item(&iter));
    const bool write_ok = sl_string_fprint(stdout, &line);
    sl_string_delete(&line);
    if (!write_ok) {
      goto done;
    }
    iter.advance(&iter);
    if (!iter.is_done(&iter) && replacement.size) {
      struct sl_string repl = sl_string_from_utf8(&replacement);
      const bool write_ok = sl_string_fprint(stdout, &repl);
      sl_string_delete(&repl);
      if (!write_ok) {
        goto done;
      }
    }
  }

  is_done = true;

done:
  sl_string_delete(&content);
  sl_data_delete(&pattern);
  sl_data_delete(&replacement);
  return is_done;
}

bool linefreq(const struct sl_args args[const static 1]) {
  {
    const size_t args_count = sl_args_count_positional(args) - 1;
    const size_t expected_count = 1;
    if (args_count != expected_count) {
      SL_LOG_ERROR("linefreq takes %zu arguments, not %zu",
                   expected_count,
                   args_count);
      return false;
    }
  }

  const char* path = sl_args_get_positional(args, 1);

  bool is_done = false;

  struct sl_hashmap freq = sl_hashmap_create();
  struct sl_string content = sl_string_from_file(path);
  if (!content.length) {
    goto done;
  }

  const struct sl_data newline = sl_data_view(1, (unsigned char[]){'\n'});
  struct sl_tokenizer newline_tokenizer =
      sl_tokenizer_create(&(content.utf8_data), &newline);

  for (struct sl_iterator line_iter = sl_tokenizer_iter(&newline_tokenizer);
       !line_iter.is_done(&line_iter);
       line_iter.advance(&line_iter)) {
    struct sl_data* line = line_iter.get_item(&line_iter);
    if (!(line->size)) {
      continue;
    }
    if (!sl_hashmap_contains(&freq, line)) {
      sl_hashmap_insert(&freq, line, 0);
    }
    ++(sl_hashmap_get(&freq, line)->value);
  }

  for (struct sl_iterator freq_iter = sl_hashmap_iter(&freq);
       !freq_iter.is_done(&freq_iter);
       freq_iter.advance(&freq_iter)) {
    struct sl_hashmap_slot* slot = freq_iter.get_item(&freq_iter);
    if (printf("%zu ", slot->value) < 0) {
      goto done;
    }
    struct sl_string key_str = sl_string_from_utf8(&(slot->key));
    bool write_ok = sl_string_fprint(stdout, &key_str);
    sl_string_delete(&key_str);
    if (!write_ok || (printf("\n") < 0)) {
      goto done;
    }
  }

  is_done = true;

done:
  sl_string_delete(&content);
  sl_hashmap_delete(&freq);
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
          args->program,
          args->program,
          args->program,
          args->program,
          args->program);
}

int main(int argc, char* const argv[argc + 1]) {
  struct sl_args args = sl_args_from_argv(argc, argv);
  bool ok = false;
  const char* command = sl_args_get_positional(&args, 0);
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
  sl_args_destroy(&args);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
