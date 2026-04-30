/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cmp_log.h"
/* clang-format on */

struct cmp_scroll_smooth {
  float start_position;
  float target_position;
  float current_position;
  float duration_ms;
  float elapsed_time_ms;
  int is_complete;
};

/**
 * @brief Create a smooth scrolling context.
 *
 * @param out_smooth Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_smooth_create(cmp_scroll_smooth_t **out_smooth) {
  int rc = CMP_SUCCESS;
  struct cmp_scroll_smooth *ctx;

  rc = CMP_SUCCESS;

  if (out_smooth == NULL) {
    LOG_DEBUG("Invalid argument: out_smooth is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_scroll_smooth), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  memset(ctx, 0, sizeof(struct cmp_scroll_smooth));
  ctx->duration_ms = 300.0f; /* Default CSS scroll-behavior: smooth duration */
  ctx->is_complete = 1;

  *out_smooth = (cmp_scroll_smooth_t *)ctx;
  return rc;
}

/**
 * @brief Destroy a smooth scrolling context.
 *
 * @param smooth Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_smooth_destroy(cmp_scroll_smooth_t *smooth) {
  int rc = CMP_SUCCESS;
  struct cmp_scroll_smooth *ctx;

  rc = CMP_SUCCESS;
  ctx = (struct cmp_scroll_smooth *)smooth;

  if (ctx == NULL) {
    LOG_DEBUG("Invalid argument: smooth is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return rc;
}

/**
 * @brief Start a smooth scroll animation.
 *
 * @param smooth Parameter description.
 * @param current_pos Parameter description.
 * @param target_pos Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_smooth_start(cmp_scroll_smooth_t *smooth, float current_pos,
                            float target_pos) {
  int rc = CMP_SUCCESS;
  struct cmp_scroll_smooth *ctx;

  ctx = (struct cmp_scroll_smooth *)smooth;

  if (ctx == NULL) {
    LOG_DEBUG("Invalid argument: smooth is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx->start_position = current_pos;
  ctx->current_position = current_pos;
  ctx->target_position = target_pos;
  ctx->elapsed_time_ms = 0.0f;
  ctx->is_complete = 0;

  return rc;
}

/* Simple cubic bezier ease-in-out mapping for progress 0..1 */
CMP_EXEMPT(static float cmp_math_ease_in_out_cubic(float t)) {
  if (t < 0.5f) {
    return 4.0f * t * t * t;
  } else {
    float f = ((2.0f * t) - 2.0f);
    return 0.5f * f * f * f + 1.0f;
  }
}

/**
 * @brief Step a smooth scroll animation.
 *
 * @param smooth Parameter description.
 * @param dt_ms Parameter description.
 * @param out_current_position Parameter description.
 * @param out_is_complete Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_smooth_step(cmp_scroll_smooth_t *smooth, float dt_ms,
                           float *out_current_position, int *out_is_complete) {
  int rc = CMP_SUCCESS;
  struct cmp_scroll_smooth *ctx;
  float progress, eased_progress;

  ctx = (struct cmp_scroll_smooth *)smooth;

  if (ctx == NULL || out_current_position == NULL || out_is_complete == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (ctx->is_complete) {
    *out_current_position = ctx->current_position;
    *out_is_complete = 1;
    return rc;
  }

  ctx->elapsed_time_ms += dt_ms;

  if (ctx->elapsed_time_ms >= ctx->duration_ms) {
    ctx->current_position = ctx->target_position;
    ctx->is_complete = 1;
    *out_current_position = ctx->current_position;
    *out_is_complete = 1;
    return rc;
  }

  progress = ctx->elapsed_time_ms / ctx->duration_ms;
  eased_progress = cmp_math_ease_in_out_cubic(progress);

  ctx->current_position =
      ctx->start_position +
      (ctx->target_position - ctx->start_position) * eased_progress;

  *out_current_position = ctx->current_position;
  *out_is_complete = ctx->is_complete;

  return rc;
}
