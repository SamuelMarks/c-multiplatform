#ifndef M3_ANIM_H
#define M3_ANIM_H

/**
 * @file m3_anim.h
 * @brief Animation timing and spring physics for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_math.h"

/** @brief Linear easing. */
#define M3_ANIM_EASE_LINEAR 0
/** @brief Ease-in (quadratic). */
#define M3_ANIM_EASE_IN 1
/** @brief Ease-out (quadratic). */
#define M3_ANIM_EASE_OUT 2
/** @brief Ease-in-out (quadratic). */
#define M3_ANIM_EASE_IN_OUT 3

/** @brief Controller idle mode. */
#define M3_ANIM_MODE_NONE 0
/** @brief Controller timing mode. */
#define M3_ANIM_MODE_TIMING 1
/** @brief Controller spring mode. */
#define M3_ANIM_MODE_SPRING 2

/**
 * @brief Timing animation descriptor.
 */
typedef struct M3AnimTiming {
  M3Scalar from;     /**< Start value. */
  M3Scalar to;       /**< End value. */
  M3Scalar duration; /**< Duration in seconds. */
  M3Scalar elapsed;  /**< Elapsed time in seconds. */
  m3_u32 easing;     /**< Easing mode (M3_ANIM_EASE_*). */
} M3AnimTiming;

/**
 * @brief Spring simulation descriptor.
 */
typedef struct M3Spring {
  M3Scalar position;      /**< Current position. */
  M3Scalar velocity;      /**< Current velocity. */
  M3Scalar target;        /**< Target position. */
  M3Scalar stiffness;     /**< Spring stiffness. */
  M3Scalar damping;       /**< Damping coefficient. */
  M3Scalar mass;          /**< Mass. */
  M3Scalar tolerance;     /**< Position tolerance for rest. */
  M3Scalar rest_velocity; /**< Velocity tolerance for rest. */
} M3Spring;

/**
 * @brief Animation controller for timing or spring modes.
 */
typedef struct M3AnimController {
  m3_u32 mode;         /**< Active mode (M3_ANIM_MODE_*). */
  M3Bool running;      /**< M3_TRUE when the animation is running. */
  M3Scalar value;      /**< Current value. */
  M3AnimTiming timing; /**< Timing state. */
  M3Spring spring;     /**< Spring state. */
} M3AnimController;

/**
 * @brief Initialize a timing animation.
 * @param timing Timing descriptor to initialize.
 * @param from Start value.
 * @param to End value.
 * @param duration Duration in seconds (>= 0).
 * @param easing Easing mode (M3_ANIM_EASE_*).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_anim_timing_init(M3AnimTiming *timing, M3Scalar from,
                                       M3Scalar to, M3Scalar duration,
                                       m3_u32 easing);

/**
 * @brief Step a timing animation.
 * @param timing Timing descriptor.
 * @param dt Delta time in seconds (>= 0).
 * @param out_value Receives the sampled value.
 * @param out_finished Receives M3_TRUE when finished.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_anim_timing_step(M3AnimTiming *timing, M3Scalar dt,
                                       M3Scalar *out_value,
                                       M3Bool *out_finished);

/**
 * @brief Initialize a spring simulation.
 * @param spring Spring descriptor to initialize.
 * @param position Initial position.
 * @param target Target position.
 * @param stiffness Spring stiffness (> 0).
 * @param damping Damping coefficient (>= 0).
 * @param mass Mass (> 0).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_spring_init(M3Spring *spring, M3Scalar position,
                                  M3Scalar target, M3Scalar stiffness,
                                  M3Scalar damping, M3Scalar mass);

/**
 * @brief Set a spring target.
 * @param spring Spring descriptor.
 * @param target New target position.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_spring_set_target(M3Spring *spring, M3Scalar target);

/**
 * @brief Configure spring rest tolerances.
 * @param spring Spring descriptor.
 * @param tolerance Position tolerance (>= 0).
 * @param rest_velocity Velocity tolerance (>= 0).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_spring_set_tolerance(M3Spring *spring, M3Scalar tolerance,
                                           M3Scalar rest_velocity);

/**
 * @brief Step a spring simulation.
 * @param spring Spring descriptor.
 * @param dt Delta time in seconds (>= 0).
 * @param out_finished Receives M3_TRUE when the spring is at rest.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_spring_step(M3Spring *spring, M3Scalar dt,
                                  M3Bool *out_finished);

/**
 * @brief Initialize an animation controller.
 * @param controller Controller to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_anim_controller_init(M3AnimController *controller);

/**
 * @brief Start a timing animation on the controller.
 * @param controller Controller instance.
 * @param from Start value.
 * @param to End value.
 * @param duration Duration in seconds (>= 0).
 * @param easing Easing mode (M3_ANIM_EASE_*).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_anim_controller_start_timing(M3AnimController *controller,
                                                   M3Scalar from, M3Scalar to,
                                                   M3Scalar duration,
                                                   m3_u32 easing);

/**
 * @brief Start a spring animation on the controller.
 * @param controller Controller instance.
 * @param position Initial position.
 * @param target Target position.
 * @param stiffness Spring stiffness (> 0).
 * @param damping Damping coefficient (>= 0).
 * @param mass Mass (> 0).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_anim_controller_start_spring(
    M3AnimController *controller, M3Scalar position, M3Scalar target,
    M3Scalar stiffness, M3Scalar damping, M3Scalar mass);

/**
 * @brief Update the spring target for the controller.
 * @param controller Controller instance.
 * @param target New target value.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_anim_controller_set_spring_target(
    M3AnimController *controller, M3Scalar target);

/**
 * @brief Step the controller.
 * @param controller Controller instance.
 * @param dt Delta time in seconds (>= 0).
 * @param out_value Receives the current value.
 * @param out_finished Receives M3_TRUE when finished.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_anim_controller_step(M3AnimController *controller,
                                           M3Scalar dt, M3Scalar *out_value,
                                           M3Bool *out_finished);

/**
 * @brief Retrieve the current controller value.
 * @param controller Controller instance.
 * @param out_value Receives the current value.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_anim_controller_get_value(
    const M3AnimController *controller, M3Scalar *out_value);

/**
 * @brief Query whether the controller is running.
 * @param controller Controller instance.
 * @param out_running Receives M3_TRUE when running.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_anim_controller_is_running(
    const M3AnimController *controller, M3Bool *out_running);

/**
 * @brief Stop the controller.
 * @param controller Controller instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_anim_controller_stop(M3AnimController *controller);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for easing evaluation.
 * @param easing Easing mode (M3_ANIM_EASE_*).
 * @param t Normalized time (clamped to 0..1).
 * @param out_value Receives the eased value.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_anim_test_apply_ease(m3_u32 easing, M3Scalar t,
                                           M3Scalar *out_value);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_ANIM_H */
