/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

struct cmp_spring_ease {
  float mass;
  float stiffness;
  float damping;
  float initial_velocity;
};

int cmp_spring_ease_create(float mass, float stiffness, float damping,
                           float initial_velocity,
                           cmp_spring_ease_t **out_spring) {
  struct cmp_spring_ease *spring;

  if (!out_spring || mass <= 0.0f || stiffness <= 0.0f || damping < 0.0f)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_spring_ease), (void **)&spring) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  spring->mass = mass;
  spring->stiffness = stiffness;
  spring->damping = damping;
  spring->initial_velocity = initial_velocity;

  *out_spring = (cmp_spring_ease_t *)spring;
  return CMP_SUCCESS;
}

int cmp_spring_ease_destroy(cmp_spring_ease_t *spring) {
  struct cmp_spring_ease *internal_spring = (struct cmp_spring_ease *)spring;
  if (!internal_spring)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_spring);
  return CMP_SUCCESS;
}

int cmp_spring_ease_evaluate(cmp_spring_ease_t *spring, double t,
                             float *out_value) {
  struct cmp_spring_ease *s = (struct cmp_spring_ease *)spring;
  float w0;
  float zeta;
  float envelope;

  if (!s || !out_value || t < 0.0)
    return CMP_ERROR_INVALID_ARG;

  /* Simplified damped harmonic oscillator math */
  w0 = (float)sqrt(s->stiffness / s->mass);
  zeta = s->damping / (2.0f * (float)sqrt(s->stiffness * s->mass));

  if (zeta < 1.0f) {
    /* Under-damped */
    float wd = w0 * (float)sqrt(1.0f - zeta * zeta);
    float A = 1.0f;
    float B = (zeta * w0 + s->initial_velocity) / wd;
    envelope = (float)exp(-zeta * w0 * t) *
               (A * (float)cos(wd * t) + B * (float)sin(wd * t));
  } else {
    /* Critically or over-damped approximation */
    float A = 1.0f;
    float B = s->initial_velocity + w0;
    envelope = (float)exp(-w0 * t) * (A + B * (float)t);
  }

  *out_value = 1.0f - envelope;
  return CMP_SUCCESS;
}