/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_range_slider_lifecycle(void) {
  cmp_range_slider_t *slider = NULL;
  int res = cmp_range_slider_create(&slider);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, slider);

  res = cmp_range_slider_destroy(slider);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_range_slider_null_args(void) {
  int res = cmp_range_slider_create(NULL);
  cmp_range_slider_t *slider = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_range_slider_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_range_slider_set_value(NULL, 0, 50.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_range_slider_create(&slider);

  res = cmp_range_slider_set_value(slider, -1, 50.0f); /* Invalid index */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_range_slider_set_value(slider, 2, 50.0f); /* Invalid index */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_range_slider_destroy(slider);
  PASS();
}

TEST test_range_slider_set_value(void) {
  cmp_range_slider_t *slider = NULL;
  int res;
  cmp_range_slider_create(&slider);

  res = cmp_range_slider_set_value(slider, 0, 25.0f);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_range_slider_set_value(slider, 1, 75.0f);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_range_slider_destroy(slider);
  PASS();
}

SUITE(range_slider_suite) {
  RUN_TEST(test_range_slider_lifecycle);
  RUN_TEST(test_range_slider_null_args);
  RUN_TEST(test_range_slider_set_value);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(range_slider_suite);
  GREATEST_MAIN_END();
}
