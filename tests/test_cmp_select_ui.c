/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_select_ui_lifecycle(void) {
  cmp_select_ui_t *select = NULL;
  int res = cmp_select_ui_create(&select);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, select);

  res = cmp_select_ui_destroy(select);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_select_ui_null_args(void) {
  int res = cmp_select_ui_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_select_ui_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_select_ui_open(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

TEST test_select_ui_open(void) {
  cmp_select_ui_t *select = NULL;
  int res;
  cmp_select_ui_create(&select);

  res = cmp_select_ui_open(select);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_select_ui_destroy(select);
  PASS();
}

SUITE(select_ui_suite) {
  RUN_TEST(test_select_ui_lifecycle);
  RUN_TEST(test_select_ui_null_args);
  RUN_TEST(test_select_ui_open);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(select_ui_suite);
  GREATEST_MAIN_END();
}
