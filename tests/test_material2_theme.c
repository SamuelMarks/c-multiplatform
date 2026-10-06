/**
 * @file test_material2_theme.c
 * @brief Unit tests for Material Design 2 theme.
 */

/* clang-format off */
#include "material2/md2_theme.h"
#include "ui_error.h"
#include <greatest.h>
/* clang-format on */

TEST test_md2_color_palette_init_light(void) {
  struct md2_color_palette palette;
  ui_error_t rc;

  rc = md2_color_palette_init_light(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_color_palette_init_light(&palette);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(0xFF6200EE, palette.primary, "0x%08X");
  ASSERT_EQ_FMT(0xFFFFFFFF, palette.background, "0x%08X");

  PASS();
}

TEST test_md2_color_palette_init_dark(void) {
  struct md2_color_palette palette;
  ui_error_t rc;

  rc = md2_color_palette_init_dark(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_color_palette_init_dark(&palette);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(0xFFBB86FC, palette.primary, "0x%08X");
  ASSERT_EQ_FMT(0xFF121212, palette.background, "0x%08X");

  PASS();
}

TEST test_md2_typography_get_style(void) {
  struct md2_text_style style;
  ui_error_t rc;

  rc = md2_typography_get_style(MD2_TYPOGRAPHY_H1, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  /* Test all switch branches */
  rc = md2_typography_get_style(MD2_TYPOGRAPHY_H1, &style);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(96.0f, style.font_size_sp, "%f");

  rc = md2_typography_get_style(MD2_TYPOGRAPHY_H2, &style);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(60.0f, style.font_size_sp, "%f");

  rc = md2_typography_get_style(MD2_TYPOGRAPHY_H3, &style);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(48.0f, style.font_size_sp, "%f");

  rc = md2_typography_get_style(MD2_TYPOGRAPHY_H4, &style);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(34.0f, style.font_size_sp, "%f");

  rc = md2_typography_get_style(MD2_TYPOGRAPHY_H5, &style);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(24.0f, style.font_size_sp, "%f");

  rc = md2_typography_get_style(MD2_TYPOGRAPHY_H6, &style);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(20.0f, style.font_size_sp, "%f");

  rc = md2_typography_get_style(MD2_TYPOGRAPHY_SUBTITLE1, &style);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(16.0f, style.font_size_sp, "%f");

  rc = md2_typography_get_style(MD2_TYPOGRAPHY_SUBTITLE2, &style);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(14.0f, style.font_size_sp, "%f");

  rc = md2_typography_get_style(MD2_TYPOGRAPHY_BODY1, &style);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(16.0f, style.font_size_sp, "%f");

  rc = md2_typography_get_style(MD2_TYPOGRAPHY_BODY2, &style);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(14.0f, style.font_size_sp, "%f");

  rc = md2_typography_get_style(MD2_TYPOGRAPHY_BUTTON, &style);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(14.0f, style.font_size_sp, "%f");

  rc = md2_typography_get_style(MD2_TYPOGRAPHY_CAPTION, &style);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(12.0f, style.font_size_sp, "%f");

  rc = md2_typography_get_style(MD2_TYPOGRAPHY_OVERLINE, &style);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(10.0f, style.font_size_sp, "%f");

  /* Test default branch */
  rc = md2_typography_get_style((enum md2_typography_style)999, &style);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(10.0f, style.font_size_sp, "%f");

  PASS();
}

TEST test_md2_theme_init_light(void) {
  struct md2_theme theme;
  ui_error_t rc;

  rc = md2_theme_init_light(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_theme_init_light(&theme);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(0, theme.is_dark, "%d");
  ASSERT_EQ_FMT(1.0f, theme.font_scale, "%f");

  PASS();
}

TEST test_md2_theme_init_dark(void) {
  struct md2_theme theme;
  ui_error_t rc;

  rc = md2_theme_init_dark(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_theme_init_dark(&theme);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(1, theme.is_dark, "%d");
  ASSERT_EQ_FMT(1.0f, theme.font_scale, "%f");

  PASS();
}

extern int g_md2_theme_mock_color_init_fail;

TEST test_md2_theme_mock_fail(void) {
  struct md2_theme theme;
  ui_error_t rc;

  g_md2_theme_mock_color_init_fail = 1;
  rc = md2_theme_init_light(&theme);
  ASSERT_EQ_FMT(UI_ERROR_UNKNOWN, rc, "%d");

  rc = md2_theme_init_dark(&theme);
  ASSERT_EQ_FMT(UI_ERROR_UNKNOWN, rc, "%d");
  g_md2_theme_mock_color_init_fail = 0;

  PASS();
}

SUITE(material2_theme_suite) {
  RUN_TEST(test_md2_color_palette_init_light);
  RUN_TEST(test_md2_color_palette_init_dark);
  RUN_TEST(test_md2_typography_get_style);
  RUN_TEST(test_md2_theme_init_light);
  RUN_TEST(test_md2_theme_init_dark);
  RUN_TEST(test_md2_theme_mock_fail);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(material2_theme_suite);
  GREATEST_MAIN_END();
}
