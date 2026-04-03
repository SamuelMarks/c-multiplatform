/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_pickers_menus.h"
#include "themes/cmp_material3_pickers_menus_ext.h"
#include <stddef.h>
/* clang-format on */

int cmp_m3_menu_resolve(cmp_m3_menu_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;
  out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_2;
  out_metrics->shape = CMP_M3_SHAPE_EXTRA_SMALL; /* 4dp */
  return CMP_SUCCESS;
}

int cmp_m3_sheet_resolve(cmp_m3_sheet_variant_t variant,
                         cmp_m3_sheet_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;

  out_metrics->has_scrim = 0;
  out_metrics->has_drag_handle = 0;

  switch (variant) {
  case CMP_M3_SHEET_BOTTOM_STANDARD:
    out_metrics->elevation =
        CMP_M3_ELEVATION_LEVEL_1; /* Or 2/3/4 depending on context */
    out_metrics->shape = CMP_M3_SHAPE_EXTRA_LARGE; /* 28dp top corners */
    out_metrics->has_drag_handle = 1;
    break;
  case CMP_M3_SHEET_BOTTOM_MODAL:
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_1;
    out_metrics->shape = CMP_M3_SHAPE_EXTRA_LARGE;
    out_metrics->has_scrim = 1;
    out_metrics->has_drag_handle = 1;
    break;
  case CMP_M3_SHEET_SIDE_STANDARD:
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_1;
    out_metrics->shape = CMP_M3_SHAPE_NONE;
    break;
  case CMP_M3_SHEET_SIDE_MODAL:
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_1;
    out_metrics->shape = CMP_M3_SHAPE_NONE; /* Standard side sheet */
    out_metrics->has_scrim = 1;
    break;
  case CMP_M3_SHEET_SIDE_DETACHED:
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_1;
    out_metrics->shape = CMP_M3_SHAPE_LARGE; /* Fully rounded typically 16dp */
    out_metrics->has_scrim = 1;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}

int cmp_m3_date_picker_resolve(cmp_m3_date_picker_variant_t variant,
                               cmp_m3_date_picker_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;

  out_metrics->day_selection_size = 40.0f;

  switch (variant) {
  case CMP_M3_DATE_PICKER_MODAL:
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_3;
    out_metrics->shape = CMP_M3_SHAPE_EXTRA_LARGE;
    break;
  case CMP_M3_DATE_PICKER_DOCKED:
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_0;
    out_metrics->shape =
        CMP_M3_SHAPE_NONE; /* Or small depending on container */
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}

int cmp_m3_time_picker_resolve(cmp_m3_time_picker_variant_t variant,
                               cmp_m3_time_picker_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;

  out_metrics->am_pm_segment_height = 40.0f; /* Segmented button sizes */

  switch (variant) {
  case CMP_M3_TIME_PICKER_DIAL:
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_3;
    out_metrics->shape = CMP_M3_SHAPE_EXTRA_LARGE;
    break;
  case CMP_M3_TIME_PICKER_INPUT:
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_3;
    out_metrics->shape = CMP_M3_SHAPE_EXTRA_LARGE;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}