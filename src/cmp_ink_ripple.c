/* clang-format off */
#include "cmp_ink_ripple.h"
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

struct cmp_ink_ripple {
  float radius;
  float target_radius;
  float origin_x;
  float origin_y;
  float opacity;

  float t_expand; /* 0.0 to 1.0 */
  float t_fade;   /* 0.0 to 1.0 */

  int active; /* 1 if expanding/held */
  int fading; /* 1 if pointer released */
};

/* Helper: Cubic Bezier (0, 0, 0, 1) - Standard Decelerate */
static float cubic_bezier_decelerate(float t) {
  float tt = t * t;
  float ttt = tt * t;
  float p = ttt * 1.0f;
  return p;
}

/* Helper: Cubic Bezier (0.3, 0, 1, 1) - Standard Accelerate */
static float cubic_bezier_accelerate(float t) {
  /* For standard accelerate, p0=0, p1=0, p2=1, p3=1 */
  /* This is just an approximation for ease-in */
  return t * t; /* Simple quadratic ease-in for alpha fade out */
}

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
    ctx->t_expand += dt_ms / 200.0f; /* 200ms expansion */
    if (ctx->t_expand > 1.0f)
      ctx->t_expand = 1.0f;
    ctx->radius = ctx->target_radius * cubic_bezier_decelerate(ctx->t_expand);
  }

  if (ctx->fading) {
    ctx->t_fade += dt_ms / 150.0f; /* 150ms fade */
    if (ctx->t_fade > 1.0f) {
      ctx->t_fade = 1.0f;
      ctx->active = 0;
      ctx->fading = 0;
    }
    ctx->opacity =
        0.12f *
        (1.0f - cubic_bezier_accelerate(ctx->t_fade)); /* Base ripple opacity */
  }

  return CMP_SUCCESS;
}

int cmp_ink_ripple_trigger(cmp_ink_ripple_t *ripple, float start_x,
                           float start_y, float box_width, float box_height) {
  struct cmp_ink_ripple *ctx = (struct cmp_ink_ripple *)ripple;
  float dx, dy, dx2, dy2, max_dist;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->origin_x = start_x;
  ctx->origin_y = start_y;

  dx = (start_x > box_width / 2.0f) ? start_x : box_width - start_x;
  dy = (start_y > box_height / 2.0f) ? start_y : box_height - start_y;
  dx2 = dx * dx;
  dy2 = dy * dy;
  max_dist = (float)sqrt(dx2 + dy2);

  ctx->target_radius = max_dist;
  ctx->radius = 0.0f;
  ctx->opacity = 0.12f; /* Ripple peak opacity */
  ctx->t_expand = 0.0f;
  ctx->t_fade = 0.0f;
  ctx->active = 1;
  ctx->fading = 0;
  return CMP_SUCCESS;
}

int cmp_ink_ripple_release(cmp_ink_ripple_t *ripple) {
  struct cmp_ink_ripple *ctx = (struct cmp_ink_ripple *)ripple;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  if (ctx->active) {
    ctx->fading = 1;
  }
  return CMP_SUCCESS;
}

int cmp_ink_ripple_get_state(cmp_ink_ripple_t *ripple, float *out_radius,
                             float *out_opacity, float *out_x, float *out_y) {
  struct cmp_ink_ripple *ctx = (struct cmp_ink_ripple *)ripple;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  if (out_radius)
    *out_radius = ctx->radius;
  if (out_opacity)
    *out_opacity = ctx->opacity;
  if (out_x)
    *out_x = ctx->origin_x;
  if (out_y)
    *out_y = ctx->origin_y;
  return CMP_SUCCESS;
}