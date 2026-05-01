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

/**
 * @brief Creates a new rubber band spring physics context.
 *
 * @param out_band Pointer to store the newly created context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_rubber_band_create(cmp_rubber_band_t **out_band) {
  int rc = CMP_SUCCESS;
  struct cmp_rubber_band *ctx;

  if (!out_band)
    return CMP_ERROR_INVALID_ARG;

  rc = CMP_MALLOC(sizeof(struct cmp_rubber_band), (void **)&ctx);
  if (rc != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(ctx, 0, sizeof(struct cmp_rubber_band));
  ctx->stiffness = 200.0f; /* Default spring stiffness */
  ctx->damping = 20.0f;    /* Default spring damping */
  ctx->is_resting = 1;

  *out_band = (cmp_rubber_band_t *)ctx;
  return rc;
}

/**
 * @brief Destroys a rubber band context and frees its resources.
 *
 * @param band Pointer to the rubber band context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_rubber_band_destroy(cmp_rubber_band_t *band) {
  int rc = CMP_SUCCESS;
  struct cmp_rubber_band *ctx = (struct cmp_rubber_band *)band;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    return rc;
  }
  return rc;
}

/**
 * @brief Initializes the rubber band physics state with an initial velocity and
 * position.
 *
 * @param band Pointer to the rubber band context.
 * @param initial_velocity The starting velocity (e.g., from a flick gesture).
 * @param current_position The starting position.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_rubber_band_start(cmp_rubber_band_t *band, float initial_velocity,
                          float current_position) {
  int rc = CMP_SUCCESS;
  struct cmp_rubber_band *ctx = (struct cmp_rubber_band *)band;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->velocity = initial_velocity;
  ctx->current_position = current_position;
  ctx->is_resting = 0;
  return rc;
}

/**
 * @brief Steps the rubber band physics simulation forward by a delta time.
 *
 * @param band Pointer to the rubber band context.
 * @param dt_ms Delta time in milliseconds since the last step.
 * @param target_rest_position The target position the spring is pulling
 * towards.
 * @param out_current_position Pointer to store the new calculated position.
 * @param out_is_resting Pointer to store a boolean indicating if the spring has
 * come to rest (1) or is still moving (0).
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_rubber_band_step(cmp_rubber_band_t *band, float dt_ms,
                         float target_rest_position,
                         float *out_current_position, int *out_is_resting) {
  int rc = CMP_SUCCESS;
  struct cmp_rubber_band *ctx = (struct cmp_rubber_band *)band;
  float dt_sec, spring_force, damping_force, acceleration;

  if (!ctx || !out_current_position || !out_is_resting)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->is_resting) {
    *out_current_position = ctx->current_position;
    *out_is_resting = 1;
    return rc;
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
  return rc;
}
