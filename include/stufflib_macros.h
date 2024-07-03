#ifndef SL_MACROS_H_INCLUDED
#define SL_MACROS_H_INCLUDED

#define SL_MIN(x, y) ((x) < (y) ? (x) : (y))
#define SL_MAX(x, y) ((x) < (y) ? (y) : (x))

#define SL_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))
#define SL_STR_EQ(a, b) (strcmp((a), (b)) == 0)

#define SL_ASSERT_BINOP(binop, type, fmt, lhs, rhs, ...)                     \
  do {                                                                       \
    const type sl_assert_lhs = (type)(lhs);                                  \
    const type sl_assert_rhs = (type)(rhs);                                  \
    if (!(sl_assert_lhs binop sl_assert_rhs)) {                              \
      fprintf(                                                               \
          stderr,                                                            \
          ("assertion failed: (" #type ")(" #lhs ") " #binop " (" #type      \
           ")(" #rhs ") "                                                    \
           "\n  lhs: " #lhs " = " fmt "\n  rhs: " #rhs " = " fmt             \
           "\n  function: %s\n  file: %s\n  line: %d\n  msg: \"" __VA_ARGS__ \
           "\"\n"),                                                          \
          sl_assert_lhs,                                                     \
          sl_assert_rhs,                                                     \
          __func__,                                                          \
          __FILE__,                                                          \
          __LINE__);                                                         \
      abort();                                                               \
    }                                                                        \
  } while (false)

#define SL_ASSERT_STRCMP(expect, type, fmt, lhs, rhs, ...)                     \
  do {                                                                         \
    type sl_assert_lhs = (type)(lhs);                                          \
    type sl_assert_rhs = (type)(rhs);                                          \
    if (!(strcmp(sl_assert_lhs, sl_assert_rhs) == expect)) {                   \
      fprintf(                                                                 \
          stderr,                                                              \
          ("assertion failed: strcmp((" #type ")(" #lhs "), (" #type ")(" #rhs \
           ")) == " #expect "\n  lhs: " #lhs " = \"" fmt "\"\n  rhs: " #rhs    \
           " = \"" fmt "\"\n  function: %s\n  file: %s\n  line: %d\n  msg: "   \
           "\"" __VA_ARGS__ "\"\n"),                                           \
          sl_assert_lhs,                                                       \
          sl_assert_rhs,                                                       \
          __func__,                                                            \
          __FILE__,                                                            \
          __LINE__);                                                           \
      abort();                                                                 \
    }                                                                          \
  } while (false)

#define SL_ASSERT_TRUE(expr, ...) \
  SL_ASSERT_BINOP(==, bool, "%d", (expr), true, __VA_ARGS__)
#define SL_ASSERT_EQ_CHAR(...) SL_ASSERT_BINOP(==, char, "%c", __VA_ARGS__)
#define SL_ASSERT_EQ_LL(...) SL_ASSERT_BINOP(==, long long, "%lld", __VA_ARGS__)
#define SL_ASSERT_EQ_PTR(...) SL_ASSERT_BINOP(==, void*, "%p", __VA_ARGS__)
#define SL_ASSERT_EQ_STR(...) \
  SL_ASSERT_STRCMP(0, const char* const, "%s", __VA_ARGS__)

#define SL_LOG(level, ...)                             \
  do {                                                 \
    const char* const sl_log_fname = __FILE__;         \
    const unsigned long sl_log_lineno = __LINE__;      \
    fprintf(stderr, "{\"level\":\"%s\"", level);       \
    fprintf(stderr, ",\"file\":\"%s\"", sl_log_fname); \
    fprintf(stderr, ",\"line\":%ld", sl_log_lineno);   \
    fprintf(stderr, ",\"msg\":\"");                    \
    fprintf(stderr, __VA_ARGS__);                      \
    fprintf(stderr, "\"}\n");                          \
  } while (false)

#define SL_LOG_INFO(...) SL_LOG("info", __VA_ARGS__)
#define SL_LOG_ERROR(...) SL_LOG("error", __VA_ARGS__)

#define SL_TEST_MAIN(...)                                    \
  int main(int argc, char* const argv[argc + 1]) {           \
    struct sl_args args = {.argc = argc, .argv = argv};      \
    const bool verbose = sl_args_parse_flag(&args, "-v");    \
    bool (*tests[])(const bool) = {__VA_ARGS__};             \
    char test_names_str[] = #__VA_ARGS__;                    \
    bool ok = true;                                          \
    const char* test_name = "";                              \
    for (size_t t = 0; ok && t < SL_ARRAY_LEN(tests); ++t) { \
      test_name = strtok(t ? 0 : test_names_str, ", ");      \
      assert(test_name);                                     \
      if (verbose) {                                         \
        printf("%s\n", test_name);                           \
      }                                                      \
      ok = tests[t](verbose);                                \
      if (!ok) {                                             \
        SL_LOG_ERROR("test %s (%zu) failed", test_name, t);  \
      }                                                      \
    }                                                        \
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;                 \
  }

#define SL_REPEAT_1(x) x
#define SL_REPEAT_2(x) SL_REPEAT_1(x) x
#define SL_REPEAT_3(x) SL_REPEAT_2(x) x
#define SL_REPEAT_4(x) SL_REPEAT_3(x) x
#define SL_REPEAT_5(x) SL_REPEAT_4(x) x
#define SL_REPEAT_6(x) SL_REPEAT_5(x) x
#define SL_REPEAT_7(x) SL_REPEAT_6(x) x
#define SL_REPEAT_8(x) SL_REPEAT_7(x) x
#define SL_REPEAT_9(x) SL_REPEAT_8(x) x
#define SL_REPEAT_10(x) SL_REPEAT_9(x) x
#define SL_REPEAT_11(x) SL_REPEAT_10(x) x
#define SL_REPEAT_12(x) SL_REPEAT_11(x) x
#define SL_REPEAT_13(x) SL_REPEAT_12(x) x
#define SL_REPEAT_14(x) SL_REPEAT_13(x) x
#define SL_REPEAT_15(x) SL_REPEAT_14(x) x
#define SL_REPEAT_16(x) SL_REPEAT_15(x) x
#define SL_REPEAT_17(x) SL_REPEAT_16(x) x
#define SL_REPEAT_18(x) SL_REPEAT_17(x) x
#define SL_REPEAT_19(x) SL_REPEAT_18(x) x
#define SL_REPEAT_20(x) SL_REPEAT_19(x) x
#define SL_REPEAT_21(x) SL_REPEAT_20(x) x
#define SL_REPEAT_22(x) SL_REPEAT_21(x) x
#define SL_REPEAT_23(x) SL_REPEAT_22(x) x
#define SL_REPEAT_24(x) SL_REPEAT_23(x) x
#define SL_REPEAT_25(x) SL_REPEAT_24(x) x
#define SL_REPEAT_26(x) SL_REPEAT_25(x) x
#define SL_REPEAT_27(x) SL_REPEAT_26(x) x
#define SL_REPEAT_28(x) SL_REPEAT_27(x) x
#define SL_REPEAT_29(x) SL_REPEAT_28(x) x
#define SL_REPEAT_30(x) SL_REPEAT_29(x) x
#define SL_REPEAT_31(x) SL_REPEAT_30(x) x
#define SL_REPEAT_32(x) SL_REPEAT_31(x) x
#define SL_REPEAT_33(x) SL_REPEAT_32(x) x
#define SL_REPEAT_34(x) SL_REPEAT_33(x) x
#define SL_REPEAT_35(x) SL_REPEAT_34(x) x
#define SL_REPEAT_36(x) SL_REPEAT_35(x) x
#define SL_REPEAT_37(x) SL_REPEAT_36(x) x
#define SL_REPEAT_38(x) SL_REPEAT_37(x) x
#define SL_REPEAT_39(x) SL_REPEAT_38(x) x
#define SL_REPEAT_40(x) SL_REPEAT_39(x) x
#define SL_REPEAT_41(x) SL_REPEAT_40(x) x
#define SL_REPEAT_42(x) SL_REPEAT_41(x) x
#define SL_REPEAT_43(x) SL_REPEAT_42(x) x
#define SL_REPEAT_44(x) SL_REPEAT_43(x) x
#define SL_REPEAT_45(x) SL_REPEAT_44(x) x
#define SL_REPEAT_46(x) SL_REPEAT_45(x) x
#define SL_REPEAT_47(x) SL_REPEAT_46(x) x
#define SL_REPEAT_48(x) SL_REPEAT_47(x) x
#define SL_REPEAT_49(x) SL_REPEAT_48(x) x
#define SL_REPEAT_50(x) SL_REPEAT_49(x) x
#define SL_REPEAT_51(x) SL_REPEAT_50(x) x
#define SL_REPEAT_52(x) SL_REPEAT_51(x) x
#define SL_REPEAT_53(x) SL_REPEAT_52(x) x
#define SL_REPEAT_54(x) SL_REPEAT_53(x) x
#define SL_REPEAT_55(x) SL_REPEAT_54(x) x
#define SL_REPEAT_56(x) SL_REPEAT_55(x) x
#define SL_REPEAT_57(x) SL_REPEAT_56(x) x
#define SL_REPEAT_58(x) SL_REPEAT_57(x) x
#define SL_REPEAT_59(x) SL_REPEAT_58(x) x
#define SL_REPEAT_60(x) SL_REPEAT_59(x) x
#define SL_REPEAT_61(x) SL_REPEAT_60(x) x
#define SL_REPEAT_62(x) SL_REPEAT_61(x) x
#define SL_REPEAT_63(x) SL_REPEAT_62(x) x
#define SL_REPEAT_64(x) SL_REPEAT_63(x) x

#endif  // SL_MACROS_H_INCLUDED
