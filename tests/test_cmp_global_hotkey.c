/* clang-format off */
#include "cmp.h"
#include "cmp_global_hotkey.h"
#include "greatest.h"
/* clang-format on */

TEST test_global_hotkey_lifecycle(void) {
  cmp_global_hotkey_t *hotkey = NULL;
  int id = -1;
  int res;

  res = cmp_global_hotkey_create(&hotkey);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, hotkey);

  res = cmp_global_hotkey_register(hotkey, 'C', 1, &id);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(-1, id);

  res = cmp_global_hotkey_unregister(hotkey, id);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_global_hotkey_destroy(hotkey);
  ASSERT_EQ(CMP_SUCCESS, res);

  PASS();
}

TEST test_global_hotkey_null_args(void) {
  cmp_global_hotkey_t *hotkey = NULL;
  int id = -1;
  int res;

  res = cmp_global_hotkey_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_global_hotkey_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_global_hotkey_create(&hotkey);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_global_hotkey_register(NULL, 'C', 1, &id);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_global_hotkey_register(hotkey, 'C', 1, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_global_hotkey_unregister(NULL, id);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_global_hotkey_destroy(hotkey);
  ASSERT_EQ(CMP_SUCCESS, res);

  PASS();
}

SUITE(global_hotkey_suite) {
  RUN_TEST(test_global_hotkey_lifecycle);
  RUN_TEST(test_global_hotkey_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(global_hotkey_suite);
  GREATEST_MAIN_END();
}
