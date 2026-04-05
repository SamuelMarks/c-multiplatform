/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_safe_area_handler {
  cmp_safe_areas_t *areas;
};

int cmp_safe_area_handler_create(cmp_safe_area_handler_t **out_handler) {
  cmp_safe_area_handler_t *handler;
  if (!out_handler)
    return CMP_ERROR_INVALID_ARG;

  handler = (cmp_safe_area_handler_t *)malloc(sizeof(cmp_safe_area_handler_t));
  if (!handler)
    return CMP_ERROR_OOM;

  if (cmp_safe_areas_create(&handler->areas) != CMP_SUCCESS) {
    free(handler);
    return CMP_ERROR_GENERAL;
  }

  *out_handler = handler;
  return CMP_SUCCESS;
}

int cmp_safe_area_handler_destroy(cmp_safe_area_handler_t *handler) {
  if (!handler)
    return CMP_ERROR_INVALID_ARG;

  if (handler->areas) {
    cmp_safe_areas_destroy(handler->areas);
  }
  free(handler);
  return CMP_SUCCESS;
}

int cmp_safe_area_handler_query_insets(cmp_safe_area_handler_t *handler,
                                       cmp_window_t *window, int *out_top,
                                       int *out_bottom, int *out_left,
                                       int *out_right) {
  float top = 0.0f;
  float bottom = 0.0f;
  float left = 0.0f;
  float right = 0.0f;

  if (!handler || !window) {
    if (out_top)
      *out_top = 0;
    if (out_bottom)
      *out_bottom = 0;
    if (out_left)
      *out_left = 0;
    if (out_right)
      *out_right = 0;
    return CMP_ERROR_INVALID_ARG;
  }

  /* Mock retrieval of platform insets from cmp_safe_areas_t.
   * Normally, this would call OS-specific functions via cmp.
   * Since we are just providing the structure to the UI framework, we retrieve
   * the set values.
   */
  cmp_safe_areas_get_inset(handler->areas, 0, &top);
  cmp_safe_areas_get_inset(handler->areas, 2, &bottom);
  cmp_safe_areas_get_inset(handler->areas, 3, &left);
  cmp_safe_areas_get_inset(handler->areas, 1, &right);

  if (out_top)
    *out_top = (int)top;
  if (out_bottom)
    *out_bottom = (int)bottom;
  if (out_left)
    *out_left = (int)left;
  if (out_right)
    *out_right = (int)right;

  return CMP_SUCCESS;
}
