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
       "  %s check [-v] path"
       "\n"
       "  %s count-nodes path"
       "\n"
       "  %s get json_path path"
       "\n"),
      args->argv[0],
      args->argv[0],
      args->argv[0]
  );
}

bool sl_check(struct sl_context ctx[static 1], const struct sl_args args[const static 1]) {
  {
    const int args_count     = sl_args_count_positional(args) - 1;
    const int expected_count = 1;
    if (args_count != expected_count) {
      SL_ERROR(ctx, "check takes %d argument, not %d", expected_count, args_count);
      return false;
    }
  }

  const bool verbose    = sl_args_parse_flag(args, "-v");
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
  if (data.size == 0) {
    SL_ERROR(ctx, "%s is empty", path);
    goto done;
  }

  ok = sl_json_is_valid(ctx, data.size, (const char*)data.data);
  if (!ok && !verbose) {
    sl_error_clear(&ctx->errors);
  }

done:
  sl_span_destroy(&data);
  sl_span_destroy(&buffer);
  return ok;
}

bool sl_count_nodes(struct sl_context ctx[static 1], const struct sl_args args[const static 1]) {
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
  if (data.size == 0) {
    SL_ERROR(ctx, "%s is empty", path);
    goto done;
  }

  size_t n = sl_json_count_nodes(ctx, data.size, (const char*)data.data);
  if (printf("%zu\n", n) < 0) {
    goto done;
  }

  ok = true;

done:
  sl_span_destroy(&data);
  sl_span_destroy(&buffer);
  return ok;
}

bool sl_get(struct sl_context ctx[static 1], const struct sl_args args[const static 1]) {
  {
    const int args_count     = sl_args_count_positional(args) - 1;
    const int expected_count = 2;
    if (args_count != expected_count) {
      SL_ERROR(ctx, "get takes %d arguments, not %d", expected_count, args_count);
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

  const char* json_path = sl_args_get_positional(args, 1);
  const char* path      = sl_args_get_positional(args, 2);
  data                  = sl_fs_read_file(ctx, path, &buffer);
  if (sl_context_error_occurred(ctx)) {
    SL_ERROR(ctx, "failed reading %s", path);
    goto done;
  }
  if (data.size == 0) {
    SL_ERROR(ctx, "%s is empty", path);
    goto done;
  }

  const char* json         = (const char*)data.data;
  struct sl_json_node node = {0};
  if (!sl_json_find(ctx, data.size, json, strlen(json_path), json_path, &node)) {
    if (!sl_context_error_occurred(ctx)) {
      SL_ERROR(ctx, "'%s' not found in %s", json_path, path);
    }
    goto done;
  }

  if (printf("%.*s\n", (int)node.value_len, json + node.value_begin) < 0) {
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
      ok = sl_check(&ctx, &args);
    } else if (strcmp(command, "count-nodes") == 0) {
      ok = sl_count_nodes(&ctx, &args);
    } else if (strcmp(command, "get") == 0) {
      ok = sl_get(&ctx, &args);
    } else {
      print_usage(&args);
    }
  } else {
    print_usage(&args);
  }
  if (!sl_context_unwind_errors(&ctx, stderr)) {
    ok = false;
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
