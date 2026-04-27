/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_scroll_snap_stop_evaluate(void) {
  int res;

  /* ALWAYS mode always returns 1 regardless of velocity */
  res = cmp_scroll_snap_stop_evaluate(CMP_SCROLL_SNAP_STOP_ALWAYS, 2000.0f);
  ASSERT_EQ(1, res);

  res = cmp_scroll_snap_stop_evaluate(CMP_SCROLL_SNAP_STOP_ALWAYS, 0.0f);
  ASSERT_EQ(1, res);

  /* NORMAL mode returns 1 if velocity < 1000, else 0 */
  res = cmp_scroll_snap_stop_evaluate(CMP_SCROLL_SNAP_STOP_NORMAL, 500.0f);
  ASSERT_EQ(1, res);

  res = cmp_scroll_snap_stop_evaluate(CMP_SCROLL_SNAP_STOP_NORMAL, -500.0f);
  ASSERT_EQ(1, res);

  res = cmp_scroll_snap_stop_evaluate(CMP_SCROLL_SNAP_STOP_NORMAL, 1500.0f);
  ASSERT_EQ(0, res);

  res = cmp_scroll_snap_stop_evaluate(CMP_SCROLL_SNAP_STOP_NORMAL, -1500.0f);
  ASSERT_EQ(0, res);

  /* Invalid mode */
  res = cmp_scroll_snap_stop_evaluate((cmp_scroll_snap_stop_t)999, 500.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

SUITE(scroll_snap_stop_suite) { RUN_TEST(test_scroll_snap_stop_evaluate); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(scroll_snap_stop_suite);
  GREATEST_MAIN_END();
}
