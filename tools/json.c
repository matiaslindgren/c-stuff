#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/filesystem/filesystem.h>
#include <stufflib/json/json.h>
#include <stufflib/macros/macros.h>
#include <stufflib/span/span.h>

void print_usage(const struct sl_args args[const static 1]) {
  fprintf(
      stderr,
      ("usage:"
       "\n"
       "  %s check path"
       "\n"
       "  %s count-nodes path"
       "\n"),
      args->argv[0],
      args->argv[0]
  );
}

bool check(struct sl_context ctx[static 1], const struct sl_args args[const static 1]) {
  {
    const int args_count     = sl_args_count_positional(args) - 1;
    const int expected_count = 1;
    if (args_count != expected_count) {
      SL_ERROR(ctx, "check takes %d argument, not %d", expected_count, args_count);
      return false;
    }
  }

  struct sl_span buffer = {0};
  struct sl_span data   = {0};
  bool ok               = false;

  if (!sl_span_create(ctx, 4096, &buffer)) {
    SL_ERROR(ctx, "failed allocating read buffer");
    goto done;
  }

  const char* path = sl_args_get_positional(args, 1);
  data             = sl_fs_read_file(ctx, path, &buffer);
  if (sl_context_error_occurred(ctx)) {
    SL_ERROR(ctx, "failed reading %s", path);
    goto done;
  }

  ok = sl_json_is_valid(data.size, (const char*)data.data);

done:
  sl_span_destroy(&data);
  sl_span_destroy(&buffer);
  return ok;
}

bool count_nodes(struct sl_context ctx[static 1], const struct sl_args args[const static 1]) {
  {
    const int args_count     = sl_args_count_positional(args) - 1;
    const int expected_count = 1;
    if (args_count != expected_count) {
      SL_ERROR(ctx, "count-nodes takes %d argument, not %d", expected_count, args_count);
      return false;
    }
  }

  struct sl_span buffer = {0};
  struct sl_span data   = {0};
  bool ok               = false;

  if (!sl_span_create(ctx, 4096, &buffer)) {
    SL_ERROR(ctx, "failed allocating read buffer");
    goto done;
  }

  const char* path = sl_args_get_positional(args, 1);
  data             = sl_fs_read_file(ctx, path, &buffer);
  if (sl_context_error_occurred(ctx)) {
    SL_ERROR(ctx, "failed reading %s", path);
    goto done;
  }

  size_t n = sl_json_count_nodes(data.size, (const char*)data.data);
  if (printf("%zu\n", n) < 0) {
    goto done;
  }

  ok = true;

done:
  sl_span_destroy(&data);
  sl_span_destroy(&buffer);
  return ok;
}

int main(int argc, char* const argv[argc + 1]) {
  struct sl_context ctx = {0};
  struct sl_args args   = {.argc = argc, .argv = argv};
  bool ok               = false;

  char* command = sl_args_get_positional(&args, 0);
  if (command) {
    if (strcmp(command, "check") == 0) {
      ok = check(&ctx, &args);
    } else if (strcmp(command, "count-nodes") == 0) {
      ok = count_nodes(&ctx, &args);
    } else {
      print_usage(&args);
    }
  }
  if (!sl_context_unwind_errors(&ctx, stderr)) {
    ok = false;
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
