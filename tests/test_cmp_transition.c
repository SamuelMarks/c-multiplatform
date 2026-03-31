/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_transition_lifecycle(void) {
  cmp_transition_t *transition = NULL;
  int res = cmp_transition_create(100.0, 0.0, CMP_TRANSITION_BEHAVIOR_NORMAL,
                                  &transition);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, transition);

  res = cmp_transition_destroy(transition);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_transition_null_args(void) {
  int res =
      cmp_transition_create(100.0, 0.0, CMP_TRANSITION_BEHAVIOR_NORMAL, NULL);
  float progress;
  cmp_transition_t *transition = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_transition_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_transition_step(NULL, 16.6, &progress);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_transition_create(100.0, 0.0, CMP_TRANSITION_BEHAVIOR_NORMAL,
                        &transition);
  res = cmp_transition_step(transition, 16.6, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_transition_destroy(transition);
  PASS();
}

TEST test_transition_step_normal(void) {
  cmp_transition_t *transition = NULL;
  float progress;
  int res;
  cmp_transition_create(100.0, 0.0, CMP_TRANSITION_BEHAVIOR_NORMAL,
                        &transition);

  /* Step 1 */
  res = cmp_transition_step(transition, 50.0, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(0.5f, progress, "%f");

  /* Step 2 */
  res = cmp_transition_step(transition, 50.0, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(1.0f, progress, "%f");

  /* Step past end */
  res = cmp_transition_step(transition, 50.0, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(1.0f, progress, "%f");

  cmp_transition_destroy(transition);
  PASS();
}

TEST test_transition_step_with_delay(void) {
  cmp_transition_t *transition = NULL;
  float progress;
  int res;
  cmp_transition_create(100.0, 50.0, CMP_TRANSITION_BEHAVIOR_NORMAL,
                        &transition);

  /* Within delay */
  res = cmp_transition_step(transition, 25.0, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(0.0f, progress, "%f");

  /* End of delay */
  res = cmp_transition_step(transition, 25.0, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(0.0f, progress, "%f");

  /* Halfway through active duration */
  res = cmp_transition_step(transition, 50.0, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(0.5f, progress, "%f");

  /* End of active duration */
  res = cmp_transition_step(transition, 50.0, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(1.0f, progress, "%f");

  cmp_transition_destroy(transition);
  PASS();
}

TEST test_transition_behavior_discrete(void) {
  cmp_transition_t *transition = NULL;
  float progress;
  int res;
  cmp_transition_create(100.0, 0.0, CMP_TRANSITION_BEHAVIOR_ALLOW_DISCRETE,
                        &transition);

  /* Before 50% */
  res = cmp_transition_step(transition, 49.0, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(0.0f, progress, "%f");

  /* At 50% */
  res = cmp_transition_step(transition, 1.0, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(1.0f, progress, "%f");

  /* Past 50% */
  res = cmp_transition_step(transition, 50.0, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(1.0f, progress, "%f");

  cmp_transition_destroy(transition);
  PASS();
}

SUITE(transition_suite) {
  RUN_TEST(test_transition_lifecycle);
  RUN_TEST(test_transition_null_args);
  RUN_TEST(test_transition_step_normal);
  RUN_TEST(test_transition_step_with_delay);
  RUN_TEST(test_transition_behavior_discrete);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(transition_suite);
  GREATEST_MAIN_END();
}
