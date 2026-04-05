#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_f2_theme.h"
/* clang-format on */

TEST test_f2_neutral_palette(void) {
  cmp_f2_theme_t theme;
  int res;

  res = cmp_f2_theme_generate(0x0078D4, 0, &theme);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Verify some expected grayscale boundaries */
  ASSERT(theme.neutral_10 < theme.neutral_100);
  ASSERT(theme.neutral_50 > theme.neutral_40);
  ASSERT(theme.neutral_100 == 0xFFFFFF);

  /* Verify brand ramps */
  ASSERT_EQ(0x0078D4, theme.brand.step_80);
  ASSERT(theme.brand.step_10 < theme.brand.step_20);   /* Shade comparison */
  ASSERT(theme.brand.step_160 > theme.brand.step_150); /* Tint comparison */

  /* Validate color mixing */
  /* Mix Black (0) with Brand (0x0078D4) at 50% = 0x003C6A */
  ASSERT_EQ(0x003C6A, theme.brand.step_40);

  /* Verify shared palettes */
  ASSERT_EQ(0xD13438, theme.red.step_80);
  ASSERT_EQ(0x00B7C3, theme.light_teal.step_80);
  ASSERT_EQ(0xE43BA6, theme.pink.step_80);

  PASS();
}

TEST test_f2_semantic_tokens(void) {
  cmp_f2_theme_t theme_light;
  cmp_f2_theme_t theme_dark;
  int res;

  res = cmp_f2_theme_generate(0x0078D4, 0, &theme_light);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_theme_generate(0x0078D4, 1, &theme_dark);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Light mode checks */
  ASSERT_EQ(theme_light.neutral_100, theme_light.color_neutral_background_1);
  ASSERT_EQ(theme_light.neutral_10, theme_light.color_neutral_foreground_1);
  ASSERT_EQ(theme_light.brand.step_80, theme_light.color_brand_background);

  /* Dark mode checks */
  ASSERT_EQ(theme_dark.neutral_10, theme_dark.color_neutral_background_1);
  ASSERT_EQ(theme_dark.neutral_100, theme_dark.color_neutral_foreground_1);
  ASSERT_EQ(theme_dark.brand.step_100, theme_dark.color_brand_background);

  /* Status Tokens */
  ASSERT_EQ(theme_light.dark_green.step_160,
            theme_light.color_success_background);
  ASSERT_EQ(theme_dark.dark_green.step_30, theme_dark.color_success_background);

  /* Alpha Tokens */
  ASSERT_EQ(((uint32_t)0x80 << 24) | theme_light.neutral_90,
            theme_light.color_neutral_background_alpha);
  ASSERT_EQ(((uint32_t)0x80 << 24) | theme_dark.neutral_12,
            theme_dark.color_neutral_background_alpha);

  PASS();
}

TEST test_f2_high_contrast(void) {
  cmp_f2_theme_t theme_hc;
  int res;

  /* Mock HC system colors (e.g. Windows High Contrast Black) */
  uint32_t window = 0x000000;
  uint32_t window_text = 0xFFFFFF;
  uint32_t highlight = 0x3399FF;
  uint32_t highlight_text = 0xFFFFFF;
  uint32_t button_face = 0x000000;
  uint32_t button_text = 0xFFFFFF;
  uint32_t gray_text = 0x00FF00;

  res =
      cmp_f2_theme_generate_hc(window, window_text, highlight, highlight_text,
                               button_face, button_text, gray_text, &theme_hc);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Verify strictly mapped HC tokens */
  ASSERT_EQ(window, theme_hc.color_neutral_background_1);
  ASSERT_EQ(window_text, theme_hc.color_neutral_foreground_1);
  ASSERT_EQ(gray_text, theme_hc.color_neutral_foreground_4);
  ASSERT_EQ(highlight, theme_hc.color_brand_background);
  ASSERT_EQ(highlight_text, theme_hc.color_brand_foreground_1);

  /* Alpha should be 0xFF opaque */
  ASSERT_EQ(((uint32_t)0xFF << 24) | window,
            theme_hc.color_neutral_background_alpha);

  PASS();
}

TEST test_f2_typography_tokens(void) {
  cmp_f2_theme_t theme;
  int res;

  res = cmp_f2_theme_generate(0x0078D4, 0, &theme);
  ASSERT_EQ(CMP_SUCCESS, res);

  ASSERT(theme.font_family_base != NULL);
  ASSERT(theme.font_family_monospace != NULL);

  ASSERT_EQ(400, theme.font_weight_regular);
  ASSERT_EQ(700, theme.font_weight_bold);

  ASSERT_EQ(12.0f, theme.font_size_caption_1);
  ASSERT_EQ(14.0f, theme.font_size_body_1);
  ASSERT_EQ(28.0f, theme.font_size_title_1);
  ASSERT_EQ(68.0f, theme.font_size_display);

  ASSERT(theme.letter_spacing_caption_2 > 0.0f);
  ASSERT(theme.letter_spacing_display < 0.0f);

  ASSERT(theme.font_family_optical_display != NULL);
  ASSERT(theme.font_family_optical_text != NULL);
  ASSERT(theme.font_family_optical_small != NULL);

  PASS();
}

TEST test_f2_shadows_and_materials(void) {
  cmp_f2_theme_t theme_light;
  cmp_f2_theme_t theme_dark;
  cmp_f2_theme_t theme_hc;
  int res;

  res = cmp_f2_theme_generate(0x0078D4, 0, &theme_light);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_theme_generate(0x0078D4, 1, &theme_dark);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_theme_generate_hc(0x000000, 0xFFFFFF, 0x3399FF, 0xFFFFFF,
                                 0x000000, 0xFFFFFF, 0x00FF00, &theme_hc);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Light shadows */
  ASSERT_EQ(1.0f, theme_light.shadow_2.y1);
  ASSERT_EQ(2.0f, theme_light.shadow_2.blur1);
  ASSERT_EQ(32.0f, theme_light.shadow_64.y1);
  ASSERT_EQ(64.0f, theme_light.shadow_64.blur1);

  /* Dark shadows (usually suppressed color-wise, but present for structure) */
  ASSERT_EQ(1.0f, theme_dark.shadow_2.y1);
  ASSERT_EQ(0x1A000000, theme_dark.shadow_2.color1); /* 10% black */

  /* HC shadows are completely suppressed */
  ASSERT_EQ(0.0f, theme_hc.shadow_2.y1);
  ASSERT_EQ(0.0f, theme_hc.shadow_2.blur1);
  ASSERT_EQ(0x00000000, theme_hc.shadow_2.color1);
  ASSERT_EQ(0.0f, theme_hc.shadow_64.y1);

  /* Materials */
  ASSERT_EQ(((uint32_t)0xFF << 24) | theme_light.neutral_90,
            theme_light.material_mica_background);
  ASSERT_EQ(30.0f, theme_light.material_acrylic_blur);

  ASSERT_EQ(((uint32_t)0xFF << 24) | theme_dark.neutral_10,
            theme_dark.material_mica_background);
  ASSERT_EQ(30.0f, theme_dark.material_acrylic_blur);

  /* HC Materials fallback to window colors */
  ASSERT_EQ(((uint32_t)0xFF << 24) | 0x000000,
            theme_hc.material_mica_background);
  ASSERT_EQ(0.0f, theme_hc.material_acrylic_blur);

  PASS();
}

TEST test_f2_spacing_and_radii(void) {
  cmp_f2_theme_t theme;
  cmp_f2_theme_t theme_hc;
  int res;

  res = cmp_f2_theme_generate(0x0078D4, 0, &theme);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_theme_generate_hc(0, 0, 0, 0, 0, 0, 0, &theme_hc);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Spacing */
  ASSERT_EQ(0.0f, theme.spacing_none);
  ASSERT_EQ(4.0f, theme.spacing_xs);
  ASSERT_EQ(16.0f, theme.spacing_l);

  /* Radius */
  ASSERT_EQ(0.0f, theme.radius_none);
  ASSERT_EQ(4.0f, theme.radius_medium);
  ASSERT_EQ(9999.0f, theme.radius_circular);

  /* Standard Stroke */
  ASSERT_EQ(1.0f, theme.stroke_width_thin);
  ASSERT_EQ(1.0f, theme.focus_ring_inner_width);

  /* HC Stroke increases for visibility */
  ASSERT_EQ(2.0f, theme_hc.stroke_width_thin);
  ASSERT_EQ(2.0f, theme_hc.focus_ring_inner_width);

  PASS();
}

TEST test_f2_motion_animation(void) {
  cmp_f2_theme_t theme;
  int res;

  res = cmp_f2_theme_generate(0x0078D4, 0, &theme);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Durations */
  ASSERT_EQ(50.0f, theme.duration_ultra_fast);
  ASSERT_EQ(500.0f, theme.duration_slower);

  /* Easing */
  ASSERT_EQ(1.0f, theme.ease_linear[2]);
  ASSERT_EQ(0.33f, theme.ease_standard[0]);
  ASSERT_EQ(1.0f, theme.ease_standard[3]);

  PASS();
}

SUITE(cmp_f2_theme_suite) {
  RUN_TEST(test_f2_neutral_palette);
  RUN_TEST(test_f2_semantic_tokens);
  RUN_TEST(test_f2_high_contrast);
  RUN_TEST(test_f2_typography_tokens);
  RUN_TEST(test_f2_shadows_and_materials);
  RUN_TEST(test_f2_spacing_and_radii);
  RUN_TEST(test_f2_motion_animation);
}
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_f2_theme_suite);
  GREATEST_MAIN_END();
}
