/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cmp_log.h"
/* clang-format on */

struct cmp_spring_ease {
  float mass;
  float stiffness;
  float damping;
  float initial_velocity;
};

/**
 * @brief Create a spring ease.
 *
 * @param mass Parameter description.
 * @param stiffness Parameter description.
 * @param damping Parameter description.
 * @param initial_velocity Parameter description.
 * @param out_spring Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_spring_ease_create(float mass, float stiffness, float damping,
                           float initial_velocity,
                           cmp_spring_ease_t **out_spring) {
  int rc = CMP_SUCCESS;
  struct cmp_spring_ease *spring;

  rc = CMP_SUCCESS;

  if (out_spring == NULL || mass <= 0.0f || stiffness <= 0.0f ||
      damping < 0.0f) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_spring_ease), (void **)&spring);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  spring->mass = mass;
  spring->stiffness = stiffness;
  spring->damping = damping;
  spring->initial_velocity = initial_velocity;

  *out_spring = (cmp_spring_ease_t *)spring;
  return rc;
}

/**
 * @brief Destroy a spring ease.
 *
 * @param spring Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_spring_ease_destroy(cmp_spring_ease_t *spring) {
  int rc = CMP_SUCCESS;
  struct cmp_spring_ease *internal_spring;

  rc = CMP_SUCCESS;
  internal_spring = (struct cmp_spring_ease *)spring;

  if (internal_spring == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(internal_spring);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return rc;
}

/**
 * @brief Evaluate a spring ease.
 *
 * @param spring Parameter description.
 * @param t Parameter description.
 * @param out_value Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_spring_ease_evaluate(cmp_spring_ease_t *spring, double t,
                             float *out_value) {
  int rc = CMP_SUCCESS;
  struct cmp_spring_ease *s;
  float w0;
  float zeta;
  float envelope;

  s = (struct cmp_spring_ease *)spring;

  if (s == NULL || out_value == NULL || t < 0.0) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Simplified damped harmonic oscillator math */
  w0 = (float)sqrt((double)(s->stiffness / s->mass));
  zeta = s->damping / (2.0f * (float)sqrt((double)(s->stiffness * s->mass)));

  if (zeta < 1.0f) {
    /* Under-damped */
    float wd = w0 * (float)sqrt((double)(1.0f - zeta * zeta));
    float A = 1.0f;
    float B = (zeta * w0 + s->initial_velocity) / wd;
    envelope = (float)exp((double)(-zeta * w0 * (float)t)) *
               (A * (float)cos((double)(wd * (float)t)) +
                B * (float)sin((double)(wd * (float)t)));
  } else {
    /* Critically or over-damped approximation */
    float A = 1.0f;
    float B = s->initial_velocity + w0;
    envelope = (float)exp((double)(-w0 * (float)t)) * (A + B * (float)t);
  }

  *out_value = 1.0f - envelope;

  return rc;
}
