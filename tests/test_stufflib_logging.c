#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stufflib/json/json.h>
#include <stufflib/logging/logging.h>
#include <stufflib/testing/testing.h>

SL_TEST(test_log_json_format) {
  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  SL_ASSERT_TRUE(mem != nullptr);

  sl_logging_writef(mem, "info", "myfile.c", 42, "hello %s", "world");
  fclose(mem);

  struct sl_json_node node = {0};
  char val[SL_LOGGING_MAX_LENGTH];
  long long line = 0;

  SL_ASSERT_TRUE(sl_json_find(ctx, size, buf, strlen(".level"), ".level", &node));
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, buf, sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "info");

  SL_ASSERT_TRUE(sl_json_find(ctx, size, buf, strlen(".file"), ".file", &node));
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, buf, sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "myfile.c");

  SL_ASSERT_TRUE(sl_json_find(ctx, size, buf, strlen(".line"), ".line", &node));
  SL_ASSERT_TRUE(sl_json_get_int(&node, buf, &line));
  SL_ASSERT_EQ_LL(line, 42);

  SL_ASSERT_TRUE(sl_json_find(ctx, size, buf, strlen(".msg"), ".msg", &node));
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, buf, sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "hello world");

  free(buf);
  return true;
}

SL_TEST(test_log_escape_quote) {
  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  SL_ASSERT_TRUE(mem != nullptr);
  sl_logging_writef(mem, "info", "f.c", 1, "%c", '"');
  fclose(mem);

  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, size, buf, strlen(".msg"), ".msg", &node));
  char msg[64];
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, buf, sizeof(msg), msg));
  SL_ASSERT_EQ_STR(msg, "\"");
  free(buf);
  return true;
}

SL_TEST(test_log_escape_backslash) {
  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  SL_ASSERT_TRUE(mem != nullptr);
  sl_logging_writef(mem, "info", "f.c", 1, "%c", '\\');
  fclose(mem);

  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, size, buf, strlen(".msg"), ".msg", &node));
  char msg[64];
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, buf, sizeof(msg), msg));
  SL_ASSERT_EQ_STR(msg, "\\");
  free(buf);
  return true;
}

SL_TEST(test_log_escape_newline) {
  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  SL_ASSERT_TRUE(mem != nullptr);
  sl_logging_writef(mem, "info", "f.c", 1, "%c", '\n');
  fclose(mem);

  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, size, buf, strlen(".msg"), ".msg", &node));
  char msg[64];
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, buf, sizeof(msg), msg));
  SL_ASSERT_EQ_STR(msg, "\n");
  free(buf);
  return true;
}

SL_TEST(test_log_escape_carriage_return) {
  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  SL_ASSERT_TRUE(mem != nullptr);
  sl_logging_writef(mem, "info", "f.c", 1, "%c", '\r');
  fclose(mem);

  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, size, buf, strlen(".msg"), ".msg", &node));
  char msg[64];
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, buf, sizeof(msg), msg));
  SL_ASSERT_EQ_STR(msg, "\r");
  free(buf);
  return true;
}

SL_TEST(test_log_escape_tab) {
  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  SL_ASSERT_TRUE(mem != nullptr);
  sl_logging_writef(mem, "info", "f.c", 1, "%c", '\t');
  fclose(mem);

  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, size, buf, strlen(".msg"), ".msg", &node));
  char msg[64];
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, buf, sizeof(msg), msg));
  SL_ASSERT_EQ_STR(msg, "\t");
  free(buf);
  return true;
}

SL_TEST(test_log_escape_backspace) {
  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  SL_ASSERT_TRUE(mem != nullptr);
  sl_logging_writef(mem, "info", "f.c", 1, "%c", '\b');
  fclose(mem);

  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, size, buf, strlen(".msg"), ".msg", &node));
  char msg[64];
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, buf, sizeof(msg), msg));
  SL_ASSERT_EQ_STR(msg, "\b");
  free(buf);
  return true;
}

SL_TEST(test_log_escape_form_feed) {
  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  SL_ASSERT_TRUE(mem != nullptr);
  sl_logging_writef(mem, "info", "f.c", 1, "%c", '\f');
  fclose(mem);

  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, size, buf, strlen(".msg"), ".msg", &node));
  char msg[64];
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, buf, sizeof(msg), msg));
  SL_ASSERT_EQ_STR(msg, "\f");
  free(buf);
  return true;
}

SL_TEST(test_log_escape_control) {
  (void)ctx;
  const unsigned char controls[] = {0x01, 0x07, 0x0b, 0x0e, 0x1f};
  for (size_t i = 0; i < 5; ++i) {
    char* buf   = nullptr;
    size_t size = 0;
    FILE* mem   = open_memstream(&buf, &size);
    SL_ASSERT_TRUE(mem != nullptr);
    sl_logging_writef(mem, "info", "f.c", 1, "%c", (char)controls[i]);
    fclose(mem);

    struct sl_json_node node = {0};
    SL_ASSERT_TRUE(sl_json_find(ctx, size, buf, strlen(".msg"), ".msg", &node));
    char msg[64];
    SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, buf, sizeof(msg), msg));
    SL_ASSERT_EQ_LL((unsigned char)msg[0], controls[i]);
    SL_ASSERT_EQ_LL(msg[1], '\0');
    free(buf);
  }
  return true;
}

SL_TEST(test_log_level_none) {
  (void)ctx;
  int saved_level          = sl_logging_level_runtime;
  sl_logging_level_runtime = SL_LOGGING_LEVEL_none;

  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  SL_ASSERT_TRUE(mem != nullptr);

  SL_LOGF_TRACE(mem, "trace msg");
  SL_LOGF_INFO(mem, "info msg");
  SL_LOGF_ERROR(mem, "error msg");
  fclose(mem);

  SL_ASSERT_EQ_LL(size, 0);

  free(buf);
  sl_logging_level_runtime = saved_level;
  return true;
}

SL_TEST(test_log_level_trace) {
  (void)ctx;
  int saved_level          = sl_logging_level_runtime;
  sl_logging_level_runtime = SL_LOGGING_LEVEL_trace;

  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  SL_ASSERT_TRUE(mem != nullptr);

  SL_LOGF_TRACE(mem, "trace msg");
  SL_LOGF_INFO(mem, "info msg");
  SL_LOGF_ERROR(mem, "error msg");
  fclose(mem);

  SL_ASSERT_TRUE(strstr(buf, "\"level\":\"trace\"") != nullptr);
  SL_ASSERT_TRUE(strstr(buf, "\"level\":\"info\"") != nullptr);
  SL_ASSERT_TRUE(strstr(buf, "\"level\":\"error\"") != nullptr);

  free(buf);
  sl_logging_level_runtime = saved_level;
  return true;
}

SL_TEST(test_log_level_error) {
  (void)ctx;
  int saved_level          = sl_logging_level_runtime;
  sl_logging_level_runtime = SL_LOGGING_LEVEL_error;

  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  SL_ASSERT_TRUE(mem != nullptr);

  SL_LOGF_TRACE(mem, "trace msg");
  SL_LOGF_INFO(mem, "info msg");
  SL_LOGF_ERROR(mem, "error msg");
  fclose(mem);

  SL_ASSERT_TRUE(strstr(buf, "\"level\":\"error\"") != nullptr);
  SL_ASSERT_TRUE(strstr(buf, "\"level\":\"info\"") == nullptr);
  SL_ASSERT_TRUE(strstr(buf, "\"level\":\"trace\"") == nullptr);

  free(buf);
  sl_logging_level_runtime = saved_level;
  return true;
}

SL_TEST(test_log_level_info) {
  (void)ctx;
  int saved_level          = sl_logging_level_runtime;
  sl_logging_level_runtime = SL_LOGGING_LEVEL_info;

  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  SL_ASSERT_TRUE(mem != nullptr);

  SL_LOGF_TRACE(mem, "trace msg");
  SL_LOGF_INFO(mem, "info msg");
  SL_LOGF_ERROR(mem, "error msg");
  fclose(mem);

  SL_ASSERT_TRUE(strstr(buf, "\"level\":\"error\"") != nullptr);
  SL_ASSERT_TRUE(strstr(buf, "\"level\":\"info\"") != nullptr);
  SL_ASSERT_TRUE(strstr(buf, "\"level\":\"trace\"") == nullptr);

  free(buf);
  sl_logging_level_runtime = saved_level;
  return true;
}

SL_TEST_MAIN()
