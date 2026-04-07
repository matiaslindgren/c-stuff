#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stufflib/error/error.h>
#include <stufflib/macros/macros.h>

void sl_error_push(
    struct sl_error_stack s[static 1],
    const char file[static 1],
    int line,
    const char msg[static 1]
) {
  if (s->top >= SL_ERROR_STACK_DEPTH) {
    fprintf(
        stderr,
        "[stufflib_error_stack] FATAL: stack overflow (depth %d) while pushing:"
        " %s:%d %s\n",
        SL_ERROR_STACK_DEPTH,
        file,
        line,
        msg
    );
    abort();
  }
  struct sl_error_msg* e = &s->entries[s->top];
  e->file                = file;
  e->line                = line;
  strncpy(e->msg, msg, sizeof(e->msg));
  e->msg[sizeof(e->msg) - 1] = 0;
  ++(s->top);
}

bool sl_error_pop(struct sl_error_stack s[static 1], struct sl_error_msg out[static 1]) {
  if (s->top) {
    --(s->top);
    *out = s->entries[s->top];
    return true;
  }
  return false;
}

const struct sl_error_msg* sl_error_peek(struct sl_error_stack s[static 1]) {
  if (s->top) {
    return &s->entries[s->top - 1];
  }
  return nullptr;
}

void sl_error_clear(struct sl_error_stack s[static 1]) {
  s->top = 0;
}

size_t sl_error_depth(struct sl_error_stack s[static 1]) {
  return s->top;
}
