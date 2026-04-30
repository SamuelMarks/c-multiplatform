/* clang-format off */
#include "cmp.h"

#include <stdlib.h>

#include "cmp_log.h"
/* clang-format on */

/**
 * @brief Calculate reserved layout space for a scrollbar gutter.
 *
 * @param mode Parameter description.
 * @param is_overflowing Parameter description.
 * @param scrollbar_thickness Parameter description.
 * @param out_reserved_left Parameter description.
 * @param out_reserved_right Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scrollbar_gutter_calculate(cmp_scrollbar_gutter_t mode,
                                   int is_overflowing,
                                   float scrollbar_thickness,
                                   float *out_reserved_left,
                                   float *out_reserved_right) {
  int rc = CMP_SUCCESS;
  if (out_reserved_left == NULL || out_reserved_right == NULL) {
    LOG_DEBUG(
        "Invalid argument: out_reserved_left or out_reserved_right is NULL\n");
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
    LOG_DEBUG("Invalid argument: unrecognized mode\n");
    return CMP_ERROR_INVALID_ARG;
  }

  return rc;
}
