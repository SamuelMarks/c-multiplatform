/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_containment.h"
#include "greatest.h"
/* clang-format on */

TEST test_m3_card_resolve(void) {
  cmp_m3_card_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_card_resolve(CMP_M3_CARD_ELEVATED, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_card_resolve(CMP_M3_CARD_ELEVATED, &m));
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_1, m.elevation);
  ASSERT_EQ(CMP_M3_SHAPE_MEDIUM, m.shape);
  ASSERT_EQ(0.0f, m.border_thickness);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_card_resolve(CMP_M3_CARD_FILLED, &m));
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_0, m.elevation);
  ASSERT_EQ(CMP_M3_SHAPE_MEDIUM, m.shape);
  ASSERT_EQ(0.0f, m.border_thickness);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_card_resolve(CMP_M3_CARD_OUTLINED, &m));
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_0, m.elevation);
  ASSERT_EQ(CMP_M3_SHAPE_MEDIUM, m.shape);
  ASSERT_EQ(1.0f, m.border_thickness);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_card_resolve((cmp_m3_card_variant_t)999, &m));

  PASS();
}

TEST test_m3_dialog_resolve(void) {
  cmp_m3_dialog_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_dialog_resolve(CMP_M3_DIALOG_BASIC, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_dialog_resolve(CMP_M3_DIALOG_BASIC, &m));
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_3, m.elevation);
  ASSERT_EQ(CMP_M3_SHAPE_EXTRA_LARGE, m.shape);
  ASSERT_EQ(24.0f, m.padding_all);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_dialog_resolve(CMP_M3_DIALOG_FULL_SCREEN, &m));
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_0, m.elevation);
  ASSERT_EQ(CMP_M3_SHAPE_NONE, m.shape);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_dialog_resolve((cmp_m3_dialog_variant_t)999, &m));

  PASS();
}

TEST test_m3_divider_resolve(void) {
  cmp_m3_divider_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_divider_resolve(CMP_M3_DIVIDER_FULL_BLEED, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_divider_resolve(CMP_M3_DIVIDER_FULL_BLEED, &m));
  ASSERT_EQ(1.0f, m.thickness);
  ASSERT_EQ(0.0f, m.inset_start);
  ASSERT_EQ(0.0f, m.inset_end);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_divider_resolve(CMP_M3_DIVIDER_INSET, &m));
  ASSERT_EQ(1.0f, m.thickness);
  ASSERT_EQ(16.0f, m.inset_start);
  ASSERT_EQ(0.0f, m.inset_end);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_divider_resolve(CMP_M3_DIVIDER_MIDDLE_INSET, &m));
  ASSERT_EQ(1.0f, m.thickness);
  ASSERT_EQ(16.0f, m.inset_start);
  ASSERT_EQ(16.0f, m.inset_end);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_divider_resolve((cmp_m3_divider_variant_t)999, &m));

  PASS();
}

TEST test_m3_tooltip_resolve(void) {
  cmp_m3_tooltip_metrics_t m;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_tooltip_resolve(CMP_M3_TOOLTIP_PLAIN, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_tooltip_resolve(CMP_M3_TOOLTIP_PLAIN, &m));
  ASSERT_EQ(24.0f, m.height);
  ASSERT_EQ(8.0f, m.padding_all);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_0, m.elevation);
  ASSERT_EQ(CMP_M3_SHAPE_EXTRA_SMALL, m.shape);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_tooltip_resolve(CMP_M3_TOOLTIP_RICH, &m));
  ASSERT_EQ(0.0f, m.height);
  ASSERT_EQ(16.0f, m.padding_all);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_2, m.elevation);
  ASSERT_EQ(CMP_M3_SHAPE_EXTRA_SMALL, m.shape);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_tooltip_resolve((cmp_m3_tooltip_variant_t)999, &m));

  PASS();
}

SUITE(cmp_material3_containment_suite) {
  RUN_TEST(test_m3_card_resolve);
  RUN_TEST(test_m3_dialog_resolve);
  RUN_TEST(test_m3_divider_resolve);
  RUN_TEST(test_m3_tooltip_resolve);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_containment_suite);
  GREATEST_MAIN_END();
}