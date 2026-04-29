#include <stddef.h>
#include <stdint.h>

#include <stufflib/context/context.h>
#include <stufflib/json/json.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  struct sl_context ctx = {0};
  (void)sl_json_is_valid(&ctx, size, (const char*)data);
  for (struct sl_error_msg e = {0}; sl_error_pop(&ctx.errors, &e);) {
  }
  return 0;
}
