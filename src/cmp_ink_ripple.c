/* clang-format off */
#include "cmp_ink_ripple.h"
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ink_ripple {
  float radius;
  float origin_x;
  float origin_y;
  int active;
};

int cmp_ink_ripple_create(cmp_ink_ripple_t **out_ripple) {
  struct cmp_ink_ripple *ctx;
  if (!out_ripple)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_ink_ripple), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(ctx, 0, sizeof(struct cmp_ink_ripple));
  *out_ripple = (cmp_ink_ripple_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_ink_ripple_destroy(cmp_ink_ripple_t *ripple) {
  struct cmp_ink_ripple *ctx = (struct cmp_ink_ripple *)ripple;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_ink_ripple_update(cmp_ink_ripple_t *ripple, float dt_ms) {
  struct cmp_ink_ripple *ctx = (struct cmp_ink_ripple *)ripple;
  if (!ctx || dt_ms < 0.0f)
    return CMP_ERROR_INVALID_ARG;
  if (ctx->active) {
    ctx->radius += dt_ms * 0.1f; /* Simplified expansion */
  }
  return CMP_SUCCESS;
}

int cmp_ink_ripple_trigger(cmp_ink_ripple_t *ripple, float start_x,
                           float start_y) {
  struct cmp_ink_ripple *ctx = (struct cmp_ink_ripple *)ripple;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->origin_x = start_x;
  ctx->origin_y = start_y;
  ctx->radius = 0.0f;
  ctx->active = 1;
  return CMP_SUCCESS;
}
