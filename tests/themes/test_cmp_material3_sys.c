/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_sys.h"
#include "greatest.h"
/* clang-format on */

TEST test_m3_sys_colors(void) {
  cmp_m3_sys_colors_t colors;
  cmp_color_t seed;
  seed.r = 1.0f;
  seed.g = 0.0f;
  seed.b = 0.0f;
  seed.a = 1.0f;
  seed.space = CMP_COLOR_SPACE_SRGB;

  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_m3_sys_colors_generate(seed, 0, CMP_M3_CONTRAST_STANDARD, NULL));

  /* Light Standard */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_sys_colors_generate(
                             seed, 0, CMP_M3_CONTRAST_STANDARD, &colors));

  /* Light Medium */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_sys_colors_generate(
                             seed, 0, CMP_M3_CONTRAST_MEDIUM, &colors));

  /* Light High */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_sys_colors_generate(seed, 0, CMP_M3_CONTRAST_HIGH, &colors));

  /* Dark Standard */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_sys_colors_generate(
                             seed, 1, CMP_M3_CONTRAST_STANDARD, &colors));

  /* Dark Medium */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_sys_colors_generate(
                             seed, 1, CMP_M3_CONTRAST_MEDIUM, &colors));

  /* Dark High */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_sys_colors_generate(seed, 1, CMP_M3_CONTRAST_HIGH, &colors));

  PASS();
}

TEST test_m3_shape_resolve(void) {
  float tl, tr, bl, br;
  cmp_m3_shape_modifiers_t mods;

  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_m3_shape_resolve(CMP_M3_SHAPE_SMALL, NULL, NULL, &tr, &bl, &br));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_shape_resolve(CMP_M3_SHAPE_NONE, NULL, &tl, &tr, &bl, &br));
  ASSERT_EQ(0.0f, tl);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_shape_resolve(CMP_M3_SHAPE_EXTRA_SMALL, NULL,
                                              &tl, &tr, &bl, &br));
  ASSERT_EQ(4.0f, tl);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_shape_resolve(CMP_M3_SHAPE_SMALL, NULL, &tl, &tr, &bl, &br));
  ASSERT_EQ(8.0f, tl);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_shape_resolve(CMP_M3_SHAPE_MEDIUM, NULL, &tl,
                                              &tr, &bl, &br));
  ASSERT_EQ(12.0f, tl);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_shape_resolve(CMP_M3_SHAPE_LARGE, NULL, &tl, &tr, &bl, &br));
  ASSERT_EQ(16.0f, tl);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_shape_resolve(CMP_M3_SHAPE_EXTRA_LARGE, NULL,
                                              &tl, &tr, &bl, &br));
  ASSERT_EQ(28.0f, tl);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_shape_resolve(CMP_M3_SHAPE_FULL, NULL, &tl, &tr, &bl, &br));
  ASSERT_EQ(9999.0f, tl);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_shape_resolve((cmp_m3_shape_family_t)999, NULL, &tl, &tr,
                                 &bl, &br));

  mods.top_left_override = 0;
  mods.top_right_override = 2;
  mods.bottom_left_override = 4;
  mods.bottom_right_override = 6;
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_shape_resolve(CMP_M3_SHAPE_MEDIUM, &mods, &tl,
                                              &tr, &bl, &br));
  ASSERT_EQ(0.0f, tl);
  ASSERT_EQ(2.0f, tr);
  ASSERT_EQ(4.0f, bl);
  ASSERT_EQ(6.0f, br);

  mods.top_left_override = -1;
  mods.top_right_override = -1;
  mods.bottom_left_override = -1;
  mods.bottom_right_override = -1;
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_shape_resolve(CMP_M3_SHAPE_MEDIUM, &mods, &tl,
                                              &tr, &bl, &br));
  ASSERT_EQ(12.0f, tl);
  ASSERT_EQ(12.0f, tr);
  ASSERT_EQ(12.0f, bl);
  ASSERT_EQ(12.0f, br);

  PASS();
}

TEST test_m3_elevation_resolve(void) {
  float tonal, y_offset, blur, ambient, spot;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_elevation_resolve(CMP_M3_ELEVATION_LEVEL_0, NULL, &y_offset,
                                     &blur, &ambient, &spot));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_elevation_resolve(CMP_M3_ELEVATION_LEVEL_0, &tonal,
                                     &y_offset, &blur, &ambient, &spot));
  ASSERT_EQ(0.0f, tonal);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_elevation_resolve(CMP_M3_ELEVATION_LEVEL_1, &tonal,
                                     &y_offset, &blur, &ambient, &spot));
  ASSERT_EQ(0.05f, tonal);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_elevation_resolve(CMP_M3_ELEVATION_LEVEL_2, &tonal,
                                     &y_offset, &blur, &ambient, &spot));
  ASSERT_EQ(0.08f, tonal);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_elevation_resolve(CMP_M3_ELEVATION_LEVEL_3, &tonal,
                                     &y_offset, &blur, &ambient, &spot));
  ASSERT_EQ(0.11f, tonal);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_elevation_resolve(CMP_M3_ELEVATION_LEVEL_4, &tonal,
                                     &y_offset, &blur, &ambient, &spot));
  ASSERT_EQ(0.12f, tonal);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_elevation_resolve(CMP_M3_ELEVATION_LEVEL_5, &tonal,
                                     &y_offset, &blur, &ambient, &spot));
  ASSERT_EQ(0.14f, tonal);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_elevation_resolve((cmp_m3_elevation_level_t)999, &tonal,
                                     &y_offset, &blur, &ambient, &spot));

  PASS();
}

TEST test_m3_state_layer_resolve(void) {
  float opacity;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_state_layer_resolve(CMP_M3_STATE_HOVER, NULL));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_state_layer_resolve(CMP_M3_STATE_HOVER, &opacity));
  ASSERT_EQ(0.08f, opacity);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_state_layer_resolve(CMP_M3_STATE_FOCUS, &opacity));
  ASSERT_EQ(0.10f, opacity);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_state_layer_resolve(CMP_M3_STATE_PRESSED, &opacity));
  ASSERT_EQ(0.10f, opacity);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_state_layer_resolve(CMP_M3_STATE_DRAGGED, &opacity));
  ASSERT_EQ(0.16f, opacity);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_state_layer_resolve(
                             CMP_M3_STATE_DISABLED_CONTAINER, &opacity));
  ASSERT_EQ(0.12f, opacity);

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_state_layer_resolve(
                             CMP_M3_STATE_DISABLED_CONTENT, &opacity));
  ASSERT_EQ(0.38f, opacity);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_m3_state_layer_resolve((cmp_m3_state_layer_t)999, &opacity));

  PASS();
}

SUITE(cmp_material3_sys_suite) {
  RUN_TEST(test_m3_sys_colors);
  RUN_TEST(test_m3_shape_resolve);
  RUN_TEST(test_m3_elevation_resolve);
  RUN_TEST(test_m3_state_layer_resolve);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_sys_suite);
  GREATEST_MAIN_END();
}