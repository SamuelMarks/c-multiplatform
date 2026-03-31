/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_scroll_timeline {
  int is_active;
};

int cmp_scroll_timeline_create(cmp_scroll_timeline_t **out_timeline) {
  struct cmp_scroll_timeline *timeline;

  if (!out_timeline)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_scroll_timeline), (void **)&timeline) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(timeline, 0, sizeof(struct cmp_scroll_timeline));
  timeline->is_active = 1;

  *out_timeline = (cmp_scroll_timeline_t *)timeline;
  return CMP_SUCCESS;
}

int cmp_scroll_timeline_destroy(cmp_scroll_timeline_t *timeline) {
  struct cmp_scroll_timeline *internal_timeline =
      (struct cmp_scroll_timeline *)timeline;
  if (!internal_timeline)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_timeline);
  return CMP_SUCCESS;
}

int cmp_scroll_timeline_evaluate(cmp_scroll_timeline_t *timeline,
                                 float scroll_offset, float max_scroll_offset,
                                 float *out_progress) {
  struct cmp_scroll_timeline *t = (struct cmp_scroll_timeline *)timeline;
  float progress;

  if (!t || !out_progress)
    return CMP_ERROR_INVALID_ARG;

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