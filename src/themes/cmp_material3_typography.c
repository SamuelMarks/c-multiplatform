/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_typography.h"
#include <stddef.h>
/* clang-format on */

int cmp_m3_typography_resolve(cmp_m3_typescale_category_t category,
                              cmp_m3_type_metrics_t *out_metrics) {
  if (!out_metrics) {
    return CMP_ERROR_INVALID_ARG;
  }

  switch (category) {
  case CMP_M3_TYPESCALE_DISPLAY_LARGE:
    out_metrics->font_size = 57.0f;
    out_metrics->line_height = 64.0f;
    out_metrics->tracking = -0.25f;
    out_metrics->weight = CMP_M3_FONT_WEIGHT_REGULAR;
    break;
  case CMP_M3_TYPESCALE_DISPLAY_MEDIUM:
    out_metrics->font_size = 45.0f;
    out_metrics->line_height = 52.0f;
    out_metrics->tracking = 0.0f;
    out_metrics->weight = CMP_M3_FONT_WEIGHT_REGULAR;
    break;
  case CMP_M3_TYPESCALE_DISPLAY_SMALL:
    out_metrics->font_size = 36.0f;
    out_metrics->line_height = 44.0f;
    out_metrics->tracking = 0.0f;
    out_metrics->weight = CMP_M3_FONT_WEIGHT_REGULAR;
    break;
  case CMP_M3_TYPESCALE_HEADLINE_LARGE:
    out_metrics->font_size = 32.0f;
    out_metrics->line_height = 40.0f;
    out_metrics->tracking = 0.0f;
    out_metrics->weight = CMP_M3_FONT_WEIGHT_REGULAR;
    break;
  case CMP_M3_TYPESCALE_HEADLINE_MEDIUM:
    out_metrics->font_size = 28.0f;
    out_metrics->line_height = 36.0f;
    out_metrics->tracking = 0.0f;
    out_metrics->weight = CMP_M3_FONT_WEIGHT_REGULAR;
    break;
  case CMP_M3_TYPESCALE_HEADLINE_SMALL:
    out_metrics->font_size = 24.0f;
    out_metrics->line_height = 32.0f;
    out_metrics->tracking = 0.0f;
    out_metrics->weight = CMP_M3_FONT_WEIGHT_REGULAR;
    break;
  case CMP_M3_TYPESCALE_TITLE_LARGE:
    out_metrics->font_size = 22.0f;
    out_metrics->line_height = 28.0f;
    out_metrics->tracking = 0.0f;
    out_metrics->weight = CMP_M3_FONT_WEIGHT_REGULAR;
    break;
  case CMP_M3_TYPESCALE_TITLE_MEDIUM:
    out_metrics->font_size = 16.0f;
    out_metrics->line_height = 24.0f;
    out_metrics->tracking = 0.15f;
    out_metrics->weight = CMP_M3_FONT_WEIGHT_MEDIUM;
    break;
  case CMP_M3_TYPESCALE_TITLE_SMALL:
    out_metrics->font_size = 14.0f;
    out_metrics->line_height = 20.0f;
    out_metrics->tracking = 0.1f;
    out_metrics->weight = CMP_M3_FONT_WEIGHT_MEDIUM;
    break;
  case CMP_M3_TYPESCALE_BODY_LARGE:
    out_metrics->font_size = 16.0f;
    out_metrics->line_height = 24.0f;
    out_metrics->tracking = 0.5f;
    out_metrics->weight = CMP_M3_FONT_WEIGHT_REGULAR;
    break;
  case CMP_M3_TYPESCALE_BODY_MEDIUM:
    out_metrics->font_size = 14.0f;
    out_metrics->line_height = 20.0f;
    out_metrics->tracking = 0.25f;
    out_metrics->weight = CMP_M3_FONT_WEIGHT_REGULAR;
    break;
  case CMP_M3_TYPESCALE_BODY_SMALL:
    out_metrics->font_size = 12.0f;
    out_metrics->line_height = 16.0f;
    out_metrics->tracking = 0.4f;
    out_metrics->weight = CMP_M3_FONT_WEIGHT_REGULAR;
    break;
  case CMP_M3_TYPESCALE_LABEL_LARGE:
    out_metrics->font_size = 14.0f;
    out_metrics->line_height = 20.0f;
    out_metrics->tracking = 0.1f;
    out_metrics->weight = CMP_M3_FONT_WEIGHT_MEDIUM;
    break;
  case CMP_M3_TYPESCALE_LABEL_MEDIUM:
    out_metrics->font_size = 12.0f;
    out_metrics->line_height = 16.0f;
    out_metrics->tracking = 0.5f;
    out_metrics->weight = CMP_M3_FONT_WEIGHT_MEDIUM;
    break;
  case CMP_M3_TYPESCALE_LABEL_SMALL:
    out_metrics->font_size = 11.0f;
    out_metrics->line_height = 16.0f;
    out_metrics->tracking = 0.5f;
    out_metrics->weight = CMP_M3_FONT_WEIGHT_MEDIUM;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}

int cmp_m3_typography_get_family(int is_brand, const char **out_font_family) {
  if (!out_font_family) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (is_brand) {
    *out_font_family = "Roboto";
  } else {
    *out_font_family = "Roboto";
  }

  return CMP_SUCCESS;
}