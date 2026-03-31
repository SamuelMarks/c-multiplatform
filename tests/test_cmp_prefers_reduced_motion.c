/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_prefers_reduced_motion_lifecycle(void) {
  cmp_prefers_reduced_motion_t *rm = NULL;
  int res = cmp_prefers_reduced_motion_create(&rm);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, rm);

  res = cmp_prefers_reduced_motion_destroy(rm);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_prefers_reduced_motion_null_args(void) {
  int res = cmp_prefers_reduced_motion_create(NULL);
  cmp_prefers_reduced_motion_t *rm = NULL;
  float duration;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_prefers_reduced_motion_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_prefers_reduced_motion_create(&rm);

  res = cmp_prefers_reduced_motion_set(NULL, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_prefers_reduced_motion_apply(NULL, &duration);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_prefers_reduced_motion_apply(rm, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_prefers_reduced_motion_destroy(rm);
  PASS();
}

TEST test_prefers_reduced_motion_operations(void) {
  cmp_prefers_reduced_motion_t *rm = NULL;
  int res;
  float duration;

  cmp_prefers_reduced_motion_create(&rm);

  /* By default it should be full motion */
  duration = 100.0f;
  res = cmp_prefers_reduced_motion_apply(rm, &duration);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(100.0f, duration, "%f");

  /* Turn it on */
  res = cmp_prefers_reduced_motion_set(rm, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  duration = 100.0f;
  res = cmp_prefers_reduced_motion_apply(rm, &duration);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(0.0f, duration, "%f"); /* Zeros out animation */

  /* Turn it off */
  res = cmp_prefers_reduced_motion_set(rm, 0);
  ASSERT_EQ(CMP_SUCCESS, res);

  duration = 50.0f;
  res = cmp_prefers_reduced_motion_apply(rm, &duration);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(50.0f, duration, "%f");

  cmp_prefers_reduced_motion_destroy(rm);
  PASS();
}

SUITE(prefers_reduced_motion_suite) {
  RUN_TEST(test_prefers_reduced_motion_lifecycle);
  RUN_TEST(test_prefers_reduced_motion_null_args);
  RUN_TEST(test_prefers_reduced_motion_operations);
}

#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(prefers_reduced_motion_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
