#ifndef CMP_ANIM_H
#define CMP_ANIM_H

/**
 * @file cmp_anim.h
 * @brief Animation timing and spring physics for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_math.h"

/** @brief Linear easing. */
#define CMP_ANIM_EASE_LINEAR 0
/** @brief Ease-in (quadratic). */
#define CMP_ANIM_EASE_IN 1
/** @brief Ease-out (quadratic). */
#define CMP_ANIM_EASE_OUT 2
/** @brief Ease-in-out (quadratic). */
#define CMP_ANIM_EASE_IN_OUT 3

/** @brief Controller idle mode. */
#define CMP_ANIM_MODE_NONE 0
/** @brief Controller timing mode. */
#define CMP_ANIM_MODE_TIMING 1
/** @brief Controller spring mode. */
#define CMP_ANIM_MODE_SPRING 2

/**
 * @brief Timing animation descriptor.
 */
typedef struct CMPAnimTiming {
  CMPScalar from;     /**< Start value. */
  CMPScalar to;       /**< End value. */
  CMPScalar duration; /**< Duration in seconds. */
  CMPScalar elapsed;  /**< Elapsed time in seconds. */
  cmp_u32 easing;     /**< Easing mode (CMP_ANIM_EASE_*). */
} CMPAnimTiming;

/**
 * @brief Spring simulation descriptor.
 */
typedef struct CMPSpring {
  CMPScalar position;      /**< Current position. */
  CMPScalar velocity;      /**< Current velocity. */
  CMPScalar target;        /**< Target position. */
  CMPScalar stiffness;     /**< Spring stiffness. */
  CMPScalar damping;       /**< Damping coefficient. */
  CMPScalar mass;          /**< Mass. */
  CMPScalar tolerance;     /**< Position tolerance for rest. */
  CMPScalar rest_velocity; /**< Velocity tolerance for rest. */
} CMPSpring;

/**
 * @brief Animation controller for timing or spring modes.
 */
typedef struct CMPAnimController {
  cmp_u32 mode;         /**< Active mode (CMP_ANIM_MODE_*). */
  CMPBool running;      /**< CMP_TRUE when the animation is running. */
  CMPScalar value;      /**< Current value. */
  CMPAnimTiming timing; /**< Timing state. */
  CMPSpring spring;     /**< Spring state. */
} CMPAnimController;

/**
 * @brief Initialize a timing animation.
 * @param timing Timing descriptor to initialize.
 * @param from Start value.
 * @param to End value.
 * @param duration Duration in seconds (>= 0).
 * @param easing Easing mode (CMP_ANIM_EASE_*).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_anim_timing_init(CMPAnimTiming *timing, CMPScalar from,
                                       CMPScalar to, CMPScalar duration,
                                       cmp_u32 easing);

/**
 * @brief Step a timing animation.
 * @param timing Timing descriptor.
 * @param dt Delta time in seconds (>= 0).
 * @param out_value Receives the sampled value.
 * @param out_finished Receives CMP_TRUE when finished.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_anim_timing_step(CMPAnimTiming *timing, CMPScalar dt,
                                       CMPScalar *out_value,
                                       CMPBool *out_finished);

/**
 * @brief Initialize a spring simulation.
 * @param spring Spring descriptor to initialize.
 * @param position Initial position.
 * @param target Target position.
 * @param stiffness Spring stiffness (> 0).
 * @param damping Damping coefficient (>= 0).
 * @param mass Mass (> 0).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_spring_init(CMPSpring *spring, CMPScalar position,
                                  CMPScalar target, CMPScalar stiffness,
                                  CMPScalar damping, CMPScalar mass);

/**
 * @brief Set a spring target.
 * @param spring Spring descriptor.
 * @param target New target position.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_spring_set_target(CMPSpring *spring, CMPScalar target);

/**
 * @brief Configure spring rest tolerances.
 * @param spring Spring descriptor.
 * @param tolerance Position tolerance (>= 0).
 * @param rest_velocity Velocity tolerance (>= 0).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_spring_set_tolerance(CMPSpring *spring, CMPScalar tolerance,
                                           CMPScalar rest_velocity);

/**
 * @brief Step a spring simulation.
 * @param spring Spring descriptor.
 * @param dt Delta time in seconds (>= 0).
 * @param out_finished Receives CMP_TRUE when the spring is at rest.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_spring_step(CMPSpring *spring, CMPScalar dt,
                                  CMPBool *out_finished);

/**
 * @brief Initialize an animation controller.
 * @param controller Controller to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_anim_controller_init(CMPAnimController *controller);

/**
 * @brief Start a timing animation on the controller.
 * @param controller Controller instance.
 * @param from Start value.
 * @param to End value.
 * @param duration Duration in seconds (>= 0).
 * @param easing Easing mode (CMP_ANIM_EASE_*).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_anim_controller_start_timing(CMPAnimController *controller,
                                                   CMPScalar from, CMPScalar to,
                                                   CMPScalar duration,
                                                   cmp_u32 easing);

/**
 * @brief Start a spring animation on the controller.
 * @param controller Controller instance.
 * @param position Initial position.
 * @param target Target position.
 * @param stiffness Spring stiffness (> 0).
 * @param damping Damping coefficient (>= 0).
 * @param mass Mass (> 0).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_anim_controller_start_spring(
    CMPAnimController *controller, CMPScalar position, CMPScalar target,
    CMPScalar stiffness, CMPScalar damping, CMPScalar mass);

/**
 * @brief Update the spring target for the controller.
 * @param controller Controller instance.
 * @param target New target value.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_anim_controller_set_spring_target(
    CMPAnimController *controller, CMPScalar target);

/**
 * @brief Step the controller.
 * @param controller Controller instance.
 * @param dt Delta time in seconds (>= 0).
 * @param out_value Receives the current value.
 * @param out_finished Receives CMP_TRUE when finished.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_anim_controller_step(CMPAnimController *controller,
                                           CMPScalar dt, CMPScalar *out_value,
                                           CMPBool *out_finished);

/**
 * @brief Retrieve the current controller value.
 * @param controller Controller instance.
 * @param out_value Receives the current value.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_anim_controller_get_value(
    const CMPAnimController *controller, CMPScalar *out_value);

/**
 * @brief Query whether the controller is running.
 * @param controller Controller instance.
 * @param out_running Receives CMP_TRUE when running.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_anim_controller_is_running(
    const CMPAnimController *controller, CMPBool *out_running);

/**
 * @brief Stop the controller.
 * @param controller Controller instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_anim_controller_stop(CMPAnimController *controller);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for easing evaluation.
 * @param easing Easing mode (CMP_ANIM_EASE_*).
 * @param t Normalized time (clamped to 0..1).
 * @param out_value Receives the eased value.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_anim_test_apply_ease(cmp_u32 easing, CMPScalar t,
                                           CMPScalar *out_value);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_ANIM_H */
