#include "m3_motion.h"

int m3_motion_init(m3_motion_ctx_t *ctx) {
  int res, i;
  if (!ctx)
    return 1;

  for (i = 0; i < M3_EASING_COUNT; i++) {
    ctx->curves[i] = NULL;
  }

  /* Emphasized: Typically (0.2, 0.0, 0.0, 1.0) */
  res = cmp_bezier_ease_create(0.2f, 0.0f, 0.0f, 1.0f,
                               &ctx->curves[M3_EASING_EMPHASIZED]);
  if (res != 0)
    return res;

  /* Emphasized Decelerate: (0.05, 0.7, 0.1, 1.0) */
  res = cmp_bezier_ease_create(0.05f, 0.7f, 0.1f, 1.0f,
                               &ctx->curves[M3_EASING_EMPHASIZED_DECELERATE]);
  if (res != 0)
    return res;

  /* Emphasized Accelerate: (0.3, 0.0, 0.8, 0.15) */
  res = cmp_bezier_ease_create(0.3f, 0.0f, 0.8f, 0.15f,
                               &ctx->curves[M3_EASING_EMPHASIZED_ACCELERATE]);
  if (res != 0)
    return res;

  /* Standard: (0.2, 0.0, 0.0, 1.0) */
  res = cmp_bezier_ease_create(0.2f, 0.0f, 0.0f, 1.0f,
                               &ctx->curves[M3_EASING_STANDARD]);
  if (res != 0)
    return res;

  /* Standard Decelerate: (0.0, 0.0, 0.0, 1.0) */
  res = cmp_bezier_ease_create(0.0f, 0.0f, 0.0f, 1.0f,
                               &ctx->curves[M3_EASING_STANDARD_DECELERATE]);
  if (res != 0)
    return res;

  /* Standard Accelerate: (0.3, 0.0, 1.0, 1.0) */
  res = cmp_bezier_ease_create(0.3f, 0.0f, 1.0f, 1.0f,
                               &ctx->curves[M3_EASING_STANDARD_ACCELERATE]);
  if (res != 0)
    return res;

  return 0;
}

int m3_motion_cleanup(m3_motion_ctx_t *ctx) {
  int i;
  if (!ctx)
    return 1;

  for (i = 0; i < M3_EASING_COUNT; i++) {
    if (ctx->curves[i]) {
      cmp_bezier_ease_destroy(ctx->curves[i]);
      ctx->curves[i] = NULL;
    }
  }
  return 0;
}

int m3_motion_evaluate(const m3_motion_ctx_t *ctx, m3_easing_type_t type,
                       float t, float *out_val) {
  if (!ctx || !out_val || type < 0 || type >= M3_EASING_COUNT)
    return 1;
  if (!ctx->curves[type])
    return 1;

  if (t < 0.0f)
    t = 0.0f;
  if (t > 1.0f)
    t = 1.0f;

  return cmp_bezier_ease_evaluate(ctx->curves[type], t, out_val);
}

int m3_motion_shared_axis_offset(const m3_motion_ctx_t *ctx, float t,
                                 int is_forward, float distance,
                                 float *out_offset) {
  float eased_t;
  int res;

  if (!ctx || !out_offset)
    return 1;

  res = m3_motion_evaluate(ctx, M3_EASING_EMPHASIZED, t, &eased_t);
  if (res != 0)
    return res;

  /* In a forward transition, the outgoing screen moves from 0 to -distance.
   * The incoming screen moves from +distance to 0.
   * We'll just provide the raw offset based on the progress.
   */
  if (is_forward) {
    *out_offset = (1.0f - eased_t) * distance;
  } else {
    *out_offset = (1.0f - eased_t) * -distance;
  }

  return 0;
}

int m3_motion_shared_axis_fade(const m3_motion_ctx_t *ctx, float t,
                               float *out_outgoing_opacity,
                               float *out_incoming_opacity) {
  if (!ctx || !out_outgoing_opacity || !out_incoming_opacity)
    return 1;

  /* Shared axis fade typically happens entirely within the first or second
   * half, or uses standard ease */
  /* Outgoing fades out in first 30% of time */
  if (t <= 0.3f) {
    *out_outgoing_opacity = 1.0f - (t / 0.3f);
    *out_incoming_opacity = 0.0f;
  } else {
    *out_outgoing_opacity = 0.0f;
    /* Incoming fades in from 30% to 100% */
    *out_incoming_opacity = (t - 0.3f) / 0.7f;
  }

  return 0;
}
