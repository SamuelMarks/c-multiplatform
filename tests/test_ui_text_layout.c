/* clang-format off */
#include "../include/ui_text_layout.h"
#include "../include/ui_error.h"
#include "../include/ui_font_manager.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
int g_mock_font_fail = 0;
int g_mock_glyph_metrics_fail = 0;

#define ACCUM_ERR(failed, expr) failed |= ((expr) != UI_ERROR_NONE)
#define ACCUM_FAIL(failed, expr) failed |= (expr)

/* Override ui_font functions */
ui_error_t ui_font_get_vmetrics(struct ui_font *font, float font_size,
                                float *out_ascent, float *out_descent,
                                float *out_line_gap) {
  (void)font;
  (void)font_size;
  if (g_mock_font_fail)
    return UI_ERROR_UNKNOWN;
  *out_ascent = 10.0f;
  *out_descent = 2.0f;
  *out_line_gap = 1.0f;
  return UI_ERROR_NONE;
}

ui_error_t ui_font_get_glyph_metrics(struct ui_font *font, int codepoint,
                                     float font_size,
                                     struct ui_glyph_metrics *out_metrics) {
  (void)font;
  (void)font_size;
  if (g_mock_glyph_metrics_fail == 1 ||
      (g_mock_glyph_metrics_fail == 2 && codepoint == 'B')) {
    return UI_ERROR_UNKNOWN;
  }
  out_metrics->width = 10;
  out_metrics->height = 10;
  out_metrics->advance = 12.0f;
  return UI_ERROR_NONE;
}

ui_error_t ui_font_get_kerning(struct ui_font *font, int prev_codepoint,
                               int codepoint, float font_size,
                               float *out_kerning) {
  (void)font;
  (void)prev_codepoint;
  (void)codepoint;
  (void)font_size;
  if (out_kerning)
    *out_kerning = 0.0f;
  return UI_ERROR_NONE;
}

static int test_text_layout(void) {
  struct ui_text_layout *layout = NULL;
  struct ui_font *font = (struct ui_font *)1; /* Dummy pointer */
  const struct ui_positioned_glyph *glyphs = NULL;
  size_t count = 0;
  float w, h;
  const char *utf8_text = "Hello\nWorld";
  const char *utf8_complex = "\xC2\xA2 \xE2\x82\xAC \xF0\x90\x8D\x88";
  const char *long_string =
      "1234567890123456789012345678901234567890"; /* 40 chars */
  int failed = 0;

  ACCUM_ERR(failed, ui_text_layout_create(&layout));
  ACCUM_ERR(failed, ui_text_layout_destroy(layout));

  ACCUM_ERR(failed, ui_text_layout_create(&layout));

  ACCUM_ERR(failed, ui_text_layout_shape(layout, font, 12.0f, utf8_text, 100.0f,
                                         UI_TEXT_DIRECTION_LTR));
  ACCUM_ERR(failed, ui_text_layout_shape(layout, font, 12.0f, utf8_complex,
                                         5.0f, UI_TEXT_DIRECTION_LTR));

  /* Invalid/incomplete UTF-8 */
  const char *utf8_invalid = "\xC2\xE2\x82\xF0\x90\x8D";
  ACCUM_ERR(failed, ui_text_layout_shape(layout, font, 12.0f, utf8_invalid,
                                         100.0f, UI_TEXT_DIRECTION_LTR));
  const char *utf8_inc_1 = "\xC2";
  ACCUM_ERR(failed, ui_text_layout_shape(layout, font, 12.0f, utf8_inc_1,
                                         100.0f, UI_TEXT_DIRECTION_LTR));
  const char *utf8_inc_2 = "\xE2";
  ACCUM_ERR(failed, ui_text_layout_shape(layout, font, 12.0f, utf8_inc_2,
                                         100.0f, UI_TEXT_DIRECTION_LTR));
  const char *utf8_inc_3 = "\xE2\x82";
  ACCUM_ERR(failed, ui_text_layout_shape(layout, font, 12.0f, utf8_inc_3,
                                         100.0f, UI_TEXT_DIRECTION_LTR));
  const char *utf8_inc_4 = "\xF0";
  ACCUM_ERR(failed, ui_text_layout_shape(layout, font, 12.0f, utf8_inc_4,
                                         100.0f, UI_TEXT_DIRECTION_LTR));
  const char *utf8_inc_5 = "\xF0\x90";
  ACCUM_ERR(failed, ui_text_layout_shape(layout, font, 12.0f, utf8_inc_5,
                                         100.0f, UI_TEXT_DIRECTION_LTR));
  const char *utf8_inc_6 = "\xF0\x90\x8D";
  ACCUM_ERR(failed, ui_text_layout_shape(layout, font, 12.0f, utf8_inc_6,
                                         100.0f, UI_TEXT_DIRECTION_LTR));

  /* Test reallocation */
  ACCUM_ERR(failed, ui_text_layout_shape(layout, font, 12.0f, long_string,
                                         1000.0f, UI_TEXT_DIRECTION_LTR));

  /* Test zero max_width */
  ACCUM_ERR(failed, ui_text_layout_shape(layout, font, 12.0f, utf8_text, 0.0f,
                                         UI_TEXT_DIRECTION_LTR));

  /* Test font metrics failure */
  g_mock_font_fail = 1;
  failed |= (ui_text_layout_shape(layout, font, 12.0f, "A", 100.0f,
                                  UI_TEXT_DIRECTION_LTR) == UI_ERROR_NONE);
  g_mock_font_fail = 0;

  /* Test glyph metrics failure for codepoint 'B' */
  g_mock_glyph_metrics_fail = 2;
  ACCUM_ERR(failed, ui_text_layout_shape(layout, font, 12.0f, "ABC", 100.0f,
                                         UI_TEXT_DIRECTION_LTR));
  g_mock_glyph_metrics_fail = 0;

  ACCUM_ERR(failed, ui_text_layout_get_glyphs(layout, &glyphs, &count));
  ACCUM_ERR(failed, ui_text_layout_get_bounds(layout, &w, &h));

  (void)ui_text_layout_destroy(layout);

  failed |= (ui_text_layout_create(NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_text_layout_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_text_layout_shape(NULL, font, 12.0f, utf8_text, 100.0f,
                                  UI_TEXT_DIRECTION_LTR) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_text_layout_shape(layout, NULL, 12.0f, utf8_text, 100.0f,
                                  UI_TEXT_DIRECTION_LTR) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_text_layout_shape(layout, font, 12.0f, NULL, 100.0f,
                                  UI_TEXT_DIRECTION_LTR) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_text_layout_get_glyphs(NULL, &glyphs, &count) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_text_layout_get_glyphs(layout, NULL, &count) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_text_layout_get_glyphs(layout, &glyphs, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_text_layout_get_bounds(NULL, &w, &h) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_text_layout_get_bounds(layout, NULL, &h) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_text_layout_get_bounds(layout, &w, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);

  g_malloc_fail_countdown = 0;
  failed |= (ui_text_layout_create(&layout) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  /* malloc fails during add_glyph */
  ui_text_layout_create(&layout);
  g_malloc_fail_countdown = 0;
  failed |=
      (ui_text_layout_shape(layout, font, 12.0f, "A", 100.0f,
                            UI_TEXT_DIRECTION_LTR) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  /* malloc fails during add_glyph reallocation */
  g_malloc_fail_countdown =
      1; /* first add_glyph allocs array, second fails? Wait, count > cap check.
            cap = 32. So we need countdown = 1, but we trigger it by shaping
            long string. */
  failed |=
      (ui_text_layout_shape(layout, font, 12.0f, long_string, 100.0f,
                            UI_TEXT_DIRECTION_LTR) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  (void)ui_text_layout_destroy(layout);

  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_text_layout();
  if (!failed) {
    printf("test_ui_text_layout passed\n");
  }
  return failed;
}
