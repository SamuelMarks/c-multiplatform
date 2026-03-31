/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_indeterminate_lifecycle(void) {
  cmp_indeterminate_t *state = NULL;
  int res = cmp_indeterminate_create(&state);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, state);

  res = cmp_indeterminate_destroy(state);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_indeterminate_null_args(void) {
  int res = cmp_indeterminate_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_indeterminate_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_indeterminate_set(NULL, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

TEST test_indeterminate_set(void) {
  cmp_indeterminate_t *state = NULL;
  int res;
  cmp_indeterminate_create(&state);

  res = cmp_indeterminate_set(state, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_indeterminate_set(state, 0);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_indeterminate_destroy(state);
  PASS();
}

SUITE(indeterminate_suite) {
  RUN_TEST(test_indeterminate_lifecycle);
  RUN_TEST(test_indeterminate_null_args);
  RUN_TEST(test_indeterminate_set);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(indeterminate_suite);
  GREATEST_MAIN_END();
}
