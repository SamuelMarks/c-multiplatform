/**
 * @file md3_spring.c
 * @brief Material 3 Expressive spring physics engine implementation.
 */

/* clang-format off */
#include "material3/md3_spring.h"
#include <math.h>
/* clang-format on */

/**
 * @brief Retrieves preset configuration for an Expressive spring token.
 *
 * @param token The spring token.
 * @param out_cfg Pointer to destination configuration.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_spring_get_config(enum md3_spring_token token,
                                 struct md3_spring_config *out_cfg) {
  if (!out_cfg) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if ((int)token < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_cfg->mass = 1.0f;
  out_cfg->initial_velocity = 0.0f;
  out_cfg->initial_position = 0.0f;
  out_cfg->target_position = 1.0f;

  switch (token) {
  case MD3_SPRING_TOKEN_FAST_SPATIAL:
    out_cfg->stiffness = 700.0f;
    out_cfg->damping_ratio = 0.9f;
    break;

  case MD3_SPRING_TOKEN_FAST_EFFECTS:
    out_cfg->stiffness = 800.0f;
    out_cfg->damping_ratio = 0.7f;
    break;

  case MD3_SPRING_TOKEN_SLOW_SPATIAL:
    out_cfg->stiffness = 300.0f;
    out_cfg->damping_ratio = 0.95f;
    break;

  case MD3_SPRING_TOKEN_SLOW_EFFECTS:
    out_cfg->stiffness = 200.0f;
    out_cfg->damping_ratio = 0.8f;
    break;

  case MD3_SPRING_TOKEN_BOUNCY:
    out_cfg->stiffness = 400.0f;
    out_cfg->damping_ratio = 0.5f;
    break;

  case MD3_SPRING_TOKEN_COUNT:
  default:
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Evaluates the exact second-order differential equation for a spring at
 * elapsed time.
 *
 * Handles underdamped (zeta < 1.0), critically damped (zeta == 1.0), and
 * overdamped (zeta > 1.0).
 *
 * @param cfg The spring configuration.
 * @param elapsed_s Elapsed time in seconds since animation start.
 * @param out_position Pointer to receive calculated position at elapsed_s.
 * @param out_velocity Pointer to receive calculated velocity at elapsed_s.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_spring_evaluate(const struct md3_spring_config *cfg,
                               float elapsed_s, float *out_position,
                               float *out_velocity) {
  float k;
  float m;
  float zeta;
  float omega_0;
  float x0;
  float v0;
  float t;
  float pos;
  float vel;

  if (!cfg || !out_position || !out_velocity || cfg->mass <= 0.0f ||
      cfg->stiffness <= 0.0f || cfg->damping_ratio < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (elapsed_s < 0.0f) {
    elapsed_s = 0.0f;
  }

  k = cfg->stiffness;
  m = cfg->mass;
  zeta = cfg->damping_ratio;
  omega_0 = (float)sqrt(k / m);
  x0 = cfg->initial_position - cfg->target_position;
  v0 = cfg->initial_velocity;
  t = elapsed_s;

  if (fabs(zeta - 1.0f) < 1e-4f) {
    /* Critically damped: zeta == 1 */
    float c1;
    float c2;
    float decay;

    c1 = x0;
    c2 = v0 + omega_0 * x0;
    decay = (float)exp(-omega_0 * t);

    pos = (c1 + c2 * t) * decay;
    vel = (c2 - omega_0 * (c1 + c2 * t)) * decay;
  } else if (zeta < 1.0f) {
    /* Underdamped: zeta < 1 */
    float omega_d;
    float decay;
    float c1;
    float c2;
    float sin_wt;
    float cos_wt;

    omega_d = omega_0 * (float)sqrt(1.0f - zeta * zeta);
    decay = (float)exp(-zeta * omega_0 * t);
    c1 = x0;
    c2 = (v0 + zeta * omega_0 * x0) / omega_d;

    sin_wt = (float)sin(omega_d * t);
    cos_wt = (float)cos(omega_d * t);

    pos = decay * (c1 * cos_wt + c2 * sin_wt);
    vel = -zeta * omega_0 * pos +
          decay * (-c1 * omega_d * sin_wt + c2 * omega_d * cos_wt);
  } else {
    /* Overdamped: zeta > 1 */
    float r1;
    float r2;
    float sqrt_term;
    float c1;
    float c2;
    float exp_r1;
    float exp_r2;

    sqrt_term = omega_0 * (float)sqrt(zeta * zeta - 1.0f);
    r1 = -zeta * omega_0 + sqrt_term;
    r2 = -zeta * omega_0 - sqrt_term;

    c2 = (v0 - r1 * x0) / (r2 - r1);
    c1 = x0 - c2;

    exp_r1 = (float)exp(r1 * t);
    exp_r2 = (float)exp(r2 * t);

    pos = c1 * exp_r1 + c2 * exp_r2;
    vel = c1 * r1 * exp_r1 + c2 * r2 * exp_r2;
  }

  *out_position = cfg->target_position + pos;
  *out_velocity = vel;

  return UI_ERROR_NONE;
}
