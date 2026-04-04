/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_layout.h"
#include "greatest.h"
/* clang-format on */

TEST test_m3_window_size_class_resolve(void) {
  cmp_m3_window_size_class_t size_class;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_window_size_class_resolve(-1.0f, &size_class));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_window_size_class_resolve(500.0f, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_window_size_class_resolve(0.0f, &size_class));
  ASSERT_EQ(CMP_M3_WINDOW_CLASS_COMPACT, size_class);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_window_size_class_resolve(599.9f, &size_class));
  ASSERT_EQ(CMP_M3_WINDOW_CLASS_COMPACT, size_class);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_window_size_class_resolve(600.0f, &size_class));
  ASSERT_EQ(CMP_M3_WINDOW_CLASS_MEDIUM, size_class);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_window_size_class_resolve(839.9f, &size_class));
  ASSERT_EQ(CMP_M3_WINDOW_CLASS_MEDIUM, size_class);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_window_size_class_resolve(840.0f, &size_class));
  ASSERT_EQ(CMP_M3_WINDOW_CLASS_EXPANDED, size_class);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_window_size_class_resolve(1199.9f, &size_class));
  ASSERT_EQ(CMP_M3_WINDOW_CLASS_EXPANDED, size_class);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_window_size_class_resolve(1200.0f, &size_class));
  ASSERT_EQ(CMP_M3_WINDOW_CLASS_LARGE, size_class);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_window_size_class_resolve(9999.0f, &size_class));
  ASSERT_EQ(CMP_M3_WINDOW_CLASS_LARGE, size_class);

  PASS();
}

TEST test_m3_window_layout_config_get(void) {
  cmp_m3_window_layout_config_t config;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_window_layout_config_get(CMP_M3_WINDOW_CLASS_COMPACT, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_window_layout_config_get(
                             CMP_M3_WINDOW_CLASS_COMPACT, &config));
  ASSERT_EQ(4, config.columns);
  ASSERT_EQ(16.0f, config.margins);
  ASSERT_EQ(16.0f, config.gutters);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_window_layout_config_get(
                             CMP_M3_WINDOW_CLASS_MEDIUM, &config));
  ASSERT_EQ(8, config.columns);
  ASSERT_EQ(24.0f, config.margins);
  ASSERT_EQ(24.0f, config.gutters);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_window_layout_config_get(
                             CMP_M3_WINDOW_CLASS_EXPANDED, &config));
  ASSERT_EQ(12, config.columns);
  ASSERT_EQ(24.0f, config.margins);
  ASSERT_EQ(24.0f, config.gutters);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_window_layout_config_get(
                             CMP_M3_WINDOW_CLASS_LARGE, &config));
  ASSERT_EQ(12, config.columns);
  ASSERT_EQ(24.0f, config.margins);
  ASSERT_EQ(24.0f, config.gutters);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_window_layout_config_get((cmp_m3_window_size_class_t)999,
                                            &config));

  PASS();
}

TEST test_m3_layout_pattern_resolve(void) {
  cmp_m3_layout_pattern_t pattern;

  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_m3_layout_pattern_resolve(CMP_M3_WINDOW_CLASS_COMPACT, 0, 0, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_layout_pattern_resolve(
                             CMP_M3_WINDOW_CLASS_COMPACT, 1, 0, &pattern));
  ASSERT_EQ(CMP_M3_PATTERN_FEED, pattern);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_layout_pattern_resolve(
                             CMP_M3_WINDOW_CLASS_EXPANDED, 0, 1, &pattern));
  ASSERT_EQ(CMP_M3_PATTERN_SUPPORTING_PANE, pattern);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_layout_pattern_resolve(
                             CMP_M3_WINDOW_CLASS_COMPACT, 0, 0, &pattern));
  ASSERT_EQ(CMP_M3_PATTERN_LIST_DETAIL_STACK, pattern);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_layout_pattern_resolve(
                             CMP_M3_WINDOW_CLASS_MEDIUM, 0, 0, &pattern));
  ASSERT_EQ(CMP_M3_PATTERN_LIST_DETAIL_STACK, pattern);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_layout_pattern_resolve(
                             CMP_M3_WINDOW_CLASS_EXPANDED, 0, 0, &pattern));
  ASSERT_EQ(CMP_M3_PATTERN_LIST_DETAIL_SIDE_BY_SIDE, pattern);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_layout_pattern_resolve(
                             CMP_M3_WINDOW_CLASS_LARGE, 0, 0, &pattern));
  ASSERT_EQ(CMP_M3_PATTERN_LIST_DETAIL_SIDE_BY_SIDE, pattern);

  PASS();
}

TEST test_m3_foldable_posture_resolve(void) {
  cmp_m3_posture_t posture;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_foldable_posture_resolve(0, 0.0f, 0, NULL));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_foldable_posture_resolve(0, 90.0f, 0, &posture));
  ASSERT_EQ(CMP_M3_POSTURE_FLAT, posture);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_foldable_posture_resolve(1, 180.0f, 0, &posture));
  ASSERT_EQ(CMP_M3_POSTURE_FLAT, posture);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_foldable_posture_resolve(1, 5.0f, 0, &posture));
  ASSERT_EQ(CMP_M3_POSTURE_FLAT, posture);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_foldable_posture_resolve(1, 90.0f, 1, &posture));
  ASSERT_EQ(CMP_M3_POSTURE_BOOK, posture);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_foldable_posture_resolve(1, 90.0f, 0, &posture));
  ASSERT_EQ(CMP_M3_POSTURE_TABLETOP, posture);

  PASS();
}

SUITE(cmp_material3_layout_suite) {
  RUN_TEST(test_m3_window_size_class_resolve);
  RUN_TEST(test_m3_window_layout_config_get);
  RUN_TEST(test_m3_layout_pattern_resolve);
  RUN_TEST(test_m3_foldable_posture_resolve);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_layout_suite);
  GREATEST_MAIN_END();
}