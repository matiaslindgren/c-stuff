#ifndef _STUFFLIB_TEXT_H_INCLUDED
#define _STUFFLIB_TEXT_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_macros.h"

#define _STUFFLIB_TEXT_LINEBUF_SIZE 4096

struct stufflib_text;

struct stufflib_text {
  struct stufflib_text* next;
  size_t lineno;
  size_t length;
  char* data;
};

struct stufflib_text* stufflib_text_init() {
  return (struct stufflib_text*)(calloc(1, sizeof(struct stufflib_text)));
}

void stufflib_text_destroy(struct stufflib_text text[static 1]) {
  while (text) {
    struct stufflib_text* next = text->next;
    if (text->data) {
      free(text->data);
    }
    free(text);
    text = next;
  }
}

void _stufflib_text_renumber_lines(struct stufflib_text text[static 1]) {
  size_t lineno = 0;
  while (text) {
    text->lineno = ++lineno;
    text = text->next;
  }
}

struct stufflib_text* stufflib_text_split_after(
    struct stufflib_text text[static 1],
    const size_t lineno) {
  while (text && text->lineno < lineno) {
    text = text->next;
  }
  if (!text || text->lineno != lineno) {
    return 0;
  }
  struct stufflib_text* rhs = text->next;
  _stufflib_text_renumber_lines(rhs);
  text->next = 0;
  return rhs;
}

struct stufflib_text* stufflib_text_concat(struct stufflib_text lhs[static 1],
                                           struct stufflib_text rhs[static 1]) {
  struct stufflib_text* root = lhs;
  while (lhs->next) {
    lhs = lhs->next;
  }
  lhs->next = rhs;
  _stufflib_text_renumber_lines(root);
  return root;
}

size_t stufflib_text_line_count(struct stufflib_text text[static 1]) {
  size_t lines = 0;
  while (text) {
    lines = text->lineno;
    text = text->next;
  }
  return lines;
}

int stufflib_text_fprint(FILE stream[const static 1],
                         struct stufflib_text text[static 1]) {
  int ret = 0;
  while (ret >= 0 && text) {
    ret = fprintf(stream, "%s\n", text->data);
    text = text->next;
  }
  return ret;
}

int _stufflib_text_number_width(size_t x) {
  int w = 0;
  while (x) {
    ++w;
    x /= 10;
  }
  return w;
}

int stufflib_text_pretty_fprint(FILE stream[const static 1],
                                struct stufflib_text text[static 1]) {
  int ret = 0;
  const int pad = _stufflib_text_number_width(stufflib_text_line_count(text));
  while (ret >= 0 && text) {
    ret = fprintf(stream, "%*zu: %s\n", pad, text->lineno, text->data);
    text = text->next;
  }
  return ret;
}

struct stufflib_text* stufflib_text_from_str(const char line[static 1]) {
  char* text_buf = 0;
  struct stufflib_text* text = 0;

  const size_t line_len = strcspn(line, "\r\n");
  if (line_len >= _STUFFLIB_TEXT_LINEBUF_SIZE) {
    STUFFLIB_PRINT_ERROR("line too long %zu", line_len);
    return 0;
  }
  text_buf = (char*)(calloc(line_len + 1, sizeof(char)));
  if (!text_buf) {
    STUFFLIB_PRINT_ERROR("failed allocating memory");
    goto error;
  }
  if (!strncpy(text_buf, line, line_len)) {
    STUFFLIB_PRINT_ERROR("strcpy failed");
    goto error;
  }
  text = stufflib_text_init();
  if (!text) {
    fprintf(stderr,
            "error: failed allocating memory for stufflib_text struct\n");
    goto error;
  }
  text->data = text_buf;
  text->length = line_len;
  text->lineno = 1;
  return text;

error:
  if (text_buf) {
    free(text_buf);
  }
  if (text) {
    stufflib_text_destroy(text);
  }
  return 0;
}

struct stufflib_text* stufflib_text_from_file(const char fname[static 1]) {
  struct stufflib_text* root = 0;

  FILE* fp = fopen(fname, "r");
  if (!fp) {
    STUFFLIB_PRINT_ERROR("error: cannot open %s", fname);
    goto error;
  }

  for (struct stufflib_text* head = 0; !feof(fp);) {
    const size_t max_line_len = _STUFFLIB_TEXT_LINEBUF_SIZE;
    char linebuf[_STUFFLIB_TEXT_LINEBUF_SIZE] = {0};

    const char* line = fgets(linebuf, max_line_len, fp);
    if (!line && feof(fp)) {
      break;
    }
    if (!line || ferror(fp)) {
      STUFFLIB_PRINT_ERROR("error while reading %s", fname);
      goto error;
    }
    if (linebuf[max_line_len - 1] != 0) {
      STUFFLIB_PRINT_ERROR("too long line found in %s", fname);
      goto error;
    }

    struct stufflib_text* prev = head;
    head = stufflib_text_from_str(line);
    if (!head) {
      goto error;
    }
    if (!root) {
      root = head;
    }
    if (prev) {
      head->lineno = prev->lineno + 1;
      prev->next = head;
    }
  }
  fclose(fp);

  return root;

error:
  if (fp) {
    fclose(fp);
  }
  if (root) {
    stufflib_text_destroy(root);
  }
  return 0;
}

#undef _STUFFLIB_TEXT_LINEBUF_SIZE

#endif  // _STUFFLIB_TEXT_H_INCLUDED
