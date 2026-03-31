/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_step_ease_lifecycle(void) {
  cmp_step_ease_t *step = NULL;
  int res = cmp_step_ease_create(5, CMP_STEP_POSITION_END, &step);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, step);

  res = cmp_step_ease_destroy(step);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_step_ease_null_args(void) {
  int res = cmp_step_ease_create(5, CMP_STEP_POSITION_END, NULL);
  float val;
  cmp_step_ease_t *step = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res =
      cmp_step_ease_create(0, CMP_STEP_POSITION_END, NULL); /* Invalid steps */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_step_ease_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_step_ease_evaluate(NULL, 0.5f, &val);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_step_ease_create(5, CMP_STEP_POSITION_END, &step);
  res = cmp_step_ease_evaluate(step, 0.5f, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_step_ease_evaluate(step, -0.1f, &val);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_step_ease_destroy(step);
  PASS();
}

TEST test_step_ease_evaluate_end(void) {
  cmp_step_ease_t *step = NULL;
  float val;

  cmp_step_ease_create(4, CMP_STEP_POSITION_END, &step);

  cmp_step_ease_evaluate(step, 0.0f, &val);
  ASSERT_IN_RANGE(0.0f, val, 0.001f);

  cmp_step_ease_evaluate(step, 0.24f, &val);
  ASSERT_IN_RANGE(0.0f, val, 0.001f);

  cmp_step_ease_evaluate(step, 0.26f, &val);
  ASSERT_IN_RANGE(0.25f, val, 0.001f);

  cmp_step_ease_evaluate(step, 0.99f, &val);
  ASSERT_IN_RANGE(0.75f, val, 0.001f);

  cmp_step_ease_evaluate(step, 1.0f, &val);
  ASSERT_IN_RANGE(1.0f, val, 0.001f);

  cmp_step_ease_destroy(step);
  PASS();
}

TEST test_step_ease_evaluate_start(void) {
  cmp_step_ease_t *step = NULL;
  float val;
  cmp_step_ease_create(4, CMP_STEP_POSITION_START, &step);

  cmp_step_ease_evaluate(step, 0.0f, &val);
  ASSERT_IN_RANGE(0.25f, val, 0.001f);

  cmp_step_ease_evaluate(step, 0.24f, &val);
  ASSERT_IN_RANGE(0.25f, val, 0.001f);

  cmp_step_ease_evaluate(step, 0.26f, &val);
  ASSERT_IN_RANGE(0.5f, val, 0.001f);

  cmp_step_ease_evaluate(step, 0.99f, &val);
  ASSERT_IN_RANGE(1.0f, val, 0.001f);

  cmp_step_ease_evaluate(step, 1.0f, &val);
  ASSERT_IN_RANGE(1.0f, val, 0.001f);

  cmp_step_ease_destroy(step);
  PASS();
}

SUITE(step_ease_suite) {
  RUN_TEST(test_step_ease_lifecycle);
  RUN_TEST(test_step_ease_null_args);
  RUN_TEST(test_step_ease_evaluate_end);
  RUN_TEST(test_step_ease_evaluate_start);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(step_ease_suite);
  GREATEST_MAIN_END();
}
