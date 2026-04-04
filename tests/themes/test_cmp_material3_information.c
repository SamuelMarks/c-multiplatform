/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_information.h"
#include "greatest.h"
/* clang-format on */

TEST test_m3_chip_resolve(void) {
  cmp_m3_chip_metrics_t m;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_chip_resolve(CMP_M3_CHIP_ASSIST, 0, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_chip_resolve(CMP_M3_CHIP_ASSIST, 0, &m));
  ASSERT_EQ(32.0f, m.height);
  ASSERT_EQ(CMP_M3_SHAPE_SMALL, m.shape);
  ASSERT_EQ(18.0f, m.icon_size);
  ASSERT_EQ(24.0f, m.avatar_size);
  ASSERT_EQ(1.0f, m.border_thickness);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_0, m.elevation);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_chip_resolve(CMP_M3_CHIP_FILTER, 1, &m));
  ASSERT_EQ(0.0f, m.border_thickness);
  ASSERT_EQ(CMP_M3_ELEVATION_LEVEL_1, m.elevation);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_chip_resolve(CMP_M3_CHIP_INPUT, 0, &m));
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_chip_resolve(CMP_M3_CHIP_SUGGESTION, 0, &m));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_chip_resolve((cmp_m3_chip_variant_t)999, 0, &m));
  PASS();
}

TEST test_m3_list_resolve(void) {
  cmp_m3_list_metrics_t m;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_list_resolve(CMP_M3_LIST_ONE_LINE, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_list_resolve(CMP_M3_LIST_ONE_LINE, &m));
  ASSERT_EQ(56.0f, m.height);
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_list_resolve(CMP_M3_LIST_TWO_LINE, &m));
  ASSERT_EQ(72.0f, m.height);
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_list_resolve(CMP_M3_LIST_THREE_LINE, &m));
  ASSERT_EQ(88.0f, m.height);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_list_resolve((cmp_m3_list_variant_t)999, &m));
  PASS();
}

TEST test_m3_carousel_resolve(void) {
  cmp_m3_carousel_metrics_t m;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_carousel_resolve(CMP_M3_CAROUSEL_MULTI_BROWSE, NULL));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_carousel_resolve(CMP_M3_CAROUSEL_MULTI_BROWSE, &m));
  ASSERT_EQ(8.0f, m.item_spacing);
  ASSERT_EQ(CMP_M3_SHAPE_EXTRA_LARGE, m.contained_shape);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_carousel_resolve(CMP_M3_CAROUSEL_HERO, &m));
  ASSERT_EQ(8.0f, m.item_spacing);
  ASSERT_EQ(CMP_M3_SHAPE_EXTRA_LARGE, m.contained_shape);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_carousel_resolve(CMP_M3_CAROUSEL_UNCONTAINED, &m));
  ASSERT_EQ(8.0f, m.item_spacing);
  ASSERT_EQ(CMP_M3_SHAPE_NONE, m.contained_shape);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_carousel_resolve((cmp_m3_carousel_variant_t)999, &m));
  PASS();
}

SUITE(cmp_material3_information_suite) {
  RUN_TEST(test_m3_chip_resolve);
  RUN_TEST(test_m3_list_resolve);
  RUN_TEST(test_m3_carousel_resolve);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_information_suite);
  GREATEST_MAIN_END();
}