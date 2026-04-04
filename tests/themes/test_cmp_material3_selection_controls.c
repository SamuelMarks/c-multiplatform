/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_selection_controls.h"
#include "greatest.h"
/* clang-format on */

TEST test_m3_checkbox_resolve(void) {
  cmp_m3_checkbox_metrics_t m;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_m3_checkbox_resolve(NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_checkbox_resolve(&m));
  ASSERT_EQ(18.0f, m.box_size);
  ASSERT_EQ(48.0f, m.touch_target_size);
  ASSERT_EQ(2.0f, m.border_thickness_unchecked);
  ASSERT_EQ(CMP_M3_SHAPE_EXTRA_SMALL, m.shape);
  PASS();
}

TEST test_m3_radio_resolve(void) {
  cmp_m3_radio_metrics_t m;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_m3_radio_resolve(NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_radio_resolve(&m));
  ASSERT_EQ(20.0f, m.outer_ring_size);
  ASSERT_EQ(10.0f, m.inner_dot_size);
  ASSERT_EQ(48.0f, m.touch_target_size);
  ASSERT_EQ(2.0f, m.border_thickness);
  ASSERT_EQ(CMP_M3_SHAPE_FULL, m.shape);
  PASS();
}

TEST test_m3_switch_resolve(void) {
  cmp_m3_switch_metrics_t m;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_m3_switch_resolve(NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_switch_resolve(&m));
  ASSERT_EQ(52.0f, m.track_width);
  ASSERT_EQ(32.0f, m.track_height);
  ASSERT_EQ(48.0f, m.touch_target_size);
  ASSERT_EQ(16.0f, m.thumb_unselected_size);
  ASSERT_EQ(1.0f, m.thumb_unselected_border);
  ASSERT_EQ(24.0f, m.thumb_selected_size);
  ASSERT_EQ(28.0f, m.thumb_pressed_size);
  ASSERT_EQ(CMP_M3_SHAPE_FULL, m.shape);
  PASS();
}

TEST test_m3_slider_resolve(void) {
  cmp_m3_slider_metrics_t m;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_m3_slider_resolve(NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_slider_resolve(&m));
  ASSERT_EQ(4.0f, m.track_height_inactive);
  ASSERT_EQ(16.0f, m.track_height_active);
  ASSERT_EQ(20.0f, m.handle_size);
  ASSERT_EQ(48.0f, m.touch_target_size);
  ASSERT_EQ(CMP_M3_SHAPE_FULL, m.shape);
  PASS();
}

SUITE(cmp_material3_selection_controls_suite) {
  RUN_TEST(test_m3_checkbox_resolve);
  RUN_TEST(test_m3_radio_resolve);
  RUN_TEST(test_m3_switch_resolve);
  RUN_TEST(test_m3_slider_resolve);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_selection_controls_suite);
  GREATEST_MAIN_END();
}