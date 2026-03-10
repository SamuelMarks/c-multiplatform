/* clang-format off */
#include "cmpc/cmp_text_field.h"
#include "test_utils.h"
/* clang-format on */

int main(void) {
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(0));
  CMP_TEST_OK(cmp_text_field_test_set_color_fail_after(1));
  CMP_TEST_OK(cmp_text_field_test_set_overflow_fail_after(1));
  CMP_TEST_OK(cmp_text_field_test_set_font_metrics_fail_after(1));
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  CMPColor color;
  cmp_text_field_test_set_color_fail_after(1);
  CMP_TEST_EXPECT(cmp_text_field_test_color_set(&color, 1.0f, 1.0f, 1.0f, 1.0f),
                  CMP_ERR_IO);

  cmp_text_field_test_set_overflow_fail_after(1);
  cmp_usize val;
  CMP_TEST_EXPECT(cmp_text_field_test_add_overflow(1, 1, &val),
                  CMP_ERR_OVERFLOW);

  return CMP_OK;
}
