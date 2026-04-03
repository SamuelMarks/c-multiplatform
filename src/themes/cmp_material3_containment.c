/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_containment.h"
#include <stddef.h>
/* clang-format on */

int cmp_m3_card_resolve(cmp_m3_card_variant_t variant,
                        cmp_m3_card_metrics_t *out_metrics) {
  if (!out_metrics) {
    return CMP_ERROR_INVALID_ARG;
  }

  out_metrics->shape = CMP_M3_SHAPE_MEDIUM; /* 12dp */
  out_metrics->border_thickness = 0.0f;

  switch (variant) {
  case CMP_M3_CARD_ELEVATED:
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_1;
    break;
  case CMP_M3_CARD_FILLED:
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_0;
    break;
  case CMP_M3_CARD_OUTLINED:
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_0;
    out_metrics->border_thickness = 1.0f;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}

int cmp_m3_dialog_resolve(cmp_m3_dialog_variant_t variant,
                          cmp_m3_dialog_metrics_t *out_metrics) {
  if (!out_metrics) {
    return CMP_ERROR_INVALID_ARG;
  }

  out_metrics->padding_all = 24.0f;

  switch (variant) {
  case CMP_M3_DIALOG_BASIC:
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_3;
    out_metrics->shape = CMP_M3_SHAPE_EXTRA_LARGE; /* 28dp */
    break;
  case CMP_M3_DIALOG_FULL_SCREEN:
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_0;
    out_metrics->shape = CMP_M3_SHAPE_NONE;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}

int cmp_m3_divider_resolve(cmp_m3_divider_variant_t variant,
                           cmp_m3_divider_metrics_t *out_metrics) {
  if (!out_metrics) {
    return CMP_ERROR_INVALID_ARG;
  }

  out_metrics->thickness = 1.0f;
  out_metrics->inset_start = 0.0f;
  out_metrics->inset_end = 0.0f;

  switch (variant) {
  case CMP_M3_DIVIDER_FULL_BLEED:
    break;
  case CMP_M3_DIVIDER_INSET:
    out_metrics->inset_start =
        16.0f; /* Or 72dp depending on context, using baseline */
    break;
  case CMP_M3_DIVIDER_MIDDLE_INSET:
    out_metrics->inset_start = 16.0f;
    out_metrics->inset_end = 16.0f;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}

int cmp_m3_tooltip_resolve(cmp_m3_tooltip_variant_t variant,
                           cmp_m3_tooltip_metrics_t *out_metrics) {
  if (!out_metrics) {
    return CMP_ERROR_INVALID_ARG;
  }

  switch (variant) {
  case CMP_M3_TOOLTIP_PLAIN:
    out_metrics->height = 24.0f;
    out_metrics->padding_all = 8.0f;
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_0;
    out_metrics->shape = CMP_M3_SHAPE_EXTRA_SMALL;
    break;
  case CMP_M3_TOOLTIP_RICH:
    out_metrics->height = 0.0f; /* Dynamic */
    out_metrics->padding_all =
        16.0f; /* M3 rich tooltip varies, using baseline */
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_2;
    out_metrics->shape = CMP_M3_SHAPE_EXTRA_SMALL;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}