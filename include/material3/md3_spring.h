/**
 * @file md3_spring.h
 * @brief Material 3 Expressive spring physics engine.
 */

#ifndef MATERIAL3_MD3_SPRING_H
#define MATERIAL3_MD3_SPRING_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum md3_spring_token
 * @brief Standard Material 3 Expressive spring animation tokens.
 */
enum md3_spring_token {
  MD3_SPRING_TOKEN_FAST_SPATIAL = 0,
  MD3_SPRING_TOKEN_FAST_EFFECTS,
  MD3_SPRING_TOKEN_SLOW_SPATIAL,
  MD3_SPRING_TOKEN_SLOW_EFFECTS,
  MD3_SPRING_TOKEN_BOUNCY,
  MD3_SPRING_TOKEN_COUNT
};

/**
 * @struct md3_spring_config
 * @brief Spring physical parameters for exact second-order differential
 * simulation.
 */
struct md3_spring_config {
  float stiffness;        /**< Spring stiffness k (N/m), default ~ 300.0f */
  float damping_ratio;    /**< Damping ratio zeta (1.0 = critical, <1.0 =
                             underdamped/bouncy, >1.0 = overdamped) */
  float mass;             /**< Object mass m (kg), default 1.0f */
  float initial_velocity; /**< Initial velocity v0 (m/s) */
  float initial_position; /**< Initial position x0 */
  float target_position;  /**< Target rest position */
};

/**
 * @brief Retrieves preset configuration for an Expressive spring token.
 *
 * @param token The spring token.
 * @param out_cfg Pointer to destination configuration.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_spring_get_config(
    enum md3_spring_token token, struct md3_spring_config *out_cfg);

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
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_spring_evaluate(const struct md3_spring_config *cfg, float elapsed_s,
                    float *out_position, float *out_velocity);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_SPRING_H */
