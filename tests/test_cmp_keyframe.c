/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_keyframe_lifecycle(void) {
  cmp_keyframe_t *kf = NULL;
  int res = cmp_keyframe_create(&kf);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, kf);

  res = cmp_keyframe_destroy(kf);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_keyframe_null_args(void) {
  int res = cmp_keyframe_create(NULL);
  float progress;
  cmp_keyframe_t *kf = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_keyframe_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_keyframe_add_stop(NULL, 0.5f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_keyframe_step(NULL, 16.6, CMP_ANIMATION_PLAY_STATE_RUNNING,
                          &progress);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_keyframe_create(&kf);
  res = cmp_keyframe_step(kf, 16.6, CMP_ANIMATION_PLAY_STATE_RUNNING, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_keyframe_destroy(kf);
  PASS();
}

TEST test_keyframe_add_stop(void) {
  cmp_keyframe_t *kf = NULL;
  int res;
  cmp_keyframe_create(&kf);

  res = cmp_keyframe_add_stop(kf, 0.0f);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_keyframe_add_stop(kf, 0.5f);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_keyframe_add_stop(kf, 1.0f);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_keyframe_add_stop(kf, 1.5f); /* Invalid */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_keyframe_add_stop(kf, -0.1f); /* Invalid */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_keyframe_destroy(kf);
  PASS();
}

SUITE(keyframe_suite) {
  RUN_TEST(test_keyframe_lifecycle);
  RUN_TEST(test_keyframe_null_args);
  RUN_TEST(test_keyframe_add_stop);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(keyframe_suite);
  GREATEST_MAIN_END();
}
