/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_communication.h"
#include "greatest.h"
/* clang-format on */

TEST test_m3_badge_resolve(void) {
  cmp_m3_badge_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_badge_resolve(CMP_M3_BADGE_SMALL, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_badge_resolve(CMP_M3_BADGE_SMALL, &m));
  ASSERT_EQ(6.0f, m.width);
  ASSERT_EQ(6.0f, m.height);
  ASSERT_EQ(0.0f, m.padding_left_right);
  ASSERT_EQ(CMP_M3_SHAPE_FULL, m.shape);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_badge_resolve(CMP_M3_BADGE_LARGE, &m));
  ASSERT_EQ(0.0f, m.width);
  ASSERT_EQ(16.0f, m.height);
  ASSERT_EQ(4.0f, m.padding_left_right);
  ASSERT_EQ(CMP_M3_SHAPE_FULL, m.shape);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_badge_resolve((cmp_m3_badge_variant_t)999, &m));
  PASS();
}

TEST test_m3_progress_resolve(void) {
  cmp_m3_progress_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_progress_resolve(CMP_M3_PROGRESS_LINEAR, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_progress_resolve(CMP_M3_PROGRESS_LINEAR, &m));
  ASSERT_EQ(4.0f, m.track_thickness);
  ASSERT_EQ(0.0f, m.default_size);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_progress_resolve(CMP_M3_PROGRESS_CIRCULAR, &m));
  ASSERT_EQ(4.0f, m.track_thickness);
  ASSERT_EQ(48.0f, m.default_size);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_progress_resolve((cmp_m3_progress_variant_t)999, &m));
  PASS();
}

TEST test_m3_snackbar_resolve(void) {
  cmp_m3_snackbar_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_m3_snackbar_resolve(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_snackbar_resolve(&m));
  ASSERT_EQ(16.0f, m.padding_left_right);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_3, m.elevation);
  ASSERT_EQ(CMP_M3_SHAPE_EXTRA_SMALL, m.shape);
  PASS();
}

TEST test_m3_pull_to_refresh_resolve(void) {
  cmp_m3_pull_to_refresh_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_m3_pull_to_refresh_resolve(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_pull_to_refresh_resolve(&m));
  ASSERT_EQ(48.0f, m.container_size);
  ASSERT_EQ(24.0f, m.indicator_size);
  PASS();
}

SUITE(cmp_material3_communication_suite) {
  RUN_TEST(test_m3_badge_resolve);
  RUN_TEST(test_m3_progress_resolve);
  RUN_TEST(test_m3_snackbar_resolve);
  RUN_TEST(test_m3_pull_to_refresh_resolve);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_communication_suite);
  GREATEST_MAIN_END();
}