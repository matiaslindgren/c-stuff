#ifndef SL_CONTEXT_H_INCLUDED
#define SL_CONTEXT_H_INCLUDED

#include <stdarg.h>
#include <stdio.h>
#include <stufflib/error/error.h>

struct sl_context {
  struct sl_error_stack errors;
  // TODO memory arena/allocators
};

__attribute__((__format__(__printf__, 4, 5))) static inline void sl_context_error_pushf(
    struct sl_context ctx[static 1],
    const char file[static 1],
    int line,
    const char fmt[static 1],
    ...
) {
  char buf[SL_ERROR_MSG_LEN];
  va_list fmt_args;
  va_start(fmt_args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, fmt_args);
  va_end(fmt_args);
  sl_error_push(&ctx->errors, file, line, buf);
}

#define SL_ERROR(ctx, fmt, ...) \
  sl_context_error_pushf((ctx), __FILE__, __LINE__, (fmt)__VA_OPT__(, __VA_ARGS__))

bool sl_context_unwind_errors(struct sl_context ctx[static 1], FILE stream[static 1]);

#endif  // SL_CONTEXT_H_INCLUDED
