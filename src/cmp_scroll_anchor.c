/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_scroll_anchor {
  unsigned int active_element_id;
  float saved_visual_offset_y;
};

int cmp_scroll_anchor_create(cmp_scroll_anchor_t **out_anchor) {
  struct cmp_scroll_anchor *anchor;

  if (!out_anchor)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_scroll_anchor), (void **)&anchor) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(anchor, 0, sizeof(struct cmp_scroll_anchor));

  *out_anchor = (cmp_scroll_anchor_t *)anchor;
  return CMP_SUCCESS;
}

int cmp_scroll_anchor_destroy(cmp_scroll_anchor_t *anchor) {
  struct cmp_scroll_anchor *internal_anchor =
      (struct cmp_scroll_anchor *)anchor;

  if (!internal_anchor)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_anchor);
  return CMP_SUCCESS;
}

int cmp_scroll_anchor_save(cmp_scroll_anchor_t *anchor, unsigned int element_id,
                           float visual_offset_y) {
  struct cmp_scroll_anchor *internal_anchor =
      (struct cmp_scroll_anchor *)anchor;

  if (!internal_anchor)
    return CMP_ERROR_INVALID_ARG;

  internal_anchor->active_element_id = element_id;
  internal_anchor->saved_visual_offset_y = visual_offset_y;

  return CMP_SUCCESS;
}

int cmp_scroll_anchor_restore(const cmp_scroll_anchor_t *anchor,
                              unsigned int element_id,
                              float new_visual_offset_y,
                              float *out_scroll_delta_y) {
  const struct cmp_scroll_anchor *internal_anchor =
      (const struct cmp_scroll_anchor *)anchor;

  if (!internal_anchor || !out_scroll_delta_y)
    return CMP_ERROR_INVALID_ARG;

  if (internal_anchor->active_element_id != element_id) {
    *out_scroll_delta_y = 0.0f;
    return CMP_SUCCESS; /* Element is not the active anchor, no delta */
  }

  *out_scroll_delta_y =
      new_visual_offset_y - internal_anchor->saved_visual_offset_y;
  return CMP_SUCCESS;
}