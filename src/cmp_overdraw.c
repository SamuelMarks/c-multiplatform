/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_overdraw {
  int is_active;
};

int cmp_overdraw_create(cmp_overdraw_t **out_overdraw) {
  struct cmp_overdraw *ctx;

  if (!out_overdraw)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_overdraw), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(ctx, 0, sizeof(struct cmp_overdraw));
  ctx->is_active = 0;

  *out_overdraw = (cmp_overdraw_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_overdraw_destroy(cmp_overdraw_t *overdraw) {
  struct cmp_overdraw *ctx = (struct cmp_overdraw *)overdraw;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_overdraw_set_enabled(cmp_overdraw_t *overdraw, cmp_renderer_t *renderer,
                             int enable) {
  struct cmp_overdraw *ctx = (struct cmp_overdraw *)overdraw;

  if (!ctx || !renderer)
    return CMP_ERROR_INVALID_ARG;

  /* In a real implementation this would swap the active fragment shader
     on the renderer to an additive blending heat-map shader */
  ctx->is_active = enable ? 1 : 0;

  return CMP_SUCCESS;
}
