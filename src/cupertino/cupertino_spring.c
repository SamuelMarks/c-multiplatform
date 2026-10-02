/**
 * @file cupertino_spring.c
 * @brief Cupertino and Apple CASpringAnimation physics, rubber-band and detent
 * implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_spring.h"
#include <math.h>
/* clang-format on */

/**
 * @brief Retrieves standard configuration presets matching Apple HIG physics.
 */
ui_error_t
cupertino_spring_get_preset(enum cupertino_spring_preset preset,
                            struct cupertino_spring_config *out_cfg) {
  if (!out_cfg) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_cfg->mass = 1.0f;
  out_cfg->initial_velocity = 0.0f;
  out_cfg->initial_position = 0.0f;
  out_cfg->target_position = 1.0f;

  switch (preset) {
  case CUPERTINO_SPRING_PRESET_DEFAULT:
    out_cfg->stiffness = 100.0f;
    out_cfg->damping = 10.0f;
    break;

  case CUPERTINO_SPRING_PRESET_BOUNCY:
    out_cfg->stiffness = 180.0f;
    out_cfg->damping = 12.0f;
    break;

  case CUPERTINO_SPRING_PRESET_SNAPPY:
    out_cfg->stiffness = 300.0f;
    out_cfg->damping = 25.0f;
    break;

  case CUPERTINO_SPRING_PRESET_INTERACTIVE:
    out_cfg->stiffness = 250.0f;
    out_cfg->damping = 30.0f;
    break;

  case CUPERTINO_SPRING_PRESET_SHEET_DISMISS:
    out_cfg->stiffness = 200.0f;
    out_cfg->damping = 28.28427f; /* Critical damping: 2 * sqrt(1.0 * 200.0) */
    break;

  case CUPERTINO_SPRING_PRESET_COUNT:
  default:
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Evaluates exact analytical solution of damped harmonic oscillator at
 * time t.
 */
ui_error_t cupertino_spring_evaluate(const struct cupertino_spring_config *cfg,
                                     float elapsed_s, float *out_position,
                                     float *out_velocity) {
  float k;
  float m;
  float c;
  float zeta;
  float omega_0;
  float x0;
  float v0;
  float t;
  float pos;
  float vel;

  if (!cfg || !out_position || !out_velocity || cfg->mass <= 0.0f ||
      cfg->stiffness <= 0.0f || cfg->damping < 0.0f || elapsed_s < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  k = cfg->stiffness;
  m = cfg->mass;
  c = cfg->damping;
  zeta = c / (2.0f * (float)sqrt(m * k));
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
    /* Underdamped: zeta < 1 (expressive Apple bounce) */
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

/**
 * @brief Calculates Apple rubber-band edge displacement under interactive drag.
 */
ui_error_t cupertino_rubber_band_offset(float distance, float dimension,
                                        float coefficient, float *out_offset) {
  float sign;
  float abs_x;

  if (dimension <= 0.0f || coefficient <= 0.0f || !out_offset) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sign = (distance < 0.0f) ? -1.0f : 1.0f;
  abs_x = (float)fabs(distance);

  *out_offset = sign * ((abs_x * dimension * coefficient) /
                        (dimension + coefficient * abs_x));

  return UI_ERROR_NONE;
}

/**
 * @brief Inverts Apple rubber-band offset back to raw drag displacement.
 */
ui_error_t cupertino_rubber_band_inverse(float offset, float dimension,
                                         float coefficient,
                                         float *out_distance) {
  float sign;
  float abs_f;

  if (dimension <= 0.0f || coefficient <= 0.0f || !out_distance) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sign = (offset < 0.0f) ? -1.0f : 1.0f;
  abs_f = (float)fabs(offset);

  if (abs_f >= dimension) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_distance =
      sign * ((abs_f * dimension) / (coefficient * (dimension - abs_f)));

  return UI_ERROR_NONE;
}

/**
 * @brief Computes exponential momentum scroll decay over elapsed time.
 */
ui_error_t cupertino_momentum_scroll(float v0, float deceleration_rate,
                                     float elapsed_s, float *out_position_delta,
                                     float *out_velocity) {
  float gamma_t;
  float ln_gamma;

  if (deceleration_rate <= 0.0f || deceleration_rate >= 1.0f ||
      elapsed_s < 0.0f || !out_position_delta || !out_velocity) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  gamma_t = (float)pow(deceleration_rate, elapsed_s);
  ln_gamma = (float)log(deceleration_rate);

  *out_velocity = v0 * gamma_t;
  *out_position_delta = (v0 / ln_gamma) * (gamma_t - 1.0f);

  return UI_ERROR_NONE;
}

/**
 * @brief Finds nearest magnetic detent for sheet or picker snapping given
 * momentum.
 */
ui_error_t cupertino_detent_find_nearest(float current_pos, float velocity,
                                         const float *detents,
                                         size_t detent_count,
                                         float deceleration_rate,
                                         size_t *out_nearest_index,
                                         float *out_target_pos) {
  float projected_pos;
  float min_dist;
  size_t best_idx;
  size_t i;

  if (!detents || detent_count == 0 || deceleration_rate <= 0.0f ||
      deceleration_rate >= 1.0f || !out_nearest_index || !out_target_pos) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  projected_pos = current_pos + (velocity / (1.0f - deceleration_rate));

  best_idx = 0;
  min_dist = (float)fabs(detents[0] - projected_pos);

  for (i = 1; i < detent_count; ++i) {
    float dist = (float)fabs(detents[i] - projected_pos);
    if (dist < min_dist) {
      min_dist = dist;
      best_idx = i;
    }
  }

  *out_nearest_index = best_idx;
  *out_target_pos = detents[best_idx];

  return UI_ERROR_NONE;
}
