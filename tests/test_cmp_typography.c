/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_typography_suite);

TEST test_phase6_typography_stubs(void) {
  cmp_texture_t *tex = NULL;
  cmp_glyph_metrics_t metrics;
  int count;
  int *is_rtl = NULL;
  cmp_font_t font_stub;
  cmp_font_t *font_out = NULL;
  cmp_typography_t *typo = NULL;
  (void)typo;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_freetype_glyph_rasterize("test.ttf", 0, &tex, &metrics));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_freetype_glyph_rasterize(NULL, 0, &tex, &metrics));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_harfbuzz_text_shape(&font_stub, "test", 0, &count));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_harfbuzz_text_shape(NULL, "test", 0, &count));

  ASSERT_EQ(CMP_SUCCESS, cmp_arabic_indic_shape(&font_stub, "test", &count));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_arabic_indic_shape(NULL, "test", &count));

  ASSERT_EQ(CMP_SUCCESS, cmp_bidi_run_split("test", &count, &is_rtl));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_bidi_run_split(NULL, &count, &is_rtl));

  ASSERT_EQ(CMP_SUCCESS, cmp_font_render_sdf(&font_stub, 0, &tex));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_font_render_sdf(NULL, 0, &tex));

  ASSERT_EQ(CMP_SUCCESS, cmp_font_render_msdf(&font_stub, 0, &tex));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_font_render_msdf(NULL, 0, &tex));

  ASSERT_EQ(CMP_SUCCESS, cmp_font_render_subpixel_lcd(&font_stub, 0, &tex));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_font_render_subpixel_lcd(NULL, 0, &tex));

  ASSERT_EQ(CMP_SUCCESS, cmp_font_render_color_emoji(&font_stub, 0, &tex));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_font_render_color_emoji(NULL, 0, &tex));

  ASSERT_EQ(CMP_SUCCESS, cmp_variable_font_axis_interpolate(&font_stub, "wght",
                                                            500.0f, &font_out));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_variable_font_axis_interpolate(
                                       NULL, "wght", 500.0f, &font_out));

  PASS();
}

SUITE(cmp_typography_suite) { RUN_TEST(test_phase6_typography_stubs); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_typography_suite);
  GREATEST_MAIN_END();
}
