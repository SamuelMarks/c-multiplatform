/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

struct cmp_rubber_band {
  float current_position;
  float velocity;
  float stiffness;
  float damping;
  int is_resting;
};

int cmp_rubber_band_create(cmp_rubber_band_t **out_band) {
  struct cmp_rubber_band *ctx;

  if (!out_band)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_rubber_band), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(ctx, 0, sizeof(struct cmp_rubber_band));
  ctx->stiffness = 200.0f; /* Default spring stiffness */
  ctx->damping = 20.0f;    /* Default spring damping */
  ctx->is_resting = 1;

  *out_band = (cmp_rubber_band_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_rubber_band_destroy(cmp_rubber_band_t *band) {
  struct cmp_rubber_band *ctx = (struct cmp_rubber_band *)band;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_rubber_band_start(cmp_rubber_band_t *band, float initial_velocity,
                          float current_position) {
  struct cmp_rubber_band *ctx = (struct cmp_rubber_band *)band;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->velocity = initial_velocity;
  ctx->current_position = current_position;
  ctx->is_resting = 0;

  return CMP_SUCCESS;
}

int cmp_rubber_band_step(cmp_rubber_band_t *band, float dt_ms,
                         float target_rest_position,
                         float *out_current_position, int *out_is_resting) {
  struct cmp_rubber_band *ctx = (struct cmp_rubber_band *)band;
  float dt_sec, spring_force, damping_force, acceleration;

  if (!ctx || !out_current_position || !out_is_resting)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->is_resting) {
    *out_current_position = ctx->current_position;
    *out_is_resting = 1;
    return CMP_SUCCESS;
  }

  dt_sec = dt_ms / 1000.0f;
  if (dt_sec <= 0.0f)
    dt_sec = 0.016f; /* Fallback to 60fps if zero */

  /* Hooke's Law: F = -k * x - c * v */
  spring_force =
      -ctx->stiffness * (ctx->current_position - target_rest_position);
  damping_force = -ctx->damping * ctx->velocity;

  acceleration = spring_force + damping_force;

  /* Explicit Euler integration */
  ctx->velocity += acceleration * dt_sec;
  ctx->current_position += ctx->velocity * dt_sec;

  /* Check rest condition (very close to target and almost stopped) */
  if (fabs((double)(ctx->current_position - target_rest_position)) < 0.5 &&
      fabs((double)ctx->velocity) < 5.0) {
    ctx->current_position = target_rest_position;
    ctx->velocity = 0.0f;
    ctx->is_resting = 1;
  }

  *out_current_position = ctx->current_position;
  *out_is_resting = ctx->is_resting;

  return CMP_SUCCESS;
}
