/**
 * @file cupertino_spring.h
 * @brief Cupertino and Apple CASpringAnimation physics, rubber-band and detent
 * engine.
 */

#ifndef CUPERTINO_CUPERTINO_SPRING_H
#define CUPERTINO_CUPERTINO_SPRING_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief UIScrollView normal deceleration rate constant (0.998).
 */
#define CUPERTINO_SCROLL_DECELERATION_NORMAL 0.998f

/**
 * @brief UIScrollView fast deceleration rate constant (0.990).
 */
#define CUPERTINO_SCROLL_DECELERATION_FAST 0.990f

/**
 * @brief Apple standard rubber-band drag coefficient (~0.55).
 */
#define CUPERTINO_RUBBER_BAND_COEFF_DEFAULT 0.55f

/**
 * @enum cupertino_spring_preset
 * @brief Apple HIG and CASpringAnimation preset types.
 */
enum cupertino_spring_preset {
  CUPERTINO_SPRING_PRESET_DEFAULT = 0,
  CUPERTINO_SPRING_PRESET_BOUNCY,
  CUPERTINO_SPRING_PRESET_SNAPPY,
  CUPERTINO_SPRING_PRESET_INTERACTIVE,
  CUPERTINO_SPRING_PRESET_SHEET_DISMISS,
  CUPERTINO_SPRING_PRESET_COUNT
};

/**
 * @struct cupertino_spring_config
 * @brief Physical parameters for CASpringAnimation second-order differential
 * equation.
 */
struct cupertino_spring_config {
  float mass;             /**< Object mass m in kg (default 1.0f). */
  float stiffness;        /**< Spring stiffness k in N/m (default 100.0f). */
  float damping;          /**< Damping coefficient c (default 10.0f). */
  float initial_velocity; /**< Initial velocity v0 in pt/s. */
  float initial_position; /**< Initial displacement x0. */
  float target_position;  /**< Rest target displacement. */
};

/**
 * @brief Retrieves standard configuration presets matching Apple HIG physics.
 *
 * @param preset Preset animation curve type.
 * @param out_cfg Pointer to destination configuration structure.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_spring_get_preset(enum cupertino_spring_preset preset,
                            struct cupertino_spring_config *out_cfg);

/**
 * @brief Evaluates exact analytical solution of damped harmonic oscillator at
 * time t.
 *
 * Solves m * x'' + c * x' + k * x = 0 for underdamped, critically damped, and
 * overdamped regimes.
 *
 * @param cfg Spring configuration parameters.
 * @param elapsed_s Elapsed time in seconds since animation began (must be >=
 * 0).
 * @param out_position Pointer to receive calculated displacement at elapsed_s.
 * @param out_velocity Pointer to receive calculated velocity at elapsed_s.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_spring_evaluate(
    const struct cupertino_spring_config *cfg, float elapsed_s,
    float *out_position, float *out_velocity);

/**
 * @brief Calculates Apple rubber-band edge displacement under interactive drag.
 *
 * Evaluates f(x) = (x * d * c) / (d + c * x).
 *
 * @param distance Raw pan distance dragged past edge boundary (in points).
 * @param dimension Viewport dimension along drag axis (height or width in
 * points, must be > 0).
 * @param coefficient Drag resistance coefficient (typically 0.55f, must be >
 * 0).
 * @param out_offset Pointer to receive damped rubber-band displacement.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_rubber_band_offset(
    float distance, float dimension, float coefficient, float *out_offset);

/**
 * @brief Inverts Apple rubber-band offset back to raw drag displacement.
 *
 * Evaluates x = (f * d) / (c * (d - f)).
 *
 * @param offset Damped rubber-band displacement (must be < dimension).
 * @param dimension Viewport dimension along drag axis (must be > 0).
 * @param coefficient Drag resistance coefficient (must be > 0).
 * @param out_distance Pointer to receive reconstructed raw pan distance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_rubber_band_inverse(
    float offset, float dimension, float coefficient, float *out_distance);

/**
 * @brief Computes exponential momentum scroll decay over elapsed time.
 *
 * Evaluates v(t) = v0 * gamma^t and integrated displacement delta.
 *
 * @param v0 Initial velocity in pt/s.
 * @param deceleration_rate Deceleration constant gamma (0.990f to 0.999f).
 * @param elapsed_s Elapsed time in seconds.
 * @param out_position_delta Pointer to receive integrated position
 * displacement.
 * @param out_velocity Pointer to receive decayed instantaneous velocity.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_momentum_scroll(float v0, float deceleration_rate, float elapsed_s,
                          float *out_position_delta, float *out_velocity);

/**
 * @brief Finds nearest magnetic detent for sheet or picker snapping given
 * momentum.
 *
 * Projects resting position using x_proj = current_pos + v0 / (1 - gamma)
 * and selects the closest detent point.
 *
 * @param current_pos Current position in points.
 * @param velocity Current release gesture velocity in pt/s.
 * @param detents Array of target detent positions in points.
 * @param detent_count Number of elements in detents array (must be > 0).
 * @param deceleration_rate Deceleration constant gamma (e.g. 0.998f).
 * @param out_nearest_index Pointer to receive array index of the closest
 * detent.
 * @param out_target_pos Pointer to receive destination detent coordinate.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_detent_find_nearest(
    float current_pos, float velocity, const float *detents,
    size_t detent_count, float deceleration_rate, size_t *out_nearest_index,
    float *out_target_pos);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_SPRING_H */
