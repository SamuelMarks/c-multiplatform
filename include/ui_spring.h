#ifndef UI_SPRING_H
#define UI_SPRING_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Spring configuration based on damping ratio and frequency.
 */
struct ui_spring_config {
  float damping;   /**< Damping ratio (zeta). 1.0 is critically damped. */
  float stiffness; /**< Stiffness (k). */
  float mass;      /**< Mass (m). */
};

/**
 * @brief Current state of the spring.
 */
struct ui_spring_state {
  float value;
  float velocity;
};

/**
 * @brief Calculates the next state of a spring using semi-implicit Euler
 * integration.
 *
 * @param config The spring configuration.
 * @param current The current state.
 * @param target The target rest value.
 * @param delta_time_s Time elapsed since last update in seconds.
 * @param out_state Pointer to receive the new state.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_spring_update(const struct ui_spring_config *config,
                               const struct ui_spring_state *current,
                               float target, float delta_time_s,
                               struct ui_spring_state *out_state);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SPRING_H */
