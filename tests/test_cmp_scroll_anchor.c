/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_scroll_anchor_lifecycle(void) {
  cmp_scroll_anchor_t *anchor = NULL;
  int res = cmp_scroll_anchor_create(&anchor);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, anchor);

  res = cmp_scroll_anchor_destroy(anchor);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_scroll_anchor_null_args(void) {
  int res = cmp_scroll_anchor_create(NULL);
  cmp_scroll_anchor_t *anchor = NULL;
  float delta;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_scroll_anchor_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_scroll_anchor_save(NULL, 1, 100.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_scroll_anchor_create(&anchor);
  res = cmp_scroll_anchor_restore(anchor, 1, 150.0f, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_scroll_anchor_restore(NULL, 1, 150.0f, &delta);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_scroll_anchor_destroy(anchor);
  PASS();
}

TEST test_scroll_anchor_save_restore(void) {
  cmp_scroll_anchor_t *anchor = NULL;
  float delta;
  int res = cmp_scroll_anchor_create(&anchor);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_scroll_anchor_save(anchor, 42, 100.0f);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_scroll_anchor_restore(anchor, 42, 150.0f, &delta);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(50.0f, delta, "%f"); /* 150 - 100 */

  /* Test different element ID */
  res = cmp_scroll_anchor_restore(anchor, 99, 200.0f, &delta);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(0.0f, delta, "%f");

  cmp_scroll_anchor_destroy(anchor);
  PASS();
}

SUITE(scroll_anchor_suite) {
  RUN_TEST(test_scroll_anchor_lifecycle);
  RUN_TEST(test_scroll_anchor_null_args);
  RUN_TEST(test_scroll_anchor_save_restore);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(scroll_anchor_suite);
  GREATEST_MAIN_END();
}
