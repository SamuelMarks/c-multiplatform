/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_navigation.h"
#include "greatest.h"
/* clang-format on */

TEST test_m3_bottom_app_bar_resolve(void) {
  cmp_m3_bottom_app_bar_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_m3_bottom_app_bar_resolve(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_bottom_app_bar_resolve(&m));
  ASSERT_EQ(80.0f, m.height);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_2, m.elevation);
  ASSERT_EQ(4, m.max_action_items);

  PASS();
}

TEST test_m3_bottom_nav_resolve(void) {
  cmp_m3_bottom_nav_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_m3_bottom_nav_resolve(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_bottom_nav_resolve(&m));
  ASSERT_EQ(80.0f, m.height);
  ASSERT_EQ(64.0f, m.active_indicator_width);
  ASSERT_EQ(32.0f, m.active_indicator_height);
  ASSERT_EQ(CMP_M3_SHAPE_FULL, m.active_indicator_shape);

  PASS();
}

TEST test_m3_drawer_resolve(void) {
  cmp_m3_drawer_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_drawer_resolve(CMP_M3_DRAWER_MODAL, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_drawer_resolve(CMP_M3_DRAWER_MODAL, &m));
  ASSERT_EQ(360.0f, m.max_width);
  ASSERT_EQ(16.0f, m.active_item_padding);
  ASSERT_EQ(CMP_M3_SHAPE_FULL, m.active_item_shape);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_1, m.elevation);
  ASSERT_EQ(CMP_M3_SHAPE_LARGE, m.shape);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_drawer_resolve(CMP_M3_DRAWER_STANDARD, &m));
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_0, m.elevation);
  ASSERT_EQ(CMP_M3_SHAPE_NONE, m.shape);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_drawer_resolve((cmp_m3_drawer_variant_t)999, &m));

  PASS();
}

TEST test_m3_nav_rail_resolve(void) {
  cmp_m3_nav_rail_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_m3_nav_rail_resolve(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_nav_rail_resolve(&m));
  ASSERT_EQ(80.0f, m.width);
  ASSERT_EQ(CMP_M3_SHAPE_FULL, m.active_indicator_shape);

  PASS();
}

TEST test_m3_top_app_bar_resolve(void) {
  cmp_m3_top_app_bar_metrics_t m;

  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_m3_top_app_bar_resolve(CMP_M3_TOP_APP_BAR_CENTER_ALIGNED, NULL));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_top_app_bar_resolve(CMP_M3_TOP_APP_BAR_CENTER_ALIGNED, &m));
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_2, m.elevation_scrolled);
  ASSERT_EQ(64.0f, m.height_collapsed);
  ASSERT_EQ(64.0f, m.height_expanded);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_top_app_bar_resolve(CMP_M3_TOP_APP_BAR_SMALL, &m));
  ASSERT_EQ(64.0f, m.height_collapsed);
  ASSERT_EQ(64.0f, m.height_expanded);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_top_app_bar_resolve(CMP_M3_TOP_APP_BAR_MEDIUM, &m));
  ASSERT_EQ(64.0f, m.height_collapsed);
  ASSERT_EQ(112.0f, m.height_expanded);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_top_app_bar_resolve(CMP_M3_TOP_APP_BAR_LARGE, &m));
  ASSERT_EQ(64.0f, m.height_collapsed);
  ASSERT_EQ(152.0f, m.height_expanded);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_top_app_bar_resolve((cmp_m3_top_app_bar_variant_t)999, &m));

  PASS();
}

TEST test_m3_tabs_resolve(void) {
  cmp_m3_tabs_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_tabs_resolve(CMP_M3_TABS_PRIMARY, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_tabs_resolve(CMP_M3_TABS_PRIMARY, &m));
  ASSERT_EQ(48.0f, m.height);
  ASSERT_EQ(CMP_M3_SHAPE_FULL, m.active_indicator_shape);
  ASSERT_EQ(0.0f, m.active_indicator_thickness);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_tabs_resolve(CMP_M3_TABS_SECONDARY, &m));
  ASSERT_EQ(48.0f, m.height);
  ASSERT_EQ(CMP_M3_SHAPE_NONE, m.active_indicator_shape);
  ASSERT_EQ(2.0f, m.active_indicator_thickness);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_tabs_resolve((cmp_m3_tabs_variant_t)999, &m));

  PASS();
}

SUITE(cmp_material3_navigation_suite) {
  RUN_TEST(test_m3_bottom_app_bar_resolve);
  RUN_TEST(test_m3_bottom_nav_resolve);
  RUN_TEST(test_m3_drawer_resolve);
  RUN_TEST(test_m3_nav_rail_resolve);
  RUN_TEST(test_m3_top_app_bar_resolve);
  RUN_TEST(test_m3_tabs_resolve);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_navigation_suite);
  GREATEST_MAIN_END();
}