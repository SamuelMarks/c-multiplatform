/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_scroll_timeline_lifecycle(void) {
  cmp_scroll_timeline_t *timeline = NULL;
  int res = cmp_scroll_timeline_create(&timeline);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, timeline);

  res = cmp_scroll_timeline_destroy(timeline);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_scroll_timeline_null_args(void) {
  int res = cmp_scroll_timeline_create(NULL);
  float progress;
  cmp_scroll_timeline_t *timeline = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_scroll_timeline_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_scroll_timeline_evaluate(NULL, 10.0f, 100.0f, &progress);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_scroll_timeline_create(&timeline);
  res = cmp_scroll_timeline_evaluate(timeline, 10.0f, 100.0f, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_scroll_timeline_destroy(timeline);
  PASS();
}

TEST test_scroll_timeline_evaluate(void) {
  cmp_scroll_timeline_t *timeline = NULL;
  float progress;
  int res;

  cmp_scroll_timeline_create(&timeline);

  /* 0 / 100 = 0.0 */
  res = cmp_scroll_timeline_evaluate(timeline, 0.0f, 100.0f, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(0.0f, progress, 0.001f);

  /* 50 / 100 = 0.5 */
  res = cmp_scroll_timeline_evaluate(timeline, 50.0f, 100.0f, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(0.5f, progress, 0.001f);

  /* 100 / 100 = 1.0 */
  res = cmp_scroll_timeline_evaluate(timeline, 100.0f, 100.0f, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(1.0f, progress, 0.001f);

  /* 150 / 100 = 1.0 (clamped) */
  res = cmp_scroll_timeline_evaluate(timeline, 150.0f, 100.0f, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(1.0f, progress, 0.001f);

  /* -50 / 100 = 0.0 (clamped) */
  res = cmp_scroll_timeline_evaluate(timeline, -50.0f, 100.0f, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(0.0f, progress, 0.001f);

  /* max = 0 should return 0 */
  res = cmp_scroll_timeline_evaluate(timeline, 50.0f, 0.0f, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(0.0f, progress, 0.001f);

  cmp_scroll_timeline_destroy(timeline);
  PASS();
}

SUITE(scroll_timeline_suite) {
  RUN_TEST(test_scroll_timeline_lifecycle);
  RUN_TEST(test_scroll_timeline_null_args);
  RUN_TEST(test_scroll_timeline_evaluate);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(scroll_timeline_suite);
  GREATEST_MAIN_END();
}
