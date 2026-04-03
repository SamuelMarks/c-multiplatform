/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_components.h"
#include <stddef.h>
/* clang-format on */

int cmp_m3_button_resolve(cmp_m3_button_variant_t variant, int has_leading_icon,
                          cmp_m3_button_metrics_t *out_metrics) {
  if (!out_metrics) {
    return CMP_ERROR_INVALID_ARG;
  }

  out_metrics->height = 40.0f;
  out_metrics->shape = CMP_M3_SHAPE_FULL; /* Pill */
  out_metrics->icon_size = 18.0f;
  out_metrics->border_thickness = 0.0f;
  out_metrics->elevation_disabled = CMP_M3_ELEVATION_LEVEL_0;
  out_metrics->elevation_base = CMP_M3_ELEVATION_LEVEL_0;
  out_metrics->elevation_hover =
      CMP_M3_ELEVATION_LEVEL_0; /* Most are 0 except Elevated */
  out_metrics->elevation_pressed = CMP_M3_ELEVATION_LEVEL_0;

  switch (variant) {
  case CMP_M3_BUTTON_ELEVATED:
    out_metrics->elevation_base = CMP_M3_ELEVATION_LEVEL_1;
    out_metrics->elevation_hover = CMP_M3_ELEVATION_LEVEL_2;
    out_metrics->elevation_pressed = CMP_M3_ELEVATION_LEVEL_1;

    if (has_leading_icon) {
      out_metrics->padding_left = 16.0f;
      out_metrics->gap_with_icon = 8.0f;
      out_metrics->padding_right = 24.0f;
    } else {
      out_metrics->padding_left = 24.0f;
      out_metrics->gap_with_icon = 0.0f;
      out_metrics->padding_right = 24.0f;
    }
    break;

  case CMP_M3_BUTTON_FILLED:
  case CMP_M3_BUTTON_FILLED_TONAL:
    out_metrics->elevation_base = CMP_M3_ELEVATION_LEVEL_0;
    out_metrics->elevation_hover = CMP_M3_ELEVATION_LEVEL_1;
    out_metrics->elevation_pressed = CMP_M3_ELEVATION_LEVEL_0;

    if (has_leading_icon) {
      out_metrics->padding_left = 16.0f;
      out_metrics->gap_with_icon = 8.0f;
      out_metrics->padding_right = 24.0f;
    } else {
      out_metrics->padding_left = 24.0f;
      out_metrics->gap_with_icon = 0.0f;
      out_metrics->padding_right = 24.0f;
    }
    break;

  case CMP_M3_BUTTON_OUTLINED:
    out_metrics->border_thickness = 1.0f;
    if (has_leading_icon) {
      out_metrics->padding_left = 16.0f;
      out_metrics->gap_with_icon = 8.0f;
      out_metrics->padding_right = 24.0f;
    } else {
      out_metrics->padding_left = 24.0f;
      out_metrics->gap_with_icon = 0.0f;
      out_metrics->padding_right = 24.0f;
    }
    break;

  case CMP_M3_BUTTON_TEXT:
    if (has_leading_icon) {
      out_metrics->padding_left = 12.0f;
      out_metrics->gap_with_icon = 8.0f;
      out_metrics->padding_right = 16.0f;
    } else {
      out_metrics->padding_left = 12.0f;
      out_metrics->gap_with_icon = 0.0f;
      out_metrics->padding_right = 12.0f;
    }
    break;

  default:
    return CMP_ERROR_INVALID_ARG;
  }

  /* Helper field if people just want the total left padding */
  out_metrics->padding_leading_with_icon = out_metrics->padding_left;

  return CMP_SUCCESS;
}

int cmp_m3_fab_resolve(cmp_m3_fab_variant_t variant, int is_lowered,
                       cmp_m3_fab_metrics_t *out_metrics) {
  if (!out_metrics) {
    return CMP_ERROR_INVALID_ARG;
  }

  out_metrics->elevation_base =
      is_lowered ? CMP_M3_ELEVATION_LEVEL_1 : CMP_M3_ELEVATION_LEVEL_3;
  out_metrics->elevation_hover =
      is_lowered ? CMP_M3_ELEVATION_LEVEL_2 : CMP_M3_ELEVATION_LEVEL_4;
  out_metrics->elevation_pressed =
      is_lowered ? CMP_M3_ELEVATION_LEVEL_1 : CMP_M3_ELEVATION_LEVEL_3;

  out_metrics->padding_left = 0.0f;
  out_metrics->padding_right = 0.0f;

  switch (variant) {
  case CMP_M3_FAB_STANDARD:
    out_metrics->container_width = 56.0f;
    out_metrics->container_height = 56.0f;
    out_metrics->icon_size = 24.0f;
    out_metrics->shape = CMP_M3_SHAPE_LARGE; /* 16dp radius typically */
    break;
  case CMP_M3_FAB_SMALL:
    out_metrics->container_width = 40.0f;
    out_metrics->container_height = 40.0f;
    out_metrics->icon_size = 24.0f;
    out_metrics->shape = CMP_M3_SHAPE_MEDIUM; /* 12dp radius typically */
    break;
  case CMP_M3_FAB_LARGE:
    out_metrics->container_width = 96.0f;
    out_metrics->container_height = 96.0f;
    out_metrics->icon_size = 36.0f;
    out_metrics->shape = CMP_M3_SHAPE_EXTRA_LARGE; /* 28dp radius */
    break;
  case CMP_M3_FAB_EXTENDED:
    out_metrics->container_width = 0.0f; /* Dynamic */
    out_metrics->container_height = 56.0f;
    out_metrics->icon_size = 24.0f;
    out_metrics->padding_left = 16.0f;
    out_metrics->padding_right = 16.0f;
    out_metrics->shape = CMP_M3_SHAPE_LARGE;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}

int cmp_m3_icon_button_resolve(cmp_m3_icon_button_variant_t variant,
                               cmp_m3_icon_button_metrics_t *out_metrics) {
  if (!out_metrics) {
    return CMP_ERROR_INVALID_ARG;
  }

  out_metrics->target_size = 48.0f;    /* Accessibility target size */
  out_metrics->footprint_size = 40.0f; /* Visual footprint */
  out_metrics->icon_size = 24.0f;
  out_metrics->shape = CMP_M3_SHAPE_FULL; /* Circular */
  out_metrics->border_thickness = 0.0f;

  switch (variant) {
  case CMP_M3_ICON_BUTTON_STANDARD:
    break;
  case CMP_M3_ICON_BUTTON_FILLED:
    break;
  case CMP_M3_ICON_BUTTON_FILLED_TONAL:
    break;
  case CMP_M3_ICON_BUTTON_OUTLINED:
    out_metrics->border_thickness = 1.0f;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}

int cmp_m3_segmented_button_resolve(
    cmp_m3_segmented_button_metrics_t *out_metrics) {
  if (!out_metrics) {
    return CMP_ERROR_INVALID_ARG;
  }

  out_metrics->height = 40.0f;
  out_metrics->border_thickness = 1.0f;
  out_metrics->checkmark_size = 18.0f;
  out_metrics->icon_size = 18.0f;

  return CMP_SUCCESS;
}