/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_scrollbar_gutter_auto_no_overflow(void) {
  float left, right;
  int res = cmp_scrollbar_gutter_calculate(CMP_SCROLLBAR_GUTTER_AUTO, 0, 15.0f,
                                           &left, &right);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(0.0f, left, "%f");
  ASSERT_EQ_FMT(0.0f, right, "%f");
  PASS();
}

TEST test_scrollbar_gutter_auto_overflow(void) {
  float left, right;
  int res = cmp_scrollbar_gutter_calculate(CMP_SCROLLBAR_GUTTER_AUTO, 1, 15.0f,
                                           &left, &right);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(0.0f, left, "%f");
  ASSERT_EQ_FMT(15.0f, right, "%f");
  PASS();
}

TEST test_scrollbar_gutter_stable_no_overflow(void) {
  float left, right;
  int res = cmp_scrollbar_gutter_calculate(CMP_SCROLLBAR_GUTTER_STABLE, 0,
                                           15.0f, &left, &right);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(0.0f, left, "%f");
  ASSERT_EQ_FMT(15.0f, right, "%f");
  PASS();
}

TEST test_scrollbar_gutter_stable_both_edges(void) {
  float left, right;
  int res = cmp_scrollbar_gutter_calculate(
      CMP_SCROLLBAR_GUTTER_STABLE_BOTH_EDGES, 0, 15.0f, &left, &right);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(15.0f, left, "%f");
  ASSERT_EQ_FMT(15.0f, right, "%f");
  PASS();
}

TEST test_scrollbar_gutter_invalid_args(void) {
  float left;
  int res = cmp_scrollbar_gutter_calculate(CMP_SCROLLBAR_GUTTER_AUTO, 0, 15.0f,
                                           NULL, &left);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);
  res = cmp_scrollbar_gutter_calculate(CMP_SCROLLBAR_GUTTER_AUTO, 0, 15.0f,
                                       &left, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);
  res = cmp_scrollbar_gutter_calculate(999, 0, 15.0f, &left, &left);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);
  PASS();
}

SUITE(scrollbar_gutter_suite) {
  RUN_TEST(test_scrollbar_gutter_auto_no_overflow);
  RUN_TEST(test_scrollbar_gutter_auto_overflow);
  RUN_TEST(test_scrollbar_gutter_stable_no_overflow);
  RUN_TEST(test_scrollbar_gutter_stable_both_edges);
  RUN_TEST(test_scrollbar_gutter_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(scrollbar_gutter_suite);
  GREATEST_MAIN_END();
}
