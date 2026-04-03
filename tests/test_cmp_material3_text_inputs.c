/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_text_inputs.h"
#include "greatest.h"
/* clang-format on */

TEST test_m3_text_field_resolve(void) {
  cmp_m3_text_field_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_text_field_resolve(CMP_M3_TEXT_FIELD_FILLED, NULL));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_text_field_resolve(CMP_M3_TEXT_FIELD_FILLED, &m));
  ASSERT_EQ(56.0f, m.container_height_single_line);
  ASSERT_EQ(24.0f, m.icon_size);
  ASSERT_EQ(CMP_M3_SHAPE_EXTRA_SMALL, m.shape);
  ASSERT_EQ(0, m.shape_modifiers.bottom_left_override);
  ASSERT_EQ(0, m.shape_modifiers.bottom_right_override);
  ASSERT_EQ(1.0f, m.border_thickness_unfocused);
  ASSERT_EQ(2.0f, m.border_thickness_focused);
  ASSERT_EQ(1, m.has_floating_label);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_text_field_resolve(CMP_M3_TEXT_FIELD_OUTLINED, &m));
  ASSERT_EQ(56.0f, m.container_height_single_line);
  ASSERT_EQ(CMP_M3_SHAPE_EXTRA_SMALL, m.shape);
  ASSERT_EQ(-1, m.shape_modifiers.bottom_left_override); /* Not overridden */
  ASSERT_EQ(-1, m.shape_modifiers.bottom_right_override);
  ASSERT_EQ(1.0f, m.border_thickness_unfocused);
  ASSERT_EQ(2.0f, m.border_thickness_focused);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_text_field_resolve((cmp_m3_text_field_variant_t)999, &m));

  PASS();
}

SUITE(cmp_material3_text_inputs_suite) { RUN_TEST(test_m3_text_field_resolve); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_text_inputs_suite);
  GREATEST_MAIN_END();
}