/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_information.h"
#include <stddef.h>
/* clang-format on */

int cmp_m3_chip_resolve(cmp_m3_chip_variant_t variant, int is_elevated,
                        cmp_m3_chip_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;

  out_metrics->height = 32.0f;
  out_metrics->shape = CMP_M3_SHAPE_SMALL; /* 8dp */
  out_metrics->icon_size = 18.0f;
  out_metrics->avatar_size = 24.0f;
  out_metrics->border_thickness = is_elevated ? 0.0f : 1.0f;
  out_metrics->elevation =
      is_elevated ? CMP_M3_ELEVATION_LEVEL_1 : CMP_M3_ELEVATION_LEVEL_0;
  out_metrics->padding_left = 8.0f; /* Base padding */
  out_metrics->padding_right = 8.0f;

  switch (variant) {
  case CMP_M3_CHIP_ASSIST:
    break;
  case CMP_M3_CHIP_FILTER:
    break;
  case CMP_M3_CHIP_INPUT:
    break;
  case CMP_M3_CHIP_SUGGESTION:
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}

int cmp_m3_list_resolve(cmp_m3_list_variant_t variant,
                        cmp_m3_list_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;

  out_metrics->padding_left = 16.0f;
  out_metrics->padding_right = 16.0f;

  switch (variant) {
  case CMP_M3_LIST_ONE_LINE:
    out_metrics->height = 56.0f;
    break;
  case CMP_M3_LIST_TWO_LINE:
    out_metrics->height = 72.0f;
    break;
  case CMP_M3_LIST_THREE_LINE:
    out_metrics->height = 88.0f;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}

int cmp_m3_carousel_resolve(cmp_m3_carousel_variant_t variant,
                            cmp_m3_carousel_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;

  switch (variant) {
  case CMP_M3_CAROUSEL_MULTI_BROWSE:
  case CMP_M3_CAROUSEL_HERO:
    out_metrics->item_spacing = 8.0f;
    out_metrics->contained_shape = CMP_M3_SHAPE_EXTRA_LARGE; /* 28dp */
    break;
  case CMP_M3_CAROUSEL_UNCONTAINED:
    out_metrics->item_spacing = 8.0f; /* Uncontained has shape none typically */
    out_metrics->contained_shape = CMP_M3_SHAPE_NONE;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}