/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_a11y_transparency_lifecycle(void) {
  cmp_a11y_transparency_t *trans = NULL;
  int res = cmp_a11y_transparency_create(&trans);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, trans);

  res = cmp_a11y_transparency_destroy(trans);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_a11y_transparency_null_args(void) {
  cmp_a11y_transparency_t *trans = NULL;
  float opacity = 0.5f;
  int res = cmp_a11y_transparency_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_transparency_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_transparency_set(NULL, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_a11y_transparency_create(&trans);

  res = cmp_a11y_transparency_apply(NULL, &opacity, 1.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_transparency_apply(trans, NULL, 1.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_a11y_transparency_destroy(trans);
  PASS();
}

TEST test_a11y_transparency_apply(void) {
  cmp_a11y_transparency_t *trans = NULL;
  float opacity = 0.5f;
  int res;

  cmp_a11y_transparency_create(&trans);

  /* By default, not enabled */
  res = cmp_a11y_transparency_apply(trans, &opacity, 1.0f);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(0.5f, opacity, "%f"); /* Should not change */

  /* Enable reduced transparency */
  res = cmp_a11y_transparency_set(trans, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Should override opacity with fallback */
  res = cmp_a11y_transparency_apply(trans, &opacity, 1.0f);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(1.0f, opacity, "%f"); /* Should change to fallback */

  cmp_a11y_transparency_destroy(trans);
  PASS();
}

SUITE(a11y_transparency_suite) {
  RUN_TEST(test_a11y_transparency_lifecycle);
  RUN_TEST(test_a11y_transparency_null_args);
  RUN_TEST(test_a11y_transparency_apply);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  (void)argc;
  (void)argv;
  RUN_SUITE(a11y_transparency_suite);
  GREATEST_MAIN_END();
}
