/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_selection_controls.h"
#include <stddef.h>
/* clang-format on */

int cmp_m3_checkbox_resolve(cmp_m3_checkbox_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;
  out_metrics->box_size = 18.0f;
  out_metrics->touch_target_size = 48.0f;
  out_metrics->border_thickness_unchecked = 2.0f;
  out_metrics->border_thickness_checked = 0.0f;  /* Usually filled */
  out_metrics->shape = CMP_M3_SHAPE_EXTRA_SMALL; /* 4dp */
  return CMP_SUCCESS;
}

int cmp_m3_radio_resolve(cmp_m3_radio_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;
  out_metrics->outer_ring_size = 20.0f;
  out_metrics->inner_dot_size = 10.0f;
  out_metrics->touch_target_size = 48.0f;
  out_metrics->border_thickness = 2.0f;
  out_metrics->shape = CMP_M3_SHAPE_FULL; /* Circular */
  return CMP_SUCCESS;
}

int cmp_m3_switch_resolve(cmp_m3_switch_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;
  out_metrics->track_width = 52.0f;
  out_metrics->track_height = 32.0f;
  out_metrics->touch_target_size = 48.0f;
  out_metrics->thumb_unselected_size = 16.0f;
  out_metrics->thumb_unselected_border = 1.0f;
  out_metrics->thumb_selected_size = 24.0f;
  out_metrics->thumb_pressed_size = 28.0f;
  out_metrics->shape = CMP_M3_SHAPE_FULL; /* Pill */
  return CMP_SUCCESS;
}

int cmp_m3_slider_resolve(cmp_m3_slider_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;
  out_metrics->track_height_inactive = 4.0f;
  out_metrics->track_height_active =
      16.0f; /* M3 Expressive can stretch up to 16dp */
  out_metrics->handle_size = 20.0f;
  out_metrics->touch_target_size = 48.0f;
  out_metrics->shape = CMP_M3_SHAPE_FULL; /* Pill/Circular */
  return CMP_SUCCESS;
}