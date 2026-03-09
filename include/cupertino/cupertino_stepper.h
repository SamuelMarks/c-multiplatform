#ifndef CUPERTINO_STEPPER_H
#define CUPERTINO_STEPPER_H

/**
 * @file cupertino_stepper.h
 * @brief Apple Cupertino style Stepper component (+/-).
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
#include "cupertino/cupertino_color.h"

/** @brief Cupertino Stepper Widget */
typedef struct CupertinoStepper {
  CMPWidget widget;     /**< Base widget interface. */
  double value;         /**< Current value. */
  double min_value;     /**< Minimum value. */
  double max_value;     /**< Maximum value. */
  double step_value;    /**< Value to increment/decrement by. */
  CMPRect bounds;       /**< Layout bounds. */
  CMPBool is_dark_mode; /**< Dark mode styling. */
  CMPBool is_disabled;  /**< Disabled state. */
  int pressed_button;   /**< Which button is pressed (-1: none, 0: minus, 1:
                           plus). */
} CupertinoStepper;

/**
 * @brief Initializes a Cupertino Stepper.
 * @param stepper Pointer to the stepper instance.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_stepper_init(CupertinoStepper *stepper);

/**
 * @brief Sets the current value of the stepper.
 * @param stepper Pointer to the stepper instance.
 * @param value The value to set.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_stepper_set_value(CupertinoStepper *stepper,
                                                 double value);

/**
 * @brief Renders the stepper.
 * @param stepper Pointer to the stepper instance.
 * @param ctx Pointer to the paint context.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_stepper_paint(const CupertinoStepper *stepper,
                                             CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_STEPPER_H */
