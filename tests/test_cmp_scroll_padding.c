/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_scroll_padding_apply(void) {
  cmp_rect_f_t element_bounds = {0, 100, 50, 50};
  cmp_scroll_padding_t padding = {10, 10, 10, 10};
  cmp_scroll_padding_t margin = {5, 5, 5, 5};
  float out_target_scroll_y;
  int res;

  res = cmp_scroll_padding_apply(&element_bounds, &padding, &margin,
                                 &out_target_scroll_y);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(85.0f, out_target_scroll_y, "%f"); /* 100 - 5 - 10 = 85 */

  res = cmp_scroll_padding_apply(&element_bounds, NULL, &margin,
                                 &out_target_scroll_y);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(95.0f, out_target_scroll_y, "%f");

  res = cmp_scroll_padding_apply(&element_bounds, &padding, NULL,
                                 &out_target_scroll_y);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(90.0f, out_target_scroll_y, "%f");

  PASS();
}

TEST test_scroll_padding_null_args(void) {
  cmp_rect_f_t element_bounds = {0, 100, 50, 50};
  cmp_scroll_padding_t padding = {10, 10, 10, 10};
  cmp_scroll_padding_t margin = {5, 5, 5, 5};
  float out_target_scroll_y;
  int res;

  res = cmp_scroll_padding_apply(NULL, &padding, &margin, &out_target_scroll_y);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_scroll_padding_apply(&element_bounds, &padding, &margin, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

SUITE(scroll_padding_suite) {
  RUN_TEST(test_scroll_padding_apply);
  RUN_TEST(test_scroll_padding_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(scroll_padding_suite);
  GREATEST_MAIN_END();
}
