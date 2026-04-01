/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_flip {
  cmp_rect_t initial_bounds;
  int has_first;
};

int cmp_flip_create(cmp_flip_t **out_flip) {
  struct cmp_flip *flip;

  if (!out_flip)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_flip), (void **)&flip) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(flip, 0, sizeof(struct cmp_flip));

  *out_flip = (cmp_flip_t *)flip;
  return CMP_SUCCESS;
}

int cmp_flip_destroy(cmp_flip_t *flip) {
  struct cmp_flip *internal_flip = (struct cmp_flip *)flip;
  if (!internal_flip)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_flip);
  return CMP_SUCCESS;
}

int cmp_flip_first(cmp_flip_t *flip, const cmp_rect_t *initial_bounds) {
  struct cmp_flip *internal_flip = (struct cmp_flip *)flip;

  if (!internal_flip || !initial_bounds)
    return CMP_ERROR_INVALID_ARG;

  internal_flip->initial_bounds = *initial_bounds;
  internal_flip->has_first = 1;

  return CMP_SUCCESS;
}

int cmp_flip_last_and_invert(cmp_flip_t *flip, const cmp_rect_t *final_bounds,
                             float *out_translate_x, float *out_translate_y,
                             float *out_scale_x, float *out_scale_y) {
  struct cmp_flip *internal_flip = (struct cmp_flip *)flip;

  if (!internal_flip || !final_bounds || !out_translate_x || !out_translate_y ||
      !out_scale_x || !out_scale_y)
    return CMP_ERROR_INVALID_ARG;

  if (!internal_flip->has_first)
    return CMP_ERROR_INVALID_STATE;

  /* Invert */
  *out_translate_x = internal_flip->initial_bounds.x - final_bounds->x;
  *out_translate_y = internal_flip->initial_bounds.y - final_bounds->y;

  if (final_bounds->width != 0.0f) {
    *out_scale_x = internal_flip->initial_bounds.width / final_bounds->width;
  } else {
    *out_scale_x = 1.0f;
  }

  if (final_bounds->height != 0.0f) {
    *out_scale_y = internal_flip->initial_bounds.height / final_bounds->height;
  } else {
    *out_scale_y = 1.0f;
  }

  return CMP_SUCCESS;
}
