#ifndef _STUFFLIB_TEXT_H_INCLUDED
#define _STUFFLIB_TEXT_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_macros.h"

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
      return 0;
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

int stufflib_text_append_str(stufflib_text text[const static 1],
                             const char str[const static 1],
                             const size_t length) {
  {
    char* tmp = realloc(text->str, text->length + length + 1);
    if (!tmp) {
      STUFFLIB_PRINT_ERROR("failed reallocating text->str during append str");
      return 0;
    }
    text->str = tmp;
  }
  if (length) {
    memcpy(text->str + text->length, str, length);
    text->length += length;
  }
  text->str[text->length] = 0;
  return 1;
}

int stufflib_text_fprint(FILE stream[const static 1],
                         const stufflib_text head[const static 1],
                         const char separator[const static 1]) {
  int ret = 0;
  for (const stufflib_text* text = head; text && !(ret < 0);
       text = text->next) {
    ret = fprintf(stream, "%s%s", text->str, separator);
  }
  return ret;
}

char* stufflib_text_to_str(const stufflib_text head[const static 1]) {
  char* str = calloc(stufflib_text_size(head) + 1, 1);
  if (!str) {
    STUFFLIB_PRINT_ERROR("failed allocating str");
    return 0;
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
  stufflib_text* root = 0;
  char* full_str = stufflib_text_to_str(head);
  if (!full_str) {
    STUFFLIB_PRINT_ERROR("failed flattening stufflib_text to single str");
    goto error;
  }

  stufflib_text* prev = root;
  for (const char* chunk = full_str; chunk;) {
    stufflib_text* text = calloc(1, sizeof(stufflib_text));
    if (!text) {
      STUFFLIB_PRINT_ERROR("failed allocating next stufflib_text during split");
      goto error;
    }
    if (prev) {
      prev->next = text;
    }
    const char* chunk_end = strstr(chunk, separator);
    const size_t chunk_len = chunk_end ? chunk_end - chunk : strlen(chunk);
    if (!stufflib_text_append_str(text, chunk, chunk_len)) {
      STUFFLIB_PRINT_ERROR("failed appending str during split");
      goto error;
    }
    chunk = chunk_end ? chunk_end + strlen(separator) : chunk_end;
    if (!root) {
      root = text;
    }
    prev = text;
  }
  free(full_str);

  return root;

error:
  if (full_str) {
    free(full_str);
  }
  if (root) {
    stufflib_text_destroy(root);
  }
  return 0;
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
  return 0;
}

stufflib_text* stufflib_text_from_file(const char fname[const static 1]) {
  FILE* fp = 0;
  stufflib_text* text = 0;

  fp = fopen(fname, "r");
  if (!fp) {
    STUFFLIB_PRINT_ERROR("cannot open %s", fname);
    goto error;
  }
  text = calloc(1, sizeof(stufflib_text));
  if (!text) {
    STUFFLIB_PRINT_ERROR("failed allocating stufflib_text");
    goto error;
  }

  while (!feof(fp)) {
    char buffer[STUFFLIB_TEXT_BUFFER_SIZE] = {0};
    const size_t buffer_size = STUFFLIB_TEXT_BUFFER_SIZE;

    const size_t num_read = fread(buffer, 1, buffer_size, fp);
    if (num_read == 0 && feof(fp)) {
      break;
    }
    if (num_read == 0 || ferror(fp)) {
      STUFFLIB_PRINT_ERROR("failed reading %zu bytes from %s",
                           buffer_size,
                           fname);
      goto error;
    }
    if (!stufflib_text_append_str(text, buffer, num_read)) {
      STUFFLIB_PRINT_ERROR("failed appending str during init from file");
      goto error;
    }
  }
  fclose(fp);

  return text;

error:
  if (fp) {
    fclose(fp);
  }
  if (text) {
    stufflib_text_destroy(text);
  }
  return 0;
}

#endif  // _STUFFLIB_TEXT_H_INCLUDED
