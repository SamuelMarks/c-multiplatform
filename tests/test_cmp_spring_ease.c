/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_spring_ease_lifecycle(void) {
  cmp_spring_ease_t *spring = NULL;
  int res = cmp_spring_ease_create(1.0f, 100.0f, 10.0f, 0.0f, &spring);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, spring);

  res = cmp_spring_ease_destroy(spring);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_spring_ease_null_args(void) {
  int res = cmp_spring_ease_create(1.0f, 100.0f, 10.0f, 0.0f, NULL);
  float val;
  cmp_spring_ease_t *spring = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_spring_ease_create(-1.0f, 100.0f, 10.0f, 0.0f,
                               NULL); /* Invalid mass */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_spring_ease_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_spring_ease_evaluate(NULL, 1.0, &val);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_spring_ease_create(1.0f, 100.0f, 10.0f, 0.0f, &spring);
  res = cmp_spring_ease_evaluate(spring, 1.0, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_spring_ease_evaluate(spring, -1.0, &val); /* Negative time */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_spring_ease_destroy(spring);
  PASS();
}

TEST test_spring_ease_evaluate(void) {
  cmp_spring_ease_t *spring = NULL;
  float val;
  int res;

  /* Critically damped spring */
  cmp_spring_ease_create(1.0f, 100.0f, 20.0f, 0.0f, &spring);

  /* t=0, val=0 */
  res = cmp_spring_ease_evaluate(spring, 0.0, &val);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(0.0f, val, 0.001f);

  /* t=10 (infinity), val=1 */
  res = cmp_spring_ease_evaluate(spring, 10.0, &val);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(1.0f, val, 0.001f);

  cmp_spring_ease_destroy(spring);
  PASS();
}

SUITE(spring_ease_suite) {
  RUN_TEST(test_spring_ease_lifecycle);
  RUN_TEST(test_spring_ease_null_args);
  RUN_TEST(test_spring_ease_evaluate);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(spring_ease_suite);
  GREATEST_MAIN_END();
}
