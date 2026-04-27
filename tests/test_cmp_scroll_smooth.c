/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_scroll_smooth_lifecycle(void) {
  cmp_scroll_smooth_t *smooth = NULL;
  int res = cmp_scroll_smooth_create(&smooth);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, smooth);

  res = cmp_scroll_smooth_destroy(smooth);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_scroll_smooth_null_args(void) {
  int res = cmp_scroll_smooth_create(NULL);
  cmp_scroll_smooth_t *smooth = NULL;
  float pos;
  int complete;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_scroll_smooth_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_scroll_smooth_start(NULL, 0.0f, 100.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_scroll_smooth_create(&smooth);
  res = cmp_scroll_smooth_step(NULL, 16.0f, &pos, &complete);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_scroll_smooth_step(smooth, 16.0f, NULL, &complete);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_scroll_smooth_step(smooth, 16.0f, &pos, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_scroll_smooth_destroy(smooth);
  PASS();
}

TEST test_scroll_smooth_animation(void) {
  cmp_scroll_smooth_t *smooth = NULL;
  float pos = 0.0f;
  int complete = 0;
  int res = cmp_scroll_smooth_create(&smooth);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Initially complete */
  res = cmp_scroll_smooth_step(smooth, 16.0f, &pos, &complete);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, complete);

  res = cmp_scroll_smooth_start(smooth, 0.0f, 100.0f);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_scroll_smooth_step(smooth, 150.0f, &pos, &complete);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, complete);
  ASSERT(pos > 0.0f && pos < 100.0f);

  res = cmp_scroll_smooth_step(smooth, 150.0f, &pos, &complete);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, complete);
  ASSERT_EQ_FMT(100.0f, pos, "%f");

  /* Call when already complete */
  res = cmp_scroll_smooth_step(smooth, 16.0f, &pos, &complete);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, complete);
  ASSERT_EQ_FMT(100.0f, pos, "%f");

  cmp_scroll_smooth_destroy(smooth);
  PASS();
}

SUITE(scroll_smooth_suite) {
  RUN_TEST(test_scroll_smooth_lifecycle);
  RUN_TEST(test_scroll_smooth_null_args);
  RUN_TEST(test_scroll_smooth_animation);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(scroll_smooth_suite);
  GREATEST_MAIN_END();
}
