/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

int cmp_scrollbar_gutter_calculate(cmp_scrollbar_gutter_t mode,
                                   int is_overflowing,
                                   float scrollbar_thickness,
                                   float *out_reserved_left,
                                   float *out_reserved_right) {
  if (!out_reserved_left || !out_reserved_right) {
    return CMP_ERROR_INVALID_ARG;
  }

  *out_reserved_left = 0.0f;
  *out_reserved_right = 0.0f;

  if (mode == CMP_SCROLLBAR_GUTTER_AUTO) {
    if (is_overflowing) {
      *out_reserved_right = scrollbar_thickness;
    }
  } else if (mode == CMP_SCROLLBAR_GUTTER_STABLE) {
    *out_reserved_right = scrollbar_thickness;
  } else if (mode == CMP_SCROLLBAR_GUTTER_STABLE_BOTH_EDGES) {
    *out_reserved_left = scrollbar_thickness;
    *out_reserved_right = scrollbar_thickness;
  } else {
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}