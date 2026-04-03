/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_navigation.h"
#include <stddef.h>
/* clang-format on */

int cmp_m3_bottom_app_bar_resolve(
    cmp_m3_bottom_app_bar_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;
  out_metrics->height = 80.0f;
  out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_2;
  out_metrics->max_action_items = 4;
  return CMP_SUCCESS;
}

int cmp_m3_bottom_nav_resolve(cmp_m3_bottom_nav_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;
  out_metrics->height = 80.0f;
  out_metrics->active_indicator_width = 64.0f;
  out_metrics->active_indicator_height = 32.0f;
  out_metrics->active_indicator_shape = CMP_M3_SHAPE_FULL;
  return CMP_SUCCESS;
}

int cmp_m3_drawer_resolve(cmp_m3_drawer_variant_t variant,
                          cmp_m3_drawer_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;

  out_metrics->max_width = 360.0f;
  out_metrics->active_item_padding = 16.0f;
  out_metrics->active_item_shape = CMP_M3_SHAPE_FULL;

  switch (variant) {
  case CMP_M3_DRAWER_MODAL:
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_1; /* With Scrim 32% */
    out_metrics->shape =
        CMP_M3_SHAPE_LARGE; /* Specifically on trailing edge typically */
    break;
  case CMP_M3_DRAWER_STANDARD:
    out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_0;
    out_metrics->shape = CMP_M3_SHAPE_NONE;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }
  return CMP_SUCCESS;
}

int cmp_m3_nav_rail_resolve(cmp_m3_nav_rail_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;
  out_metrics->width = 80.0f;
  out_metrics->active_indicator_shape = CMP_M3_SHAPE_FULL;
  return CMP_SUCCESS;
}

int cmp_m3_top_app_bar_resolve(cmp_m3_top_app_bar_variant_t variant,
                               cmp_m3_top_app_bar_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;
  out_metrics->elevation_scrolled = CMP_M3_ELEVATION_LEVEL_2;

  switch (variant) {
  case CMP_M3_TOP_APP_BAR_CENTER_ALIGNED:
  case CMP_M3_TOP_APP_BAR_SMALL:
    out_metrics->height_collapsed = 64.0f;
    out_metrics->height_expanded = 64.0f;
    break;
  case CMP_M3_TOP_APP_BAR_MEDIUM:
    out_metrics->height_collapsed = 64.0f;
    out_metrics->height_expanded = 112.0f;
    break;
  case CMP_M3_TOP_APP_BAR_LARGE:
    out_metrics->height_collapsed = 64.0f;
    out_metrics->height_expanded = 152.0f;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }
  return CMP_SUCCESS;
}

int cmp_m3_tabs_resolve(cmp_m3_tabs_variant_t variant,
                        cmp_m3_tabs_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;

  out_metrics->height = 48.0f;

  switch (variant) {
  case CMP_M3_TABS_PRIMARY:
    out_metrics->active_indicator_shape =
        CMP_M3_SHAPE_FULL;                          /* Pill indicator inside */
    out_metrics->active_indicator_thickness = 0.0f; /* N/A for primary */
    break;
  case CMP_M3_TABS_SECONDARY:
    out_metrics->active_indicator_shape = CMP_M3_SHAPE_NONE;
    out_metrics->active_indicator_thickness = 2.0f; /* Bottom line */
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}