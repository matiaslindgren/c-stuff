#ifndef SL_ERROR_H_INCLUDED
#define SL_ERROR_H_INCLUDED

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SL_ERROR_STACK_DEPTH 16
#define SL_ERROR_MSG_LEN 1'024

struct sl_error_msg {
  const char* file;
  int line;
  char msg[SL_ERROR_MSG_LEN];
};

struct sl_error_stack {
  struct sl_error_msg entries[SL_ERROR_STACK_DEPTH];
  size_t top;
};

void sl_error_push(
    struct sl_error_stack s[static 1],
    const char file[static 1],
    int line,
    const char msg[static 1]
);
bool sl_error_pop(struct sl_error_stack s[static 1], struct sl_error_msg out[static 1]);
const struct sl_error_msg* sl_error_peek(struct sl_error_stack s[static 1]);
void sl_error_clear(struct sl_error_stack s[static 1]);
size_t sl_error_depth(struct sl_error_stack s[static 1]);

static inline bool sl_error_occurred(struct sl_error_stack s[static 1]) {
  return sl_error_depth(s) > 0;
}

#endif  // SL_ERROR_H_INCLUDED
