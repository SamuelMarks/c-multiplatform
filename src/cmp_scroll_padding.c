/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

int cmp_scroll_padding_apply(const cmp_rect_t *element_bounds,
                             const cmp_scroll_padding_t *padding,
                             const cmp_scroll_padding_t *margin,
                             float *out_target_scroll_y) {
  float base_y;

  if (!element_bounds || !out_target_scroll_y)
    return CMP_ERROR_INVALID_ARG;

  base_y = element_bounds->y;

  /* Subtract top margin of the target element */
  if (margin) {
    base_y -= margin->top;
  }

  /* Subtract top padding of the scrolling container (e.g. sticky headers) */
  if (padding) {
    base_y -= padding->top;
  }

  *out_target_scroll_y = base_y;
  return CMP_SUCCESS;
}
