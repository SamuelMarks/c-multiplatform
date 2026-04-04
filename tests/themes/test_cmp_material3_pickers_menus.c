/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_pickers_menus.h"
#include "themes/cmp_material3_pickers_menus_ext.h"
#include "greatest.h"
/* clang-format on */

TEST test_m3_menu_resolve(void) {
  cmp_m3_menu_metrics_t m;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_m3_menu_resolve(NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_menu_resolve(&m));
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_2, m.elevation);
  ASSERT_EQ(CMP_M3_SHAPE_EXTRA_SMALL, m.shape);
  PASS();
}

TEST test_m3_sheet_resolve(void) {
  cmp_m3_sheet_metrics_t m;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_sheet_resolve(CMP_M3_SHEET_BOTTOM_STANDARD, NULL));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_sheet_resolve(CMP_M3_SHEET_BOTTOM_STANDARD, &m));
  ASSERT_EQ(0, m.has_scrim);
  ASSERT_EQ(1, m.has_drag_handle);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_1, m.elevation);
  ASSERT_EQ(CMP_M3_SHAPE_EXTRA_LARGE, m.shape);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_sheet_resolve(CMP_M3_SHEET_BOTTOM_MODAL, &m));
  ASSERT_EQ(1, m.has_scrim);
  ASSERT_EQ(1, m.has_drag_handle);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_sheet_resolve(CMP_M3_SHEET_SIDE_STANDARD, &m));
  ASSERT_EQ(0, m.has_scrim);
  ASSERT_EQ(0, m.has_drag_handle);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_sheet_resolve(CMP_M3_SHEET_SIDE_MODAL, &m));
  ASSERT_EQ(1, m.has_scrim);
  ASSERT_EQ(0, m.has_drag_handle);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_sheet_resolve(CMP_M3_SHEET_SIDE_DETACHED, &m));
  ASSERT_EQ(1, m.has_scrim);
  ASSERT_EQ(0, m.has_drag_handle);
  ASSERT_EQ(CMP_M3_SHAPE_LARGE, m.shape);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_sheet_resolve((cmp_m3_sheet_variant_t)999, &m));
  PASS();
}

TEST test_m3_date_picker_resolve(void) {
  cmp_m3_date_picker_metrics_t m;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_date_picker_resolve(CMP_M3_DATE_PICKER_MODAL, NULL));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_date_picker_resolve(CMP_M3_DATE_PICKER_MODAL, &m));
  ASSERT_EQ(40.0f, m.day_selection_size);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_3, m.elevation);
  ASSERT_EQ(CMP_M3_SHAPE_EXTRA_LARGE, m.shape);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_date_picker_resolve(CMP_M3_DATE_PICKER_DOCKED, &m));
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_0, m.elevation);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_date_picker_resolve((cmp_m3_date_picker_variant_t)999, &m));
  PASS();
}

TEST test_m3_time_picker_resolve(void) {
  cmp_m3_time_picker_metrics_t m;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_time_picker_resolve(CMP_M3_TIME_PICKER_DIAL, NULL));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_time_picker_resolve(CMP_M3_TIME_PICKER_DIAL, &m));
  ASSERT_EQ(40.0f, m.am_pm_segment_height);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_3, m.elevation);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_time_picker_resolve(CMP_M3_TIME_PICKER_INPUT, &m));
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_3, m.elevation);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_time_picker_resolve((cmp_m3_time_picker_variant_t)999, &m));
  PASS();
}

SUITE(cmp_material3_pickers_menus_suite) {
  RUN_TEST(test_m3_menu_resolve);
  RUN_TEST(test_m3_sheet_resolve);
  RUN_TEST(test_m3_date_picker_resolve);
  RUN_TEST(test_m3_time_picker_resolve);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_pickers_menus_suite);
  GREATEST_MAIN_END();
}