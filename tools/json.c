#include <stdio.h>
#include <stdlib.h>

#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/error/error.h>
#include <stufflib/filesystem/filesystem.h>
#include <stufflib/json/json.h>
#include <stufflib/logging/logging.h>
#include <stufflib/macros/macros.h>
#include <stufflib/span/span.h>

void print_usage(const struct sl_args args[const static 1]) {
  fprintf(stderr, "usage: %s --check path\n", args->argv[0]);
}

int main(int argc, char* const argv[argc + 1]) {
  struct sl_context ctx = {0};
  struct sl_args args   = {.argc = argc, .argv = argv};
  struct sl_span buffer = {0};
  struct sl_span data   = {0};
  bool ok               = false;

  if (sl_args_count_positional(&args) != 1) {
    print_usage(&args);
    goto done;
  }

  if (!sl_span_create(&ctx, 4096, &buffer)) {
    SL_ERROR(&ctx, "failed allocating read buffer");
    goto done;
  }

  if (sl_args_parse_flag(&args, "--check")) {
    const char* path = sl_args_get_positional(&args, 0);

    data = sl_fs_read_file(&ctx, path, &buffer);
    if (sl_context_error_occurred(&ctx)) {
      SL_ERROR(&ctx, "failed reading %s", path);
      goto done;
    }

    ok = sl_json_parse(data.size, (const char*)data.data);
  } else {
    ok = true;
  }

done:
  sl_span_destroy(&buffer);
  sl_span_destroy(&data);
  if (!sl_context_unwind_errors(&ctx, stderr)) {
    ok = false;
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
