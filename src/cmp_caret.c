/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_caret {
  double elapsed_ms;
  int is_visible;
  double blink_rate_ms;
};

int cmp_caret_create(cmp_caret_t **out_caret) {
  struct cmp_caret *caret;

  if (!out_caret)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_caret), (void **)&caret) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(caret, 0, sizeof(struct cmp_caret));
  caret->blink_rate_ms = 500.0; /* 500ms on, 500ms off */
  caret->is_visible = 1;

  *out_caret = (cmp_caret_t *)caret;
  return CMP_SUCCESS;
}

int cmp_caret_destroy(cmp_caret_t *caret) {
  struct cmp_caret *internal_caret = (struct cmp_caret *)caret;
  if (!internal_caret)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_caret);
  return CMP_SUCCESS;
}

int cmp_caret_update_blink(cmp_caret_t *caret, double dt_ms,
                           int *out_is_visible) {
  struct cmp_caret *internal_caret = (struct cmp_caret *)caret;

  if (!internal_caret || !out_is_visible || dt_ms < 0.0)
    return CMP_ERROR_INVALID_ARG;

  internal_caret->elapsed_ms += dt_ms;

  if (internal_caret->elapsed_ms >= internal_caret->blink_rate_ms) {
    internal_caret->elapsed_ms -= internal_caret->blink_rate_ms;
    internal_caret->is_visible = !internal_caret->is_visible;
  }

  *out_is_visible = internal_caret->is_visible;

  return CMP_SUCCESS;
}
