/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_color.h"
#include "greatest.h"
/* clang-format on */

TEST test_m3_color_conversion(void) {
  cmp_color_t srgb;
  float hue, chroma, tone;
  srgb.r = 1.0f;
  srgb.g = 0.0f;
  srgb.b = 0.0f;
  srgb.a = 1.0f;

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_srgb_to_hct(&srgb, &hue, &chroma, &tone));
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_hct_to_srgb(hue, chroma, tone, &srgb));

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_p3_to_hct(&srgb, &hue, &chroma, &tone));
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_linear_to_hct(&srgb, &hue, &chroma, &tone));

  PASS();
}

TEST test_m3_tonal_palette(void) {
  cmp_palette_t palette;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_generate_tonal_palette(0.0f, 100.0f, 50.0f, &palette));
  PASS();
}

TEST test_m3_schemes(void) {
  cmp_palette_t palette;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_scheme_tonal_spot(0.0f, 100.0f, 50.0f, 0, &palette));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_scheme_spritz(0.0f, 100.0f, 50.0f, 0, &palette));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_scheme_vibrant(0.0f, 100.0f, 50.0f, 0, &palette));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_scheme_expressive(0.0f, 100.0f, 50.0f, 0, &palette));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_scheme_rainbow(0.0f, 100.0f, 50.0f, 0, &palette));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_scheme_fruit_salad(0.0f, 100.0f, 50.0f, 0, &palette));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_scheme_monochrome(0.0f, 100.0f, 50.0f, 0, &palette));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_scheme_fidelity(0.0f, 100.0f, 50.0f, 0, &palette));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_scheme_content(0.0f, 100.0f, 50.0f, 0, &palette));
  PASS();
}

SUITE(cmp_material3_color_suite) {
  RUN_TEST(test_m3_color_conversion);
  RUN_TEST(test_m3_tonal_palette);
  RUN_TEST(test_m3_schemes);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_color_suite);
  GREATEST_MAIN_END();
}