/* clang-format off */
#include "ui_shape_family.h"
/* clang-format on */

/*
 * @brief ui_shape_family_get_radius.
 * @param family Parameter family.
 * @param out_radius Parameter out_radius.
 * @return Return value.
 */
ui_error_t ui_shape_family_get_radius(enum ui_shape_family family,
                                      float *out_radius) {
  if (!out_radius) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  switch (family) {
  case UI_SHAPE_FAMILY_NONE:
    *out_radius = 0.0f;
    break;
  case UI_SHAPE_FAMILY_EXTRA_SMALL:
    *out_radius = 4.0f;
    break;
  case UI_SHAPE_FAMILY_SMALL:
    *out_radius = 8.0f;
    break;
  case UI_SHAPE_FAMILY_MEDIUM:
    *out_radius = 12.0f;
    break;
  case UI_SHAPE_FAMILY_LARGE:
    *out_radius = 16.0f;
    break;
  case UI_SHAPE_FAMILY_EXTRA_LARGE:
    *out_radius = 28.0f;
    break;
  case UI_SHAPE_FAMILY_PILL:
    *out_radius = 9999.0f; /* Arbitrarily large value for full rounding */
    break;
  default:
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return UI_ERROR_NONE;
}
