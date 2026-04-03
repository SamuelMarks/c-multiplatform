/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_text_inputs.h"
#include <stddef.h>
/* clang-format on */

int cmp_m3_text_field_resolve(cmp_m3_text_field_variant_t variant,
                              cmp_m3_text_field_metrics_t *out_metrics) {
  if (!out_metrics)
    return CMP_ERROR_INVALID_ARG;

  out_metrics->container_height_single_line = 56.0f;
  out_metrics->icon_size = 24.0f;
  out_metrics->padding_horizontal = 16.0f;
  out_metrics->gap_with_icon = 16.0f;
  out_metrics->has_floating_label = 1; /* Both have floating label */

  out_metrics->shape_modifiers.top_left_override = -1;
  out_metrics->shape_modifiers.top_right_override = -1;
  out_metrics->shape_modifiers.bottom_left_override = -1;
  out_metrics->shape_modifiers.bottom_right_override = -1;

  switch (variant) {
  case CMP_M3_TEXT_FIELD_FILLED:
    out_metrics->shape = CMP_M3_SHAPE_EXTRA_SMALL;
    /* Top corners only */
    out_metrics->shape_modifiers.bottom_left_override = 0;
    out_metrics->shape_modifiers.bottom_right_override = 0;
    out_metrics->border_thickness_unfocused = 1.0f; /* Bottom line only */
    out_metrics->border_thickness_focused = 2.0f;
    break;
  case CMP_M3_TEXT_FIELD_OUTLINED:
    out_metrics->shape = CMP_M3_SHAPE_EXTRA_SMALL;
    out_metrics->border_thickness_unfocused = 1.0f;
    out_metrics->border_thickness_focused = 2.0f;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}