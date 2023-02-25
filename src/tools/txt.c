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

bool concat(const stufflib_args args[const static 1]) {
  {
    const size_t args_count = stufflib_args_count_positional(args) - 1;
    const size_t expected_count = 1;
    if (args_count < expected_count) {
      STUFFLIB_LOG_ERROR("concat takes %zu or more arguments, not %zu",
                         expected_count,
                         args_count);
      return false;
    }
  }

  bool ok = false;
  stufflib_string result = (stufflib_string){0};

  for (size_t i = 1;; ++i) {
    const char* path = stufflib_args_get_positional(args, i);
    if (!path) {
      break;
    }
    stufflib_string content = stufflib_string_from_file(path);
    if (!content.length) {
      goto done;
    }
    stufflib_string_extend(&result, &content);
    stufflib_string_delete(&content);
  }

  if (stufflib_string_fprint(stdout, &result, L"", L"") < 0) {
    goto done;
  }

  ok = true;

done:
  stufflib_string_delete(&result);
  return ok;
}

bool count(const stufflib_args args[const static 1]) {
  {
    const size_t args_count = stufflib_args_count_positional(args) - 1;
    const size_t expected_count = 2;
    if (args_count != expected_count) {
      STUFFLIB_LOG_ERROR("count takes %zu arguments, not %zu",
                         expected_count,
                         args_count);
      return false;
    }
  }

  const char* pattern_str = stufflib_args_get_positional(args, 1);
  if (strlen(pattern_str) == 0) {
    STUFFLIB_LOG_ERROR("pattern to count cannot be empty");
    return false;
  }

  bool ok = false;

  const char* path = stufflib_args_get_positional(args, 2);
  stufflib_string content = stufflib_string_from_file(path);

  stufflib_data pattern =
      stufflib_data_view(strlen(pattern_str), (unsigned char*)pattern_str);
  stufflib_tokenizer pattern_tokenizer =
      stufflib_tokenizer_create(&(content.utf8_data), &pattern);

  size_t n = 0;
  for (stufflib_iterator iter = stufflib_tokenizer_iter(&pattern_tokenizer);
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

  ok = true;

done:
  stufflib_string_delete(&content);
  return ok;
}

bool slicelines(const stufflib_args args[const static 1]) {
  {
    const size_t args_count = stufflib_args_count_positional(args) - 1;
    const size_t expected_count = 3;
    if (args_count != expected_count) {
      STUFFLIB_LOG_ERROR("slicelines takes %zu arguments, not %zu",
                         expected_count,
                         args_count);
      return false;
    }
  }

  const size_t begin = strtoull(stufflib_args_get_positional(args, 1), 0, 10);
  const size_t end = strtoull(stufflib_args_get_positional(args, 2), 0, 10);
  const char* path = stufflib_args_get_positional(args, 3);

  bool ok = false;

  stufflib_string content = stufflib_string_from_file(path);
  if (!content.length) {
    goto done;
  }

  const stufflib_data newline = stufflib_data_view(1, (unsigned char[]){'\n'});
  stufflib_tokenizer newline_tokenizer =
      stufflib_tokenizer_create(&(content.utf8_data), &newline);

  size_t lineno = 1;
  for (stufflib_iterator iter = stufflib_tokenizer_iter(&newline_tokenizer);
       !iter.is_done(&iter);
       iter.advance(&iter)) {
    if (begin <= lineno && lineno <= end) {
      stufflib_string line = stufflib_string_from_utf8(iter.get_item(&iter));
      const int ret = stufflib_string_fprint(stdout, &line, L"", L"\n");
      stufflib_string_delete(&line);
      if (ret < 0) {
        goto done;
      }
    }
    ++lineno;
  }

  ok = true;

done:
  stufflib_string_delete(&content);
  return ok;
}

bool replace(const stufflib_args args[const static 1]) {
  {
    const size_t args_count = stufflib_args_count_positional(args) - 1;
    const size_t expected_count = 3;
    if (args_count != expected_count) {
      STUFFLIB_LOG_ERROR("replace takes %zu arguments, not %zu",
                         expected_count,
                         args_count);
      return false;
    }
  }

  const char* pattern_str = stufflib_args_get_positional(args, 1);
  const char* replacement_str = stufflib_args_get_positional(args, 2);
  const char* path = stufflib_args_get_positional(args, 3);

  if (strlen(pattern_str) == 0) {
    // TODO allow empty pattern str to split at all codepoints
    STUFFLIB_LOG_ERROR("pattern to replace cannot be empty");
    return false;
  }

  bool ok = false;

  stufflib_string content = stufflib_string_from_file(path);
  if (!content.length) {
    goto done;
  }

  stufflib_data pattern =
      stufflib_data_view(strlen(pattern_str), (unsigned char*)pattern_str);
  stufflib_string replacement = strlen(replacement_str)
                                    ? stufflib_string_from_cstr(replacement_str)
                                    : (stufflib_string){0};

  stufflib_tokenizer pattern_tokenizer =
      stufflib_tokenizer_create(&(content.utf8_data), &pattern);

  stufflib_iterator iter = stufflib_tokenizer_iter(&pattern_tokenizer);
  while (!iter.is_done(&iter)) {
    stufflib_string line = stufflib_string_from_utf8(iter.get_item(&iter));
    const int ret = stufflib_string_fprint(stdout, &line, L"", L"");
    stufflib_string_delete(&line);
    if (ret < 0) {
      goto done;
    }
    iter.advance(&iter);
    if (!iter.is_done(&iter) && replacement.length) {
      if (stufflib_string_fprint(stdout, &replacement, L"", L"") < 0) {
        goto done;
      }
    }
  }

  ok = true;

done:
  stufflib_string_delete(&content);
  stufflib_string_delete(&replacement);
  return ok;
}

bool linefreq(const stufflib_args args[const static 1]) {
  {
    const size_t args_count = stufflib_args_count_positional(args) - 1;
    const size_t expected_count = 1;
    if (args_count != expected_count) {
      STUFFLIB_LOG_ERROR("linefreq takes %zu arguments, not %zu",
                         expected_count,
                         args_count);
      return false;
    }
  }

  const char* path = stufflib_args_get_positional(args, 1);

  bool ok = false;

  stufflib_hashmap freq = stufflib_hashmap_create();
  stufflib_string content = stufflib_string_from_file(path);
  if (!content.length) {
    goto done;
  }

  const stufflib_data newline = stufflib_data_view(1, (unsigned char[]){'\n'});
  stufflib_tokenizer newline_tokenizer =
      stufflib_tokenizer_create(&(content.utf8_data), &newline);

  for (stufflib_iterator line_iter =
           stufflib_tokenizer_iter(&newline_tokenizer);
       !line_iter.is_done(&line_iter);
       line_iter.advance(&line_iter)) {
    stufflib_data* line = line_iter.get_item(&line_iter);
    if (!(line->size)) {
      continue;
    }
    if (!stufflib_hashmap_contains(&freq, line)) {
      stufflib_hashmap_insert(&freq, line, 0);
    }
    ++(stufflib_hashmap_get(&freq, line)->value);
  }

  for (stufflib_iterator freq_iter = stufflib_hashmap_iter(&freq);
       !freq_iter.is_done(&freq_iter);
       freq_iter.advance(&freq_iter)) {
    stufflib_hashmap_slot* slot = freq_iter.get_item(&freq_iter);
    if (fwprintf(stdout, L"%zu ", slot->value) < 0) {
      goto done;
    }
    stufflib_string key_str = stufflib_string_from_utf8(&(slot->key));
    const int ret = stufflib_string_fprint(stdout, &key_str, L"", L"\n");
    stufflib_string_delete(&key_str);
    if (ret < 0) {
      goto done;
    }
  }

  ok = true;

done:
  stufflib_string_delete(&content);
  stufflib_hashmap_delete(&freq);
  return ok;
}

void print_usage(const stufflib_args args[const static 1]) {
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
  setlocale(LC_ALL, "");
  stufflib_args args = stufflib_args_from_argv(argc, argv);
  bool ok = false;
  const char* command = stufflib_args_get_positional(&args, 0);
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
      STUFFLIB_LOG_ERROR("unknown command %s", command);
    }
  }
  if (!ok) {
    print_usage(&args);
  }
  stufflib_args_destroy(&args);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
