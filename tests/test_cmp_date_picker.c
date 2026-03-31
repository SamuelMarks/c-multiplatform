/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_date_picker_lifecycle(void) {
  cmp_date_picker_t *picker = NULL;
  int res = cmp_date_picker_create(&picker);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, picker);

  res = cmp_date_picker_destroy(picker);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_date_picker_null_args(void) {
  int res = cmp_date_picker_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_date_picker_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

SUITE(date_picker_suite) {
  RUN_TEST(test_date_picker_lifecycle);
  RUN_TEST(test_date_picker_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(date_picker_suite);
  GREATEST_MAIN_END();
}
