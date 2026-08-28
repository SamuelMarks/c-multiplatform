/**
 * @file ui_typography_scale.c
 * @brief ui_typography_scale.c implementation.
 */
/* clang-format off */
#include "ui_typography_scale.h"
/* clang-format on */

/**
 * @brief ui_typography_scale_get_metrics.
 * @param scale Parameter scale.
 * @param out_metrics Parameter out_metrics.
 * @return Return value.
 */
ui_error_t
ui_typography_scale_get_metrics(enum ui_typography_scale scale,
                                struct ui_typography_metrics *out_metrics) {
  if (!out_metrics) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Values based loosely on typical modern OS/Material baseline metrics */
  switch (scale) {
  case UI_TYPOGRAPHY_SCALE_DISPLAY_LARGE:
    out_metrics->font_size = 57.0f;
    out_metrics->line_height = 64.0f;
    out_metrics->letter_spacing = -0.25f;
    out_metrics->font_weight = 400.0f;
    break;
  case UI_TYPOGRAPHY_SCALE_DISPLAY_MEDIUM:
    out_metrics->font_size = 45.0f;
    out_metrics->line_height = 52.0f;
    out_metrics->letter_spacing = 0.0f;
    out_metrics->font_weight = 400.0f;
    break;
  case UI_TYPOGRAPHY_SCALE_DISPLAY_SMALL:
    out_metrics->font_size = 36.0f;
    out_metrics->line_height = 44.0f;
    out_metrics->letter_spacing = 0.0f;
    out_metrics->font_weight = 400.0f;
    break;
  case UI_TYPOGRAPHY_SCALE_HEADLINE_LARGE:
    out_metrics->font_size = 32.0f;
    out_metrics->line_height = 40.0f;
    out_metrics->letter_spacing = 0.0f;
    out_metrics->font_weight = 400.0f;
    break;
  case UI_TYPOGRAPHY_SCALE_HEADLINE_MEDIUM:
    out_metrics->font_size = 28.0f;
    out_metrics->line_height = 36.0f;
    out_metrics->letter_spacing = 0.0f;
    out_metrics->font_weight = 400.0f;
    break;
  case UI_TYPOGRAPHY_SCALE_HEADLINE_SMALL:
    out_metrics->font_size = 24.0f;
    out_metrics->line_height = 32.0f;
    out_metrics->letter_spacing = 0.0f;
    out_metrics->font_weight = 400.0f;
    break;
  case UI_TYPOGRAPHY_SCALE_TITLE_LARGE:
    out_metrics->font_size = 22.0f;
    out_metrics->line_height = 28.0f;
    out_metrics->letter_spacing = 0.0f;
    out_metrics->font_weight = 400.0f;
    break;
  case UI_TYPOGRAPHY_SCALE_TITLE_MEDIUM:
    out_metrics->font_size = 16.0f;
    out_metrics->line_height = 24.0f;
    out_metrics->letter_spacing = 0.15f;
    out_metrics->font_weight = 500.0f;
    break;
  case UI_TYPOGRAPHY_SCALE_TITLE_SMALL:
    out_metrics->font_size = 14.0f;
    out_metrics->line_height = 20.0f;
    out_metrics->letter_spacing = 0.1f;
    out_metrics->font_weight = 500.0f;
    break;
  case UI_TYPOGRAPHY_SCALE_LABEL_LARGE:
    out_metrics->font_size = 14.0f;
    out_metrics->line_height = 20.0f;
    out_metrics->letter_spacing = 0.1f;
    out_metrics->font_weight = 500.0f;
    break;
  case UI_TYPOGRAPHY_SCALE_LABEL_MEDIUM:
    out_metrics->font_size = 12.0f;
    out_metrics->line_height = 16.0f;
    out_metrics->letter_spacing = 0.5f;
    out_metrics->font_weight = 500.0f;
    break;
  case UI_TYPOGRAPHY_SCALE_LABEL_SMALL:
    out_metrics->font_size = 11.0f;
    out_metrics->line_height = 16.0f;
    out_metrics->letter_spacing = 0.5f;
    out_metrics->font_weight = 500.0f;
    break;
  case UI_TYPOGRAPHY_SCALE_BODY_LARGE:
    out_metrics->font_size = 16.0f;
    out_metrics->line_height = 24.0f;
    out_metrics->letter_spacing = 0.5f;
    out_metrics->font_weight = 400.0f;
    break;
  case UI_TYPOGRAPHY_SCALE_BODY_MEDIUM:
    out_metrics->font_size = 14.0f;
    out_metrics->line_height = 20.0f;
    out_metrics->letter_spacing = 0.25f;
    out_metrics->font_weight = 400.0f;
    break;
  case UI_TYPOGRAPHY_SCALE_BODY_SMALL:
    out_metrics->font_size = 12.0f;
    out_metrics->line_height = 16.0f;
    out_metrics->letter_spacing = 0.4f;
    out_metrics->font_weight = 400.0f;
    break;
  default:
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return UI_ERROR_NONE;
}
