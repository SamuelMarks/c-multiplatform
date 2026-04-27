/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>

#include "cmp_log.h"
/* clang-format on */

struct cmp_scroll_timeline {
  int is_active;
};

/**
 * @brief Create a scroll timeline.
 *
 * @param out_timeline Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_timeline_create(cmp_scroll_timeline_t **out_timeline) {
  int rc;
  struct cmp_scroll_timeline *timeline;

  rc = CMP_SUCCESS;

  if (out_timeline == NULL) {
    LOG_DEBUG("Invalid argument: out_timeline is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_scroll_timeline), (void **)&timeline);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  memset(timeline, 0, sizeof(struct cmp_scroll_timeline));
  timeline->is_active = 1;

  *out_timeline = (cmp_scroll_timeline_t *)timeline;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy a scroll timeline.
 *
 * @param timeline Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_timeline_destroy(cmp_scroll_timeline_t *timeline) {
  int rc;
  struct cmp_scroll_timeline *internal_timeline;

  rc = CMP_SUCCESS;
  internal_timeline = (struct cmp_scroll_timeline *)timeline;

  if (internal_timeline == NULL) {
    LOG_DEBUG("Invalid argument: timeline is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(internal_timeline);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Evaluate the scroll timeline progress.
 *
 * @param timeline Parameter description.
 * @param scroll_offset Parameter description.
 * @param max_scroll_offset Parameter description.
 * @param out_progress Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_timeline_evaluate(cmp_scroll_timeline_t *timeline,
                                 float scroll_offset, float max_scroll_offset,
                                 float *out_progress) {
  struct cmp_scroll_timeline *t;
  float progress;

  t = (struct cmp_scroll_timeline *)timeline;

  if (t == NULL || out_progress == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (max_scroll_offset <= 0.0f) {
    *out_progress = 0.0f;
    return CMP_SUCCESS;
  }

  progress = scroll_offset / max_scroll_offset;
  if (progress < 0.0f) {
    progress = 0.0f;
  } else if (progress > 1.0f) {
    progress = 1.0f;
  }

  *out_progress = progress;

  return CMP_SUCCESS;
}
