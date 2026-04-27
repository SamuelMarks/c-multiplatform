/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>

#include "cmp_log.h"
/* clang-format on */

struct cmp_scroll_anchor {
  unsigned int active_element_id;
  float saved_visual_offset_y;
};

/**
 * @brief Create a scroll anchor.
 *
 * @param out_anchor Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_anchor_create(cmp_scroll_anchor_t **out_anchor) {
  int rc;
  struct cmp_scroll_anchor *anchor;

  rc = CMP_SUCCESS;

  if (out_anchor == NULL) {
    LOG_DEBUG("Invalid argument: out_anchor is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_scroll_anchor), (void **)&anchor);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  memset(anchor, 0, sizeof(struct cmp_scroll_anchor));

  *out_anchor = (cmp_scroll_anchor_t *)anchor;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy a scroll anchor.
 *
 * @param anchor Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_anchor_destroy(cmp_scroll_anchor_t *anchor) {
  int rc;
  struct cmp_scroll_anchor *internal_anchor;

  rc = CMP_SUCCESS;
  internal_anchor = (struct cmp_scroll_anchor *)anchor;

  if (internal_anchor == NULL) {
    LOG_DEBUG("Invalid argument: anchor is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(internal_anchor);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Save the current anchor.
 *
 * @param anchor Parameter description.
 * @param element_id Parameter description.
 * @param visual_offset_y Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_anchor_save(cmp_scroll_anchor_t *anchor, unsigned int element_id,
                           float visual_offset_y) {
  struct cmp_scroll_anchor *internal_anchor;

  internal_anchor = (struct cmp_scroll_anchor *)anchor;

  if (internal_anchor == NULL) {
    LOG_DEBUG("Invalid argument: anchor is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  internal_anchor->active_element_id = element_id;
  internal_anchor->saved_visual_offset_y = visual_offset_y;

  return CMP_SUCCESS;
}

/**
 * @brief Restore scroll offset based on the saved anchor.
 *
 * @param anchor Parameter description.
 * @param element_id Parameter description.
 * @param new_visual_offset_y Parameter description.
 * @param out_scroll_delta_y Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_anchor_restore(const cmp_scroll_anchor_t *anchor,
                              unsigned int element_id,
                              float new_visual_offset_y,
                              float *out_scroll_delta_y) {
  const struct cmp_scroll_anchor *internal_anchor;

  internal_anchor = (const struct cmp_scroll_anchor *)anchor;

  if (internal_anchor == NULL || out_scroll_delta_y == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (internal_anchor->active_element_id != element_id) {
    *out_scroll_delta_y = 0.0f;
    return CMP_SUCCESS; /* Element is not the active anchor, no delta */
  }

  *out_scroll_delta_y =
      new_visual_offset_y - internal_anchor->saved_visual_offset_y;

  return CMP_SUCCESS;
}
