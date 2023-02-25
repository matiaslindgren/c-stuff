#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_hashmap.h"
#include "stufflib_string.h"
#include "stufflib_tokenizer.h"

bool concat(const stufflib_args args[const static 1]) {
  if (stufflib_args_count_positional(args) < 2) {
    STUFFLIB_LOG_ERROR("too few arguments to concat");
    return false;
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
  if (stufflib_args_count_positional(args) != 3) {
    STUFFLIB_LOG_ERROR("too few arguments to count");
    return false;
  }

  bool ok = false;

  const char* path = stufflib_args_get_positional(args, 2);
  stufflib_string content = stufflib_string_from_file(path);

  const char* pattern_str = stufflib_args_get_positional(args, 1);
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
  if (stufflib_args_count_positional(args) != 4) {
    STUFFLIB_LOG_ERROR("too few arguments to slicelines");
    return false;
  }

  bool ok = false;

  const size_t begin = strtoull(stufflib_args_get_positional(args, 1), 0, 10);
  const size_t end = strtoull(stufflib_args_get_positional(args, 2), 0, 10);
  const char* path = stufflib_args_get_positional(args, 3);

  stufflib_string content = stufflib_string_from_file(path);
  stufflib_data newline = stufflib_data_view(1, (unsigned char[]){'\n'});

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

bool count_words(const stufflib_args args[const static 1]) {
  if (stufflib_args_count_positional(args) != 2) {
    STUFFLIB_LOG_ERROR("too few arguments to count_words");
    return false;
  }
  return false;
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
           "  %s count_words path"
           "\n"),
          args->program,
          args->program,
          args->program,
          args->program);
}

int main(int argc, char* const argv[argc + 1]) {
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
    } else if (strcmp(command, "count_words") == 0) {
      ok = count_words(&args);
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
