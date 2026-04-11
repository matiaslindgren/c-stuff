#include <stdio.h>
#include <stufflib/context/context.h>
#include <stufflib/error/error.h>

bool sl_context_unwind_errors(struct sl_context ctx[static 1], FILE stream[static 1]) {
  bool ok = true;
  for (struct sl_error_msg e = {0}; sl_error_pop(&ctx->errors, &e);) {
    if (fprintf(
            stream,
            ("{\"level\":\"error\""
             ",\"file\":\"%s\""
             ",\"line\":%d"
             ",\"msg\":\"%s\"}\n"),
            e.file,
            e.line,
            e.msg
        )
        < 0) {
      ok = false;
    }
  }
  sl_error_clear(&ctx->errors);
  return ok;
}
