#include <stdio.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_hashmap.h"
#include "stufflib_text.h"

bool concat(const stufflib_args args[const static 1]) {
  if (stufflib_args_count_positional(args) < 2) {
    STUFFLIB_PRINT_ERROR("too few arguments to concat");
    return false;
  }

  bool ok = false;
  stufflib_text* text = nullptr;

  for (size_t i = 1;; ++i) {
    const char* path = stufflib_args_get_positional(args, i);
    if (!path) {
      break;
    }
    stufflib_text* next = stufflib_text_from_file(path);
    if (!next) {
      goto done;
    }
    if (text) {
      text = stufflib_text_concat(text, next);
    } else {
      text = next;
    }
  }

  if (!text) {
    goto done;
  }

  if (stufflib_text_fprint(stdout, text, "") < 0) {
    goto done;
  }

  ok = true;

done:
  if (text) {
    stufflib_text_destroy(text);
  }
  return ok;
}

bool count(const stufflib_args args[const static 1]) {
  if (stufflib_args_count_positional(args) != 3) {
    STUFFLIB_PRINT_ERROR("too few arguments to count");
    return false;
  }

  bool ok = false;

  const char* pattern = stufflib_args_get_positional(args, 1);
  const char* path = stufflib_args_get_positional(args, 2);

  stufflib_text* text = stufflib_text_from_file(path);
  if (!text) {
    goto done;
  }
  const size_t n = stufflib_text_count_matches(text, pattern);
  if (printf("%zu\n", n) < 0) {
    goto done;
  }

  ok = true;

done:
  if (text) {
    stufflib_text_destroy(text);
  }
  return ok;
}

bool replace(const stufflib_args args[const static 1]) {
  if (stufflib_args_count_positional(args) != 4) {
    STUFFLIB_PRINT_ERROR("too few arguments to replace");
    return false;
  }

  bool ok = false;

  const char* old_str = stufflib_args_get_positional(args, 1);
  const char* new_str = stufflib_args_get_positional(args, 2);
  const char* path = stufflib_args_get_positional(args, 3);

  stufflib_text* text = stufflib_text_from_file(path);
  if (!text) {
    goto done;
  }
  stufflib_text* replaced = stufflib_text_replace(text, old_str, new_str);
  stufflib_text_destroy(text);
  text = replaced;
  if (!replaced) {
    goto done;
  }

  if (stufflib_text_fprint(stdout, text, "") < 0) {
    goto done;
  }

  ok = true;

done:
  if (text) {
    stufflib_text_destroy(text);
  }
  return ok;
}

bool slicelines(const stufflib_args args[const static 1]) {
  if (stufflib_args_count_positional(args) != 4) {
    STUFFLIB_PRINT_ERROR("too few arguments to slicelines");
    return false;
  }

  bool ok = false;

  const size_t begin = strtoull(stufflib_args_get_positional(args, 1), 0, 10);
  const size_t end = strtoull(stufflib_args_get_positional(args, 2), 0, 10);
  const char* path = stufflib_args_get_positional(args, 3);

  char** lines = stufflib_io_slurp_lines(path, "\n");
  if (!lines) {
    goto done;
  }
  for (size_t pos = 0; lines[pos] && pos < end; ++pos) {
    if (begin <= pos) {
      if (printf("%s\n", lines[pos]) < 0) {
        goto done;
      }
    }
  }

  ok = true;

done:
  if (lines) {
    stufflib_str_chunks_destroy(lines);
  }
  return ok;
}

bool count_words(const stufflib_args args[const static 1]) {
  if (stufflib_args_count_positional(args) != 2) {
    STUFFLIB_PRINT_ERROR("too few arguments to count_words");
    return false;
  }

  bool ok = false;
  char* file_content = nullptr;
  char** words = nullptr;
  stufflib_hashmap word_counts = {0};

  const char* path = stufflib_args_get_positional(args, 1);

  file_content = stufflib_io_slurp_file(path);
  if (!file_content) {
    goto done;
  }
  words = stufflib_str_split_whitespace(file_content);
  if (!words) {
    goto done;
  }
  if (!stufflib_hashmap_init(&word_counts, 1 << 16)) {
    goto done;
  }

  for (size_t i = 0; words[i]; ++i) {
    if (strlen(words[i]) == 0) {
      continue;
    }
    if (!stufflib_hashmap_contains(&word_counts, words[i])) {
      stufflib_hashmap_insert(&word_counts, words[i], 0);
    }
    ++(stufflib_hashmap_get(&word_counts, words[i])->value);
  }

  for (size_t i = 0; i < word_counts.capacity; ++i) {
    const char* key = word_counts.nodes[i].key;
    if (key) {
      const size_t value = word_counts.nodes[i].value;
      printf("%zu %s\n", value, key);
    }
  }

  ok = true;

done:
  if (file_content) {
    free(file_content);
  }
  if (words) {
    stufflib_str_chunks_destroy(words);
  }
  stufflib_hashmap_destroy(&word_counts);
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
           "  %s replace old_str new_str path"
           "\n"
           "  %s slicelines begin end path"
           "\n"
           "  %s count_words path"
           "\n"),
          args->program,
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
    } else if (strcmp(command, "replace") == 0) {
      ok = replace(&args);
    } else if (strcmp(command, "slicelines") == 0) {
      ok = slicelines(&args);
    } else if (strcmp(command, "count_words") == 0) {
      ok = count_words(&args);
    } else {
      STUFFLIB_PRINT_ERROR("unknown command %s", command);
    }
  }
  if (!ok) {
    print_usage(&args);
  }
  stufflib_args_destroy(&args);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
