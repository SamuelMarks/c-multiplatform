#ifndef M3_STEPPER_H
#define M3_STEPPER_H

/**
 * @file m3_stepper.h
 * @brief Material 3 Stepper (Multi-step flow indicator).
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_text.h"

/**
 * @brief State for an individual step in a stepper.
 */
typedef enum M3StepState {
  M3_STEP_STATE_INCOMPLETE = 0,
  M3_STEP_STATE_ACTIVE = 1,
  M3_STEP_STATE_COMPLETE = 2,
  M3_STEP_STATE_ERROR = 3
} M3StepState;

/**
 * @brief Definition of an individual step.
 */
typedef struct M3Step {
  const char* title;       /**< Step title text. */
  const char* subtitle;    /**< Optional step subtitle text. */
  M3StepState state;       /**< Current state of the step. */
} M3Step;

/**
 * @brief State for a stepper widget.
 */
typedef struct M3Stepper {
  CMPRect bounds;          /**< Bounding rectangle for the stepper. */
  M3Step* steps;           /**< Array of steps. */
  cmp_u32 step_count;      /**< Total number of steps. */
  cmp_u32 current_step;    /**< Currently active step index (0-based). */
  CMPBool vertical;        /**< CMP_TRUE for vertical layout, CMP_FALSE for horizontal. */
} M3Stepper;

/**
 * @brief Initialize a new stepper.
 * @param stepper The stepper to initialize.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL m3_stepper_init(M3Stepper* stepper);

/**
 * @brief Render the stepper.
 * @param ctx The paint context.
 * @param stepper The stepper definition.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_stepper_draw(CMPPaintContext* ctx, const M3Stepper* stepper);

/**
 * @brief Free resources associated with a stepper.
 * @param stepper The stepper to cleanup.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_stepper_cleanup(M3Stepper* stepper);

#ifdef __cplusplus
}
#endif

#endif /* M3_STEPPER_H */