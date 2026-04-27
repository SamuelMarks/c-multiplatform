/* clang-format off */
#include "cmp.h"

#include <stdlib.h>

#include "cmp_log.h"
/* clang-format on */

/**
 * @brief Apply scroll padding and margin to compute the target scroll offset.
 *
 * @param element_bounds Parameter description.
 * @param padding Parameter description.
 * @param margin Parameter description.
 * @param out_target_scroll_y Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_padding_apply(const cmp_rect_t *element_bounds,
                             const cmp_scroll_padding_t *padding,
                             const cmp_scroll_padding_t *margin,
                             float *out_target_scroll_y) {
  float base_y;

  if (element_bounds == NULL || out_target_scroll_y == NULL) {
    LOG_DEBUG(
        "Invalid argument: element_bounds or out_target_scroll_y is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  base_y = element_bounds->y;

  /* Subtract top margin of the target element */
  if (margin != NULL) {
    base_y -= margin->top;
  }

  /* Subtract top padding of the scrolling container (e.g. sticky headers) */
  if (padding != NULL) {
    base_y -= padding->top;
  }

  *out_target_scroll_y = base_y;

  return CMP_SUCCESS;
}
