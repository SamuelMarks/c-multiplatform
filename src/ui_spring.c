/* clang-format off */
#include "../include/ui_spring.h"
/* clang-format on */

/**
 * @brief ui_spring_update.
 * @param config Parameter config.
 * @param current Parameter current.
 * @param target Parameter target.
 * @param delta_time_s Parameter delta_time_s.
 * @param out_state Parameter out_state.
 * @return Return value.
 */
ui_error_t ui_spring_update(const struct ui_spring_config *config,
                            const struct ui_spring_state *current, float target,
                            float delta_time_s,
                            struct ui_spring_state *out_state) {
  float force;
  float acceleration;

  if (!config || !current || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (config->mass <= 0.0f) {
    *out_state = *current;
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (delta_time_s <= 0.0f) {
    *out_state = *current;
    return UI_ERROR_NONE;
  }

  /* F = -k * x - c * v */
  /* c = damping * 2 * sqrt(mass * stiffness) */
  /* Using simplified direct stiffness and damping values for config for now,
   * assuming config->damping is the 'c' constant and stiffness is 'k' */
  force = -config->stiffness * (current->value - target) -
          config->damping * current->velocity;

  acceleration = force / config->mass;

  /* Semi-implicit Euler integration */
  out_state->velocity = current->velocity + acceleration * delta_time_s;
  out_state->value = current->value + out_state->velocity * delta_time_s;

  return UI_ERROR_NONE;
}
