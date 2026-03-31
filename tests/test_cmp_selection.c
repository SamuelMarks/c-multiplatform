/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_selection_lifecycle(void) {
  cmp_selection_t *selection = NULL;
  int res = cmp_selection_create(&selection);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, selection);

  res = cmp_selection_destroy(selection);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_selection_null_args(void) {
  int res = cmp_selection_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_selection_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

SUITE(selection_suite) {
  RUN_TEST(test_selection_lifecycle);
  RUN_TEST(test_selection_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(selection_suite);
  GREATEST_MAIN_END();
}
