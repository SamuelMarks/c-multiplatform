/**
 * @file md2_theme.c
 * @brief Implementation of Material Design 2 colors, typography scale, and
 * theme initialization.
 */

/* clang-format off */
#include "material2/md2_theme.h"
#include <string.h>
/* clang-format on */

ui_error_t md2_color_palette_init_light(struct md2_color_palette *out_palette) {
  if (out_palette == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_palette->primary = 0xFF6200EE;
  out_palette->primary_variant = 0xFF3700B3;
  out_palette->secondary = 0xFF03DAC6;
  out_palette->secondary_variant = 0xFF018786;
  out_palette->background = 0xFFFFFFFF;
  out_palette->surface = 0xFFFFFFFF;
  out_palette->error = 0xFFB00020;
  out_palette->on_primary = 0xFFFFFFFF;
  out_palette->on_secondary = 0xFF000000;
  out_palette->on_background = 0xFF000000;
  out_palette->on_surface = 0xFF000000;
  out_palette->on_error = 0xFFFFFFFF;

  return UI_ERROR_NONE;
}

ui_error_t md2_color_palette_init_dark(struct md2_color_palette *out_palette) {
  if (out_palette == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_palette->primary = 0xFFBB86FC;
  out_palette->primary_variant = 0xFF3700B3;
  out_palette->secondary = 0xFF03DAC6;
  out_palette->secondary_variant = 0xFF03DAC6;
  out_palette->background = 0xFF121212;
  out_palette->surface = 0xFF121212;
  out_palette->error = 0xFFCF6679;
  out_palette->on_primary = 0xFF000000;
  out_palette->on_secondary = 0xFF000000;
  out_palette->on_background = 0xFFFFFFFF;
  out_palette->on_surface = 0xFFFFFFFF;
  out_palette->on_error = 0xFF000000;

  return UI_ERROR_NONE;
}

ui_error_t md2_typography_get_style(enum md2_typography_style style,
                                    struct md2_text_style *out_style) {
  if (out_style == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  switch (style) {
  case MD2_TYPOGRAPHY_H1:
    out_style->font_size_sp = 96.0f;
    out_style->line_height_sp = 112.0f;
    out_style->font_weight = 300;
    out_style->letter_spacing_sp = -1.5f;
    out_style->all_caps = 0;
    break;
  case MD2_TYPOGRAPHY_H2:
    out_style->font_size_sp = 60.0f;
    out_style->line_height_sp = 72.0f;
    out_style->font_weight = 300;
    out_style->letter_spacing_sp = -0.5f;
    out_style->all_caps = 0;
    break;
  case MD2_TYPOGRAPHY_H3:
    out_style->font_size_sp = 48.0f;
    out_style->line_height_sp = 56.0f;
    out_style->font_weight = 400;
    out_style->letter_spacing_sp = 0.0f;
    out_style->all_caps = 0;
    break;
  case MD2_TYPOGRAPHY_H4:
    out_style->font_size_sp = 34.0f;
    out_style->line_height_sp = 40.0f;
    out_style->font_weight = 400;
    out_style->letter_spacing_sp = 0.25f;
    out_style->all_caps = 0;
    break;
  case MD2_TYPOGRAPHY_H5:
    out_style->font_size_sp = 24.0f;
    out_style->line_height_sp = 32.0f;
    out_style->font_weight = 400;
    out_style->letter_spacing_sp = 0.0f;
    out_style->all_caps = 0;
    break;
  case MD2_TYPOGRAPHY_H6:
    out_style->font_size_sp = 20.0f;
    out_style->line_height_sp = 28.0f;
    out_style->font_weight = 500;
    out_style->letter_spacing_sp = 0.15f;
    out_style->all_caps = 0;
    break;
  case MD2_TYPOGRAPHY_SUBTITLE1:
    out_style->font_size_sp = 16.0f;
    out_style->line_height_sp = 24.0f;
    out_style->font_weight = 400;
    out_style->letter_spacing_sp = 0.15f;
    out_style->all_caps = 0;
    break;
  case MD2_TYPOGRAPHY_SUBTITLE2:
    out_style->font_size_sp = 14.0f;
    out_style->line_height_sp = 20.0f;
    out_style->font_weight = 500;
    out_style->letter_spacing_sp = 0.1f;
    out_style->all_caps = 0;
    break;
  case MD2_TYPOGRAPHY_BODY1:
    out_style->font_size_sp = 16.0f;
    out_style->line_height_sp = 24.0f;
    out_style->font_weight = 400;
    out_style->letter_spacing_sp = 0.5f;
    out_style->all_caps = 0;
    break;
  case MD2_TYPOGRAPHY_BODY2:
    out_style->font_size_sp = 14.0f;
    out_style->line_height_sp = 20.0f;
    out_style->font_weight = 400;
    out_style->letter_spacing_sp = 0.25f;
    out_style->all_caps = 0;
    break;
  case MD2_TYPOGRAPHY_BUTTON:
    out_style->font_size_sp = 14.0f;
    out_style->line_height_sp = 20.0f;
    out_style->font_weight = 500;
    out_style->letter_spacing_sp = 1.25f;
    out_style->all_caps = 1;
    break;
  case MD2_TYPOGRAPHY_CAPTION:
    out_style->font_size_sp = 12.0f;
    out_style->line_height_sp = 16.0f;
    out_style->font_weight = 400;
    out_style->letter_spacing_sp = 0.4f;
    out_style->all_caps = 0;
    break;
  case MD2_TYPOGRAPHY_OVERLINE:
  default:
    out_style->font_size_sp = 10.0f;
    out_style->line_height_sp = 16.0f;
    out_style->font_weight = 400;
    out_style->letter_spacing_sp = 1.5f;
    out_style->all_caps = 1;
    break;
  }

  return UI_ERROR_NONE;
}

ui_error_t md2_theme_init_light(struct md2_theme *out_theme) {
  ui_error_t rc;

  if (out_theme == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_theme->is_dark = 0;
  out_theme->font_scale = 1.0f;
  rc = md2_color_palette_init_light(&out_theme->colors);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md2_theme_init_dark(struct md2_theme *out_theme) {
  ui_error_t rc;

  if (out_theme == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_theme->is_dark = 1;
  out_theme->font_scale = 1.0f;
  rc = md2_color_palette_init_dark(&out_theme->colors);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}
