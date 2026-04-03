/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_components.h"
#include "greatest.h"
/* clang-format on */

TEST test_m3_button_resolve(void) {
  cmp_m3_button_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_button_resolve(CMP_M3_BUTTON_ELEVATED, 0, NULL));

  /* Elevated, no icon */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_button_resolve(CMP_M3_BUTTON_ELEVATED, 0, &m));
  ASSERT_EQ(40.0f, m.height);
  ASSERT_EQ(CMP_M3_SHAPE_FULL, m.shape);
  ASSERT_EQ(18.0f, m.icon_size);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_1, m.elevation_base);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_2, m.elevation_hover);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_1, m.elevation_pressed);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_0, m.elevation_disabled);
  ASSERT_EQ(0.0f, m.border_thickness);
  ASSERT_EQ(24.0f, m.padding_left);
  ASSERT_EQ(0.0f, m.gap_with_icon);
  ASSERT_EQ(24.0f, m.padding_right);
  ASSERT_EQ(24.0f, m.padding_leading_with_icon);

  /* Elevated, with leading icon */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_button_resolve(CMP_M3_BUTTON_ELEVATED, 1, &m));
  ASSERT_EQ(16.0f, m.padding_left);
  ASSERT_EQ(8.0f, m.gap_with_icon);
  ASSERT_EQ(24.0f, m.padding_right);

  /* Filled */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_button_resolve(CMP_M3_BUTTON_FILLED, 0, &m));
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_0, m.elevation_base);

  /* Filled Tonal */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_button_resolve(CMP_M3_BUTTON_FILLED_TONAL, 0, &m));
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_0, m.elevation_base);

  /* Outlined */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_button_resolve(CMP_M3_BUTTON_OUTLINED, 0, &m));
  ASSERT_EQ(1.0f, m.border_thickness);

  /* Text */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_button_resolve(CMP_M3_BUTTON_TEXT, 0, &m));
  ASSERT_EQ(12.0f, m.padding_left);
  ASSERT_EQ(0.0f, m.gap_with_icon);
  ASSERT_EQ(12.0f, m.padding_right);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_button_resolve(CMP_M3_BUTTON_TEXT, 1, &m));
  ASSERT_EQ(12.0f, m.padding_left);
  ASSERT_EQ(8.0f, m.gap_with_icon);
  ASSERT_EQ(16.0f, m.padding_right);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_button_resolve((cmp_m3_button_variant_t)999, 0, &m));
  PASS();
}

TEST test_m3_fab_resolve(void) {
  cmp_m3_fab_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_fab_resolve(CMP_M3_FAB_STANDARD, 0, NULL));

  /* Standard */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_fab_resolve(CMP_M3_FAB_STANDARD, 0, &m));
  ASSERT_EQ(56.0f, m.container_width);
  ASSERT_EQ(56.0f, m.container_height);
  ASSERT_EQ(24.0f, m.icon_size);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_3, m.elevation_base);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_4, m.elevation_hover);
  ASSERT_EQ(CMP_M3_SHAPE_LARGE, m.shape);

  /* Standard Lowered */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_fab_resolve(CMP_M3_FAB_STANDARD, 1, &m));
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_1, m.elevation_base);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_2, m.elevation_hover);

  /* Small */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_fab_resolve(CMP_M3_FAB_SMALL, 0, &m));
  ASSERT_EQ(40.0f, m.container_width);
  ASSERT_EQ(40.0f, m.container_height);
  ASSERT_EQ(24.0f, m.icon_size);
  ASSERT_EQ(CMP_M3_SHAPE_MEDIUM, m.shape);

  /* Large */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_fab_resolve(CMP_M3_FAB_LARGE, 0, &m));
  ASSERT_EQ(96.0f, m.container_width);
  ASSERT_EQ(96.0f, m.container_height);
  ASSERT_EQ(36.0f, m.icon_size);
  ASSERT_EQ(CMP_M3_SHAPE_EXTRA_LARGE, m.shape);

  /* Extended */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_fab_resolve(CMP_M3_FAB_EXTENDED, 0, &m));
  ASSERT_EQ(0.0f, m.container_width);
  ASSERT_EQ(56.0f, m.container_height);
  ASSERT_EQ(16.0f, m.padding_left);
  ASSERT_EQ(16.0f, m.padding_right);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_fab_resolve((cmp_m3_fab_variant_t)999, 0, &m));

  PASS();
}

TEST test_m3_icon_button_resolve(void) {
  cmp_m3_icon_button_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_icon_button_resolve(CMP_M3_ICON_BUTTON_STANDARD, NULL));

  /* Standard */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_icon_button_resolve(CMP_M3_ICON_BUTTON_STANDARD, &m));
  ASSERT_EQ(48.0f, m.target_size);
  ASSERT_EQ(40.0f, m.footprint_size);
  ASSERT_EQ(24.0f, m.icon_size);
  ASSERT_EQ(CMP_M3_SHAPE_FULL, m.shape);
  ASSERT_EQ(0.0f, m.border_thickness);

  /* Outlined */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_icon_button_resolve(CMP_M3_ICON_BUTTON_OUTLINED, &m));
  ASSERT_EQ(1.0f, m.border_thickness);

  /* Filled and Tonal check fallthroughs essentially */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_icon_button_resolve(CMP_M3_ICON_BUTTON_FILLED, &m));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_icon_button_resolve(CMP_M3_ICON_BUTTON_FILLED_TONAL, &m));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_icon_button_resolve((cmp_m3_icon_button_variant_t)999, &m));

  PASS();
}

TEST test_m3_segmented_button_resolve(void) {
  cmp_m3_segmented_button_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_m3_segmented_button_resolve(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_segmented_button_resolve(&m));
  ASSERT_EQ(40.0f, m.height);
  ASSERT_EQ(1.0f, m.border_thickness);
  ASSERT_EQ(18.0f, m.checkmark_size);
  ASSERT_EQ(18.0f, m.icon_size);

  PASS();
}

SUITE(cmp_material3_components_suite) {
  RUN_TEST(test_m3_button_resolve);
  RUN_TEST(test_m3_fab_resolve);
  RUN_TEST(test_m3_icon_button_resolve);
  RUN_TEST(test_m3_segmented_button_resolve);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_components_suite);
  GREATEST_MAIN_END();
}