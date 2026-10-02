/**
 * @file cupertino_stepper.h
 * @brief Cupertino Stepper component wrapping ui_spin_button_base with Apple
 * HIG segmented pill styling.
 */

#ifndef CUPERTINO_CUPERTINO_STEPPER_H
#define CUPERTINO_CUPERTINO_STEPPER_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_spin_button_base.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @brief Standard Apple HIG Stepper width (94pt).
 */
#define CUPERTINO_STEPPER_WIDTH 94.0f
/**
 * @brief Standard Apple HIG Stepper height (32pt).
 */
#define CUPERTINO_STEPPER_HEIGHT 32.0f
/**
 * @brief Standard Apple HIG Stepper corner radius (8pt).
 */
#define CUPERTINO_STEPPER_CORNER_RADIUS 8.0f

/**
 * @struct cupertino_stepper
 * @brief Cupertino Stepper segmented pill control wrapping ui_spin_button_base.
 */
struct cupertino_stepper {
  struct ui_spin_button_base *base; /**< Wrapped CDK spin button primitive. */
  struct ui_control_value_accessor
      cva;                     /**< Control Value Accessor interface. */
  double min;                  /**< Minimum allowed value. */
  double max;                  /**< Maximum allowed value. */
  double step;                 /**< Step increment/decrement amount. */
  double value;                /**< Current numeric value. */
  int is_disabled;             /**< 1 if disabled, 0 if enabled. */
  int auto_repeat;             /**< 1 if holding accelerates/repeats. */
  ui_color_t background_color; /**< Segmented pill background (SystemGray5). */
  ui_color_t tint_color;       /**< Minus and plus glyph color (SystemBlue). */
  ui_color_t separator_color;  /**< Hairline divider color (Separator). */
};

/**
 * @brief Creates a new Cupertino Stepper component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param min Minimum possible value.
 * @param max Maximum possible value.
 * @param step Step amount per press/tick.
 * @param out_stepper Pointer to receive newly created stepper.
 * @param out_cva Optional pointer to receive Control Value Accessor.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_stepper_create(struct ui_engine *engine, double min, double max,
                         double step, struct cupertino_stepper **out_stepper,
                         struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Cupertino stepper and releases all resources.
 *
 * @param stepper The stepper to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_stepper_destroy(struct cupertino_stepper *stepper);

/**
 * @brief Sets current stepper value (clamped between min and max).
 *
 * @param stepper The stepper.
 * @param value New value.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_stepper_set_value(struct cupertino_stepper *stepper, double value);

/**
 * @brief Queries current stepper value.
 *
 * @param stepper The stepper.
 * @param out_value Pointer to receive current value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_stepper_get_value(
    const struct cupertino_stepper *stepper, double *out_value);

/**
 * @brief Increments current stepper value by step.
 *
 * @param stepper The stepper.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_stepper_increment(struct cupertino_stepper *stepper);

/**
 * @brief Decrements current stepper value by step.
 *
 * @param stepper The stepper.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_stepper_decrement(struct cupertino_stepper *stepper);

/**
 * @brief Sets step amount.
 *
 * @param stepper The stepper.
 * @param step New step increment.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_stepper_set_step(struct cupertino_stepper *stepper, double step);

/**
 * @brief Sets minimum and maximum range bounds.
 *
 * @param stepper The stepper.
 * @param min Minimum value.
 * @param max Maximum value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_stepper_set_range(
    struct cupertino_stepper *stepper, double min, double max);

/**
 * @brief Enables or disables auto-repeat on continuous press-and-hold.
 *
 * @param stepper The stepper.
 * @param auto_repeat 1 to enable auto-repeat, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_stepper_set_auto_repeat(
    struct cupertino_stepper *stepper, int auto_repeat);

/**
 * @brief Sets disabled state of the stepper.
 *
 * @param stepper The stepper.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_stepper_set_disabled(struct cupertino_stepper *stepper, int disabled);

/**
 * @brief Advances continuous hold timer by delta_ms.
 *
 * @param stepper The stepper.
 * @param delta_ms Elapsed time in milliseconds.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_stepper_on_tick(struct cupertino_stepper *stepper, double delta_ms);

/**
 * @brief Retrieves underlying ui_spin_button_base CDK handle.
 *
 * @param stepper The stepper.
 * @param out_base Pointer to receive ui_spin_button_base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_stepper_get_base(
    struct cupertino_stepper *stepper, struct ui_spin_button_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_STEPPER_H */
