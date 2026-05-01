/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_toast_lifecycle(void) {
  cmp_toast_manager_t *manager = NULL;
  int res;

  res = cmp_toast_manager_create(&manager);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, manager);

  res = cmp_toast_manager_destroy(manager);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_toast_null_args(void) {
  cmp_toast_manager_t *manager = NULL;
  size_t count;
  int res;

  res = cmp_toast_manager_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_toast_manager_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_toast_manager_create(&manager);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_toast_manager_push(NULL, CMP_TOAST_INFO, "Msg", 1000);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_toast_manager_push(manager, CMP_TOAST_INFO, NULL, 1000);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_toast_manager_tick(NULL, 16);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_toast_manager_get_active_count(NULL, &count);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_toast_manager_get_active_count(manager, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_toast_manager_destroy(manager);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_toast_operations(void) {
  cmp_toast_manager_t *manager = NULL;
  size_t count;
  int res;

  res = cmp_toast_manager_create(&manager);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Push some toasts */
  res = cmp_toast_manager_push(manager, CMP_TOAST_INFO, "Toast 1", 1000);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_toast_manager_push(manager, CMP_TOAST_WARNING, "Toast 2", 2000);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_toast_manager_get_active_count(manager, &count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(2, (int)count);

  /* Tick forward by 500ms - both should survive */
  res = cmp_toast_manager_tick(manager, 500);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_toast_manager_get_active_count(manager, &count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(2, (int)count);

  /* Tick forward by another 600ms - first should expire (total 1100) */
  res = cmp_toast_manager_tick(manager, 600);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_toast_manager_get_active_count(manager, &count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, (int)count);

  /* Tick forward by 1000ms - second should expire (total 2100) */
  res = cmp_toast_manager_tick(manager, 1000);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_toast_manager_get_active_count(manager, &count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, (int)count);

  res = cmp_toast_manager_destroy(manager);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(toast_notifications_suite) {
  RUN_TEST(test_toast_lifecycle);
  RUN_TEST(test_toast_null_args);
  RUN_TEST(test_toast_operations);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(toast_notifications_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
