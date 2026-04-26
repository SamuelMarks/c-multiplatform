/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_crash_handler_init(void) {
  int res = cmp_crash_handler_init();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  PASS();
}

TEST test_strerror_success(void) {
  {
    const char *err_str;
    cmp_strerror(0, &err_str);
    ASSERT_STR_EQ("Success", err_str);
  }
  {
    const char *err_str;
    cmp_strerror(CMP_ERROR_OOM, &err_str);
    ASSERT_STR_EQ("Out of memory", err_str);
  }
  {
    const char *err_str;
    cmp_strerror(CMP_ERROR_INVALID_ARG, &err_str);
    ASSERT_STR_EQ("Invalid argument provided", err_str);
  }
  {
    const char *err_str;
    cmp_strerror(CMP_ERROR_NOT_FOUND, &err_str);
    ASSERT_STR_EQ("Resource not found", err_str);
  }
  {
    const char *err_str;
    cmp_strerror(CMP_ERROR_BOUNDS, &err_str);
    ASSERT_STR_EQ("Out of bounds access", err_str);
  }
  {
    const char *err_str;
    cmp_strerror(CMP_ERROR_IO, &err_str);
    ASSERT_STR_EQ("I/O error", err_str);
  }
  {
    const char *err_str;
    cmp_strerror(CMP_ERROR_INVALID_STATE, &err_str);
    ASSERT_STR_EQ("Object in invalid state", err_str);
  }
  {
    const char *err_str;
    cmp_strerror(CMP_ERROR_GENERAL, &err_str);
    ASSERT_STR_EQ("General/Unknown error", err_str);
  }
  {
    const char *err_str;
    cmp_strerror(9999, &err_str);
    ASSERT_STR_EQ("Unknown error code", err_str);
  }
  PASS();
}

TEST test_log_debug(void) {
  /* Just call it to ensure no crashes */
  cmp_log_debug("Test log: %d %s", 42, "hello");
  LOG_DEBUG("Macro log test");
  PASS();
}

TEST test_dump_stack_trace(void) {
  /* Call to ensure it doesn't crash on various platforms */
  cmp_dump_stack_trace();
  PASS();
}

static int custom_assert_called = 0;

static void test_assert_handler(const char *msg, const char *file, int line) {
  custom_assert_called = 1;
  /* Do not abort, just return so we can verify */
  (void)msg;
  (void)file;
  (void)line;
}

TEST test_set_assert_handler(void) {
  int rc;
  rc = cmp_set_assert_handler(test_assert_handler);
  ASSERT_EQ(0, rc);
  cmp_assert_fail("Test condition", "test.c", 1);
  ASSERT_EQ(1, custom_assert_called);

  /* Reset to default */
  cmp_set_assert_handler(NULL);
  PASS();
}

SUITE(error_suite) {
  RUN_TEST(test_crash_handler_init);
  RUN_TEST(test_strerror_success);
  RUN_TEST(test_log_debug);
  RUN_TEST(test_dump_stack_trace);
  RUN_TEST(test_set_assert_handler);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  int *p = NULL;

  if (argc > 1 && strcmp(argv[1], "--crash") == 0) {
    cmp_crash_handler_init();
    *p = 1; /* Trigger SIGSEGV */
    return 0;
  }
  if (argc > 1 && strcmp(argv[1], "--assert") == 0) {
    CMP_ASSERT(0 == 1); /* Trigger assert fail */
    return 0;
  }

  GREATEST_MAIN_BEGIN();
  RUN_SUITE(error_suite);
  GREATEST_MAIN_END();
}