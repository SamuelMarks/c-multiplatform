/**
 * @file md2_shape.c
 * @brief Implementation of Material Design 2 shape system.
 */

/* clang-format off */
#include "material2/md2_shape.h"
/* clang-format on */

ui_error_t md2_shape_get_corner_radius(enum md2_shape_category category,
                                       float *out_radius_dp) {
  if (out_radius_dp == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  switch (category) {
  case MD2_SHAPE_CATEGORY_SMALL_COMPONENT:
    *out_radius_dp = 4.0f;
    break;
  case MD2_SHAPE_CATEGORY_MEDIUM_COMPONENT:
    *out_radius_dp = 4.0f;
    break;
  case MD2_SHAPE_CATEGORY_LARGE_COMPONENT:
    *out_radius_dp = 0.0f;
    break;
  default:
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return UI_ERROR_NONE;
}
