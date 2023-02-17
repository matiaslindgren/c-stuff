#ifndef _STUFFLIB_TEXT_H_INCLUDED
#define _STUFFLIB_TEXT_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_io.h"
#include "stufflib_macros.h"
#include "stufflib_str.h"

#define STUFFLIB_TEXT_BUFFER_SIZE 8192

typedef struct stufflib_text stufflib_text;
struct stufflib_text {
  stufflib_text* next;
  size_t length;
  char* str;
};

void stufflib_text_destroy(stufflib_text head[const static 1]) {
  for (stufflib_text* text = head; text;) {
    stufflib_text* next = text->next;
    free(text->str);
    free(text);
    text = next;
  }
}

stufflib_text* stufflib_text_split_at(stufflib_text head[const static 1],
                                      const size_t index) {
  stufflib_text* text = head;
  for (size_t i = 0; i < index; ++i) {
    if (!text) {
      STUFFLIB_PRINT_ERROR("index %zu out of bounds", index);
      return nullptr;
    }
    text = text->next;
  }
  stufflib_text* rhs = text->next;
  text->next = 0;
  return rhs;
}

stufflib_text* stufflib_text_concat(stufflib_text lhs[static 1],
                                    stufflib_text rhs[const static 1]) {
  stufflib_text* root = lhs;
  while (lhs->next) {
    lhs = lhs->next;
  }
  lhs->next = rhs;
  return root;
}

size_t stufflib_text_size(const stufflib_text head[const static 1]) {
  size_t size = 0;
  for (const stufflib_text* text = head; text; text = text->next) {
    size += text->length;
  }
  return size;
}

size_t stufflib_text_count(const stufflib_text head[const static 1]) {
  size_t count = 0;
  for (const stufflib_text* text = head; text; text = text->next) {
    ++count;
  }
  return count;
}

bool stufflib_text_append_str(stufflib_text text[const static 1],
                              const char str[const static 1],
                              const size_t length) {
  {
    char* tmp = realloc(text->str, text->length + length + 1);
    if (!tmp) {
      STUFFLIB_PRINT_ERROR("failed reallocating text->str during append str");
      return false;
    }
    text->str = tmp;
  }
  if (length) {
    memcpy(text->str + text->length, str, length);
    text->length += length;
  }
  text->str[text->length] = 0;
  return true;
}

int stufflib_text_fprint(FILE stream[const static 1],
                         const stufflib_text head[const static 1],
                         const char separator[const static 1]) {
  int ret = 0;
  for (const stufflib_text* text = head; text && !(ret < 0);
       text = text->next) {
    ret = fprintf(stream, "%s%s", text->str, separator);
  }
  if (!(ret < 0)) {
    fflush(stream);
  }
  return ret;
}

char* stufflib_text_to_str(const stufflib_text head[const static 1]) {
  char* str = calloc(stufflib_text_size(head) + 1, 1);
  if (!str) {
    STUFFLIB_PRINT_ERROR("failed allocating str");
    return nullptr;
  }
  char* pos = str;
  for (const stufflib_text* text = head; text; text = text->next) {
    memcpy(pos, text->str, text->length);
    pos += text->length;
  }
  return str;
}

stufflib_text* stufflib_text_split(const stufflib_text head[const static 1],
                                   const char separator[const static 1]) {
  stufflib_text* root = nullptr;
  char** chunks = nullptr;
  char* full_str = nullptr;

  full_str = stufflib_text_to_str(head);
  if (!full_str) {
    STUFFLIB_PRINT_ERROR("failed flattening stufflib_text to single str");
    goto error;
  }

  chunks = stufflib_str_split(full_str, separator);
  if (!chunks) {
    STUFFLIB_PRINT_ERROR("failed splitting string");
    goto error;
  }

  stufflib_text* prev = root;
  for (char** chunk = chunks; *chunk; ++chunk) {
    stufflib_text* text = calloc(1, sizeof(stufflib_text));
    if (!text) {
      STUFFLIB_PRINT_ERROR("failed allocating stufflib_text during split");
      goto error;
    }
    if (prev) {
      prev->next = text;
    }
    if (!stufflib_text_append_str(text, *chunk, strlen(*chunk))) {
      STUFFLIB_PRINT_ERROR("failed appending str during split");
      goto error;
    }
    if (!root) {
      root = text;
    }
    prev = text;
  }
  free(full_str);
  stufflib_str_chunks_destroy(chunks);

  return root;

error:
  if (full_str) {
    free(full_str);
  }
  if (chunks) {
    stufflib_str_chunks_destroy(chunks);
  }
  if (root) {
    stufflib_text_destroy(root);
  }
  return nullptr;
}

stufflib_text* stufflib_text_from_str(const char str[const static 1]) {
  stufflib_text* text = calloc(1, sizeof(stufflib_text));
  if (!text) {
    STUFFLIB_PRINT_ERROR("failed allocating stufflib_text");
    goto error;
  }
  if (!stufflib_text_append_str(text, str, strlen(str))) {
    STUFFLIB_PRINT_ERROR("failed appending str to stufflib_text");
    goto error;
  }
  return text;

error:
  if (text) {
    stufflib_text_destroy(text);
  }
  return nullptr;
}

stufflib_text* stufflib_text_replace(const stufflib_text head[const static 1],
                                     const char old_str[const static 1],
                                     const char new_str[const static 1]) {
  stufflib_text* root = nullptr;

  stufflib_text* prev = nullptr;
  for (const stufflib_text* src = head; src; src = src->next) {
    stufflib_text* dst = calloc(1, sizeof(stufflib_text));
    if (!dst) {
      STUFFLIB_PRINT_ERROR("failed allocating stufflib_text during replace");
      goto error;
    }
    if (!root) {
      root = dst;
    }
    if (prev) {
      prev->next = dst;
    }

    for (const char* src_str = src->str; src_str;) {
      const char* match = strstr(src_str, old_str);
      if (match) {
        if (!stufflib_text_append_str(dst, src_str, match - src_str)) {
          STUFFLIB_PRINT_ERROR("failed appending prefix during replace");
          goto error;
        }
        if (!stufflib_text_append_str(dst, new_str, strlen(new_str))) {
          STUFFLIB_PRINT_ERROR("failed appending new_str during replace");
          goto error;
        }
        src_str = match + strlen(old_str);
      } else {
        if (!stufflib_text_append_str(dst, src_str, strlen(src_str))) {
          STUFFLIB_PRINT_ERROR("failed appending src->ste during replace");
          goto error;
        }
        src_str = 0;
      }
    }

    prev = dst;
  }

  return root;

error:
  stufflib_text_destroy(root);
  return nullptr;
}

size_t stufflib_text_count_matches(const stufflib_text head[const static 1],
                                   const char pattern[const static 1]) {
  size_t count = 0;
  for (const stufflib_text* src = head; src; src = src->next) {
    for (const char* src_str = src->str; src_str;) {
      const char* match = strstr(src_str, pattern);
      if (match) {
        src_str = match + strlen(pattern);
        ++count;
      } else {
        src_str = 0;
      }
    }
  }
  return count;
}

stufflib_text* stufflib_text_from_file(const char fname[const static 1]) {
  stufflib_text* text = calloc(1, sizeof(stufflib_text));
  char* const file_contents = stufflib_io_slurp_file(fname);
  if (!file_contents) {
    STUFFLIB_PRINT_ERROR("failed reading contents of %s", fname);
    goto error;
  }
  if (!text) {
    STUFFLIB_PRINT_ERROR("failed allocating stufflib_text");
    goto error;
  }
  if (!stufflib_text_append_str(text, file_contents, strlen(file_contents))) {
    STUFFLIB_PRINT_ERROR("failed appending str during init from file");
    goto error;
  }
  free(file_contents);
  return text;

error:
  if (file_contents) {
    free(file_contents);
  }
  if (text) {
    stufflib_text_destroy(text);
  }
  return nullptr;
}

#endif  // _STUFFLIB_TEXT_H_INCLUDED
