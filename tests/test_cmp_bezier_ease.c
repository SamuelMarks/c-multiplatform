/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_bezier_ease_lifecycle(void) {
  cmp_bezier_ease_t *bezier = NULL;
  int res = cmp_bezier_ease_create(0.25f, 0.1f, 0.25f, 1.0f, &bezier);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, bezier);

  res = cmp_bezier_ease_destroy(bezier);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_bezier_ease_null_args(void) {
  int res = cmp_bezier_ease_create(0.25f, 0.1f, 0.25f, 1.0f, NULL);
  float val;
  cmp_bezier_ease_t *bezier = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_bezier_ease_create(-0.1f, 0.1f, 0.25f, 1.0f, NULL); /* Invalid X */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_bezier_ease_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_bezier_ease_evaluate(NULL, 0.5f, &val);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_bezier_ease_create(0.25f, 0.1f, 0.25f, 1.0f, &bezier);
  res = cmp_bezier_ease_evaluate(bezier, 0.5f, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_bezier_ease_evaluate(bezier, -0.1f, &val);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_bezier_ease_destroy(bezier);
  PASS();
}

TEST test_bezier_ease_evaluate(void) {
  cmp_bezier_ease_t *bezier = NULL;
  float val;
  int res;

  /* Linear-ish */
  cmp_bezier_ease_create(0.0f, 0.0f, 1.0f, 1.0f, &bezier);

  res = cmp_bezier_ease_evaluate(bezier, 0.0f, &val);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(0.0f, val, 0.001f);

  res = cmp_bezier_ease_evaluate(bezier, 1.0f, &val);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(1.0f, val, 0.001f);

  res = cmp_bezier_ease_evaluate(bezier, 0.5f, &val);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(0.5f, val, 0.001f);

  cmp_bezier_ease_destroy(bezier);
  PASS();
}

SUITE(bezier_ease_suite) {
  RUN_TEST(test_bezier_ease_lifecycle);
  RUN_TEST(test_bezier_ease_null_args);
  RUN_TEST(test_bezier_ease_evaluate);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(bezier_ease_suite);
  GREATEST_MAIN_END();
}
