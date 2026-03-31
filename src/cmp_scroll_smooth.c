/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

struct cmp_scroll_smooth {
  float start_position;
  float target_position;
  float current_position;
  float duration_ms;
  float elapsed_time_ms;
  int is_complete;
};

int cmp_scroll_smooth_create(cmp_scroll_smooth_t **out_smooth) {
  struct cmp_scroll_smooth *ctx;

  if (!out_smooth)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_scroll_smooth), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(ctx, 0, sizeof(struct cmp_scroll_smooth));
  ctx->duration_ms = 300.0f; /* Default CSS scroll-behavior: smooth duration */
  ctx->is_complete = 1;

  *out_smooth = (cmp_scroll_smooth_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_scroll_smooth_destroy(cmp_scroll_smooth_t *smooth) {
  struct cmp_scroll_smooth *ctx = (struct cmp_scroll_smooth *)smooth;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_scroll_smooth_start(cmp_scroll_smooth_t *smooth, float current_pos,
                            float target_pos) {
  struct cmp_scroll_smooth *ctx = (struct cmp_scroll_smooth *)smooth;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->start_position = current_pos;
  ctx->current_position = current_pos;
  ctx->target_position = target_pos;
  ctx->elapsed_time_ms = 0.0f;
  ctx->is_complete = 0;

  return CMP_SUCCESS;
}

/* Simple cubic bezier ease-in-out mapping for progress 0..1 */
static float ease_in_out_cubic(float t) {
  if (t < 0.5f) {
    return 4.0f * t * t * t;
  } else {
    float f = ((2.0f * t) - 2.0f);
    return 0.5f * f * f * f + 1.0f;
  }
}

int cmp_scroll_smooth_step(cmp_scroll_smooth_t *smooth, float dt_ms,
                           float *out_current_position, int *out_is_complete) {
  struct cmp_scroll_smooth *ctx = (struct cmp_scroll_smooth *)smooth;
  float progress, eased_progress;

  if (!ctx || !out_current_position || !out_is_complete)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->is_complete) {
    *out_current_position = ctx->current_position;
    *out_is_complete = 1;
    return CMP_SUCCESS;
  }

  ctx->elapsed_time_ms += dt_ms;

  if (ctx->elapsed_time_ms >= ctx->duration_ms) {
    ctx->current_position = ctx->target_position;
    ctx->is_complete = 1;
    *out_current_position = ctx->current_position;
    *out_is_complete = 1;
    return CMP_SUCCESS;
  }

  progress = ctx->elapsed_time_ms / ctx->duration_ms;
  eased_progress = ease_in_out_cubic(progress);

  ctx->current_position =
      ctx->start_position +
      (ctx->target_position - ctx->start_position) * eased_progress;

  *out_current_position = ctx->current_position;
  *out_is_complete = ctx->is_complete;

  return CMP_SUCCESS;
}
