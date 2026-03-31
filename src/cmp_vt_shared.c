/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_vt_shared {
  int dummy;
};

int cmp_vt_shared_create(cmp_vt_shared_t **out_shared) {
  struct cmp_vt_shared *shared;

  if (!out_shared)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_vt_shared), (void **)&shared) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(shared, 0, sizeof(struct cmp_vt_shared));

  *out_shared = (cmp_vt_shared_t *)shared;
  return CMP_SUCCESS;
}

int cmp_vt_shared_destroy(cmp_vt_shared_t *shared) {
  struct cmp_vt_shared *internal_shared = (struct cmp_vt_shared *)shared;
  if (!internal_shared)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_shared);
  return CMP_SUCCESS;
}

int cmp_vt_shared_calculate_morph(cmp_vt_shared_t *shared,
                                  const cmp_rect_t *old_rect,
                                  const cmp_rect_t *new_rect, float progress,
                                  cmp_rect_t *out_rect) {
  struct cmp_vt_shared *internal_shared = (struct cmp_vt_shared *)shared;

  if (!internal_shared || !old_rect || !new_rect || !out_rect ||
      progress < 0.0f || progress > 1.0f)
    return CMP_ERROR_INVALID_ARG;

  out_rect->x = old_rect->x + (new_rect->x - old_rect->x) * progress;
  out_rect->y = old_rect->y + (new_rect->y - old_rect->y) * progress;
  out_rect->width =
      old_rect->width + (new_rect->width - old_rect->width) * progress;
  out_rect->height =
      old_rect->height + (new_rect->height - old_rect->height) * progress;

  return CMP_SUCCESS;
}