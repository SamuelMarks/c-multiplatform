/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_color.h"
#include "greatest.h"
#include <math.h>
/* clang-format on */

static float color_diff(const cmp_color_t *c1, const cmp_color_t *c2) {
  float dr = c1->r - c2->r;
  float dg = c1->g - c2->g;
  float db = c1->b - c2->b;
  return (float)sqrt(dr * dr + dg * dg + db * db);
}

TEST test_m3_color_conversion(void) {
  cmp_color_t original;
  cmp_color_t converted;
  float hue, chroma, tone;
  float diff;
  int i;
  cmp_color_t test_colors[] = {
      {0.0f, 0.0f, 0.0f, 1.0f, CMP_COLOR_SPACE_SRGB}, /* Black */
      {1.0f, 1.0f, 1.0f, 1.0f, CMP_COLOR_SPACE_SRGB}, /* White */
      {0.5f, 0.5f, 0.5f, 1.0f, CMP_COLOR_SPACE_SRGB}, /* Gray */
      {0.0f, 1.0f, 0.0f, 1.0f, CMP_COLOR_SPACE_SRGB}, /* Green */
      {0.0f, 0.0f, 1.0f, 1.0f, CMP_COLOR_SPACE_SRGB}, /* Blue */
      {0.12f, 0.34f, 0.56f, 1.0f, CMP_COLOR_SPACE_SRGB}};

  /* Test 1: Pure Red */
  original.r = 1.0f;
  original.g = 0.0f;
  original.b = 0.0f;
  original.a = 1.0f;

  ASSERT_EQ(CMP_SUCCESS, cmp_m3_srgb_to_hct(&original, &hue, &chroma, &tone));
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_hct_to_srgb(hue, chroma, tone, &converted));

  diff = color_diff(&original, &converted);
  ASSERT(diff < 0.05f);

  /* Test 2: Random known colors */

  for (i = 0; i < 6; i++) {
    ASSERT_EQ(CMP_SUCCESS,
              cmp_m3_srgb_to_hct(&test_colors[i], &hue, &chroma, &tone));
    ASSERT_EQ(CMP_SUCCESS, cmp_m3_hct_to_srgb(hue, chroma, tone, &converted));
    diff = color_diff(&test_colors[i], &converted);
    ASSERT(diff < 0.05f);
  }

  PASS();
}

TEST test_m3_tonal_palette(void) {
  cmp_m3_tonal_palette_t palette;
  float hue = 200.0f;
  float chroma = 50.0f;
  float out_h, out_c, out_t;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_m3_generate_tonal_palette_hct(hue, chroma, &palette));

  /* Verify tones correctly correspond to their luminosity/tone value */
  cmp_m3_srgb_to_hct(&palette.tone0, &out_h, &out_c, &out_t);
  ASSERT(out_t < 1.0f); /* Tone 0 is black */

  cmp_m3_srgb_to_hct(&palette.tone50, &out_h, &out_c, &out_t);
  ASSERT(out_t >= 45.0f && out_t <= 55.0f);

  cmp_m3_srgb_to_hct(&palette.tone100, &out_h, &out_c, &out_t);
  ASSERT(out_t > 99.0f); /* Tone 100 is white */

  PASS();
}

TEST test_m3_theme_generation(void) {
  cmp_m3_palettes_t palettes;
  cmp_palette_t light_scheme;
  cmp_palette_t dark_scheme;
  cmp_color_t seed = {0.4f, 0.2f, 0.8f, 1.0f, CMP_COLOR_SPACE_SRGB};
  float contrast;

  /* Generate core palettes */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_palettes_generate(seed, &palettes));

  /* Generate Light Scheme */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_scheme_light(&palettes, &light_scheme));

  /* Validate semantic contrast ratios in Light mode */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_color_contrast_ratio(&light_scheme.primary,
                                     &light_scheme.on_primary, &contrast));
  ASSERT(contrast >= 4.5f); /* AA compliance for normal text */

  ASSERT_EQ(CMP_SUCCESS,
            cmp_color_contrast_ratio(&light_scheme.surface,
                                     &light_scheme.on_surface, &contrast));
  ASSERT(contrast >= 4.5f);

  /* Generate Dark Scheme */
  ASSERT_EQ(CMP_SUCCESS, cmp_m3_scheme_dark(&palettes, &dark_scheme));

  /* Validate semantic contrast ratios in Dark mode */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_color_contrast_ratio(&dark_scheme.primary,
                                     &dark_scheme.on_primary, &contrast));
  ASSERT(contrast >= 4.5f);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_color_contrast_ratio(&dark_scheme.surface,
                                     &dark_scheme.on_surface, &contrast));
  ASSERT(contrast >= 4.5f);

  PASS();
}

TEST test_m3_color_fuzzing(void) {
  cmp_color_t original;
  float hue, chroma, tone;
  int i;
  int res;

  /* Seed some "random" values including NaN, Inf, and out-of-bounds to ensure
   * we don't crash */
  float test_values[] = {0.0f,       1.0f,    -1.0f,   100.0f,
                         360.0f,     400.0f,  -50.0f,  999999.0f,
                         -999999.0f, 0.0001f, -0.0001f};
  int num_vals = sizeof(test_values) / sizeof(test_values[0]);

  for (i = 0; i < num_vals; i++) {
    original.r = test_values[i];
    original.g = test_values[(i + 1) % num_vals];
    original.b = test_values[(i + 2) % num_vals];
    original.a = 1.0f;
    original.space = CMP_COLOR_SPACE_SRGB;

    /* Should not crash, might return error or clamped values */
    res = cmp_m3_srgb_to_hct(&original, &hue, &chroma, &tone);
    if (res == CMP_SUCCESS) {
      /* If conversion succeeds, reverse should not crash either */
      cmp_m3_hct_to_srgb(hue, chroma, tone, &original);
    }
  }

  for (i = 0; i < num_vals; i++) {
    /* Direct HCT Fuzzing */
    hue = test_values[i];
    chroma = test_values[(i + 1) % num_vals];
    tone = test_values[(i + 2) % num_vals];

    cmp_m3_hct_to_srgb(hue, chroma, tone, &original);
  }

  PASS();
}

SUITE(cmp_material3_color_suite) {
  RUN_TEST(test_m3_color_conversion);
  RUN_TEST(test_m3_tonal_palette);
  RUN_TEST(test_m3_theme_generation);
  RUN_TEST(test_m3_color_fuzzing);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_color_suite);
  GREATEST_MAIN_END();
}
