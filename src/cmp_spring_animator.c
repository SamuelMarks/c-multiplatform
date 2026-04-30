/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <math.h>

#include "cmp_log.h"
/* clang-format on */

struct cmp_spring_animator {
  float mass;
  float stiffness;
  float damping;
  float initial_velocity;

  float current_value;
  float target_value;
  float current_velocity;

  int is_scrubbing;
  float scrub_fraction;
};

/**
 * @brief Create a spring animator.
 *
 * @param mass Parameter description.
 * @param stiffness Parameter description.
 * @param damping Parameter description.
 * @param initial_velocity Parameter description.
 * @param out_animator Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_spring_animator_create(float mass, float stiffness, float damping,
                               float initial_velocity,
                               cmp_spring_animator_t **out_animator) {
  int rc = CMP_SUCCESS;
  struct cmp_spring_animator *anim;

  rc = CMP_SUCCESS;

  if (out_animator == NULL) {
    LOG_DEBUG("Invalid argument: out_animator is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_spring_animator), (void **)&anim);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  anim->mass = mass > 0.0f ? mass : 1.0f;
  anim->stiffness = stiffness > 0.0f ? stiffness : 100.0f;
  anim->damping = damping > 0.0f ? damping : 10.0f;
  anim->initial_velocity = initial_velocity;

  anim->current_value = 0.0f;
  anim->target_value = 1.0f; /* normalized distance */
  anim->current_velocity = initial_velocity;

  anim->is_scrubbing = 0;
  anim->scrub_fraction = 0.0f;

  *out_animator = (cmp_spring_animator_t *)anim;
  return rc;
}

/**
 * @brief Destroy a spring animator.
 *
 * @param animator Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_spring_animator_destroy(cmp_spring_animator_t *animator) {
  int rc = CMP_SUCCESS;
  struct cmp_spring_animator *anim;

  rc = CMP_SUCCESS;
  anim = (struct cmp_spring_animator *)animator;

  if (anim == NULL) {
    LOG_DEBUG("Invalid argument: animator is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(anim);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return rc;
}

/**
 * @brief Interrupt a spring animator.
 *
 * @param animator Parameter description.
 * @param new_target_value Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_spring_animator_interrupt(cmp_spring_animator_t *animator,
                                  float new_target_value) {
  int rc = CMP_SUCCESS;
  struct cmp_spring_animator *anim;

  anim = (struct cmp_spring_animator *)animator;

  if (anim == NULL) {
    LOG_DEBUG("Invalid argument: animator is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Hand-off: we don't reset current_value or current_velocity, just the target
   */
  anim->target_value = new_target_value;
  anim->is_scrubbing = 0;

  return rc;
}

/**
 * @brief Scrub a spring animator.
 *
 * @param animator Parameter description.
 * @param fraction_complete Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_spring_animator_scrub(cmp_spring_animator_t *animator,
                              float fraction_complete) {
  int rc = CMP_SUCCESS;
  struct cmp_spring_animator *anim;

  anim = (struct cmp_spring_animator *)animator;

  if (anim == NULL) {
    LOG_DEBUG("Invalid argument: animator is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  anim->is_scrubbing = 1;
  anim->scrub_fraction = fraction_complete;

  if (anim->scrub_fraction < 0.0f) {
    anim->scrub_fraction = 0.0f;
  }
  if (anim->scrub_fraction > 1.0f) {
    anim->scrub_fraction = 1.0f;
  }

  /* In scrub mode, velocity is artificially clamped or tracked separately,
     but for this equivalency we just freeze the time step evaluation. */
  return rc;
}

/**
 * @brief Evaluate a spring animator.
 *
 * @param animator Parameter description.
 * @param dt_seconds Parameter description.
 * @param out_current_value Parameter description.
 * @param out_is_settled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_spring_animator_evaluate(cmp_spring_animator_t *animator,
                                 float dt_seconds, float *out_current_value,
                                 int *out_is_settled) {
  int rc = CMP_SUCCESS;
  struct cmp_spring_animator *anim;
  float displacement, spring_force, damping_force, acceleration;

  anim = (struct cmp_spring_animator *)animator;

  if (anim == NULL || out_current_value == NULL || out_is_settled == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (anim->is_scrubbing) {
    /* If scrubbed, value is linearly forced, it is never "settled" physically
     */
    anim->current_value = anim->scrub_fraction * anim->target_value;
    *out_current_value = anim->current_value;
    *out_is_settled = 0;
    return rc;
  }

  /* Hooke's Law: F = -k*x - c*v */
  displacement = anim->current_value - anim->target_value;
  spring_force = -anim->stiffness * displacement;
  damping_force = -anim->damping * anim->current_velocity;

  acceleration = (spring_force + damping_force) / anim->mass;

  /* Euler integration */
  anim->current_velocity += acceleration * dt_seconds;
  anim->current_value += anim->current_velocity * dt_seconds;

  *out_current_value = anim->current_value;

  /* Settled if velocity is near zero AND displacement is near zero */
  if (fabs((double)anim->current_velocity) < 0.001 &&
      fabs((double)(anim->current_value - anim->target_value)) < 0.001) {
    *out_is_settled = 1;
    anim->current_value = anim->target_value; /* Snap to perfect 1.0/target */
  } else {
    *out_is_settled = 0;
  }

  return rc;
}

/**
 * @brief Calculate gesture velocity.
 *
 * @param delta_x Parameter description.
 * @param delta_y Parameter description.
 * @param dt_seconds Parameter description.
 * @param out_velocity_x Parameter description.
 * @param out_velocity_y Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_spring_calculate_gesture_velocity(float delta_x, float delta_y,
                                          float dt_seconds,
                                          float *out_velocity_x,
                                          float *out_velocity_y) {
  int rc = CMP_SUCCESS;
  if (out_velocity_x == NULL || out_velocity_y == NULL || dt_seconds <= 0.0f) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_velocity_x = delta_x / dt_seconds;
  *out_velocity_y = delta_y / dt_seconds;

  return rc;
}
