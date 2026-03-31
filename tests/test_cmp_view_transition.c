/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_view_transition_lifecycle(void) {
  cmp_view_transition_t *transition = NULL;
  int res = cmp_view_transition_create(&transition);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, transition);

  res = cmp_view_transition_destroy(transition);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_view_transition_null_args(void) {
  int res = cmp_view_transition_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_view_transition_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_view_transition_start(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

TEST test_view_transition_start(void) {
  cmp_view_transition_t *transition = NULL;
  int res;
  cmp_view_transition_create(&transition);

  res = cmp_view_transition_start(transition);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_view_transition_destroy(transition);
  PASS();
}

SUITE(view_transition_suite) {
  RUN_TEST(test_view_transition_lifecycle);
  RUN_TEST(test_view_transition_null_args);
  RUN_TEST(test_view_transition_start);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(view_transition_suite);
  GREATEST_MAIN_END();
}
