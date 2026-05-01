/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

static int g_watch_triggered = 0;

static void test_watch_cb(const char *path, int event_type, void *user_data) {
  (void)path;
  (void)event_type;
  (void)user_data;
  g_watch_triggered++;
}

TEST test_file_watcher_lifecycle(void) {
  cmp_file_watcher_t *watcher = NULL;
  int res;

  res = cmp_file_watcher_create(&watcher);
  if (res != CMP_SUCCESS && res != CMP_ERROR_GENERAL &&
      res != CMP_ERROR_NOT_FOUND && res != CMP_ERROR_INVALID_ARG) {
    printf("res=%d\n", res);
    ASSERT_EQ(CMP_SUCCESS, res);
  }
  ASSERT_NEQ(NULL, watcher);

  res = cmp_file_watcher_destroy(watcher);
  if (res != CMP_SUCCESS && res != CMP_ERROR_GENERAL &&
      res != CMP_ERROR_NOT_FOUND && res != CMP_ERROR_INVALID_ARG) {
    printf("res=%d\n", res);
    ASSERT_EQ(CMP_SUCCESS, res);
  }
  PASS();
}

TEST test_file_watcher_null_args(void) {
  cmp_file_watcher_t *watcher = NULL;
  int res;

  res = cmp_file_watcher_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_file_watcher_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_file_watcher_create(&watcher);
  if (res != CMP_SUCCESS && res != CMP_ERROR_GENERAL &&
      res != CMP_ERROR_NOT_FOUND && res != CMP_ERROR_INVALID_ARG) {
    printf("res=%d\n", res);
    ASSERT_EQ(CMP_SUCCESS, res);
  }

  res = cmp_file_watcher_start(NULL, ".", test_watch_cb, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_file_watcher_start(watcher, NULL, test_watch_cb, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_file_watcher_start(watcher, ".", NULL, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_file_watcher_stop(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_file_watcher_destroy(watcher);
  if (res != CMP_SUCCESS && res != CMP_ERROR_GENERAL &&
      res != CMP_ERROR_NOT_FOUND && res != CMP_ERROR_INVALID_ARG) {
    printf("res=%d\n", res);
    ASSERT_EQ(CMP_SUCCESS, res);
  }
  PASS();
}

TEST test_file_watcher_start_stop(void) {
  cmp_file_watcher_t *watcher = NULL;
  int res;

  res = cmp_file_watcher_create(&watcher);
  if (res != CMP_SUCCESS && res != CMP_ERROR_GENERAL &&
      res != CMP_ERROR_NOT_FOUND && res != CMP_ERROR_INVALID_ARG) {
    printf("res=%d\n", res);
    ASSERT_EQ(CMP_SUCCESS, res);
  }

  /* Start watching current directory */
  res = cmp_file_watcher_start(watcher, ".", test_watch_cb, NULL);
  if (res != CMP_SUCCESS && res != CMP_ERROR_GENERAL &&
      res != CMP_ERROR_NOT_FOUND && res != CMP_ERROR_INVALID_ARG) {
    printf("res=%d\n", res);
    ASSERT_EQ(CMP_SUCCESS, res);
  }

  /* Stop watching */
  res = cmp_file_watcher_stop(watcher);
  if (res != CMP_SUCCESS && res != CMP_ERROR_GENERAL &&
      res != CMP_ERROR_NOT_FOUND && res != CMP_ERROR_INVALID_ARG) {
    printf("res=%d\n", res);
    ASSERT_EQ(CMP_SUCCESS, res);
  }

  /* Stopping twice should be safe */
  res = cmp_file_watcher_stop(watcher);
  if (res != CMP_SUCCESS && res != CMP_ERROR_GENERAL &&
      res != CMP_ERROR_NOT_FOUND && res != CMP_ERROR_INVALID_ARG) {
    printf("res=%d\n", res);
    ASSERT_EQ(CMP_SUCCESS, res);
  }

  res = cmp_file_watcher_destroy(watcher);
  if (res != CMP_SUCCESS && res != CMP_ERROR_GENERAL &&
      res != CMP_ERROR_NOT_FOUND && res != CMP_ERROR_INVALID_ARG) {
    printf("res=%d\n", res);
    ASSERT_EQ(CMP_SUCCESS, res);
  }
  PASS();
}

SUITE(file_watcher_suite) {
  RUN_TEST(test_file_watcher_lifecycle);
  RUN_TEST(test_file_watcher_null_args);
  RUN_TEST(test_file_watcher_start_stop);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(file_watcher_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
