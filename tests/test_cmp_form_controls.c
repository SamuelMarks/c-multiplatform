/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_form_controls_lifecycle(void) {
  cmp_form_controls_t *controls = NULL;
  int res = cmp_form_controls_create(&controls);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, controls);

  res = cmp_form_controls_destroy(controls);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_form_controls_null_args(void) {
  int res = cmp_form_controls_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_form_controls_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

SUITE(form_controls_suite) {
  RUN_TEST(test_form_controls_lifecycle);
  RUN_TEST(test_form_controls_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(form_controls_suite);
  GREATEST_MAIN_END();
}
