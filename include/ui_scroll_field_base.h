/**
 * @file ui_scroll_field_base.h
 * @brief Base unstyled scroll field / drum picker component for sequential
 * numeric selection.
 */

#ifndef UI_SCROLL_FIELD_BASE_H
#define UI_SCROLL_FIELD_BASE_H

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
#include "ui_types.h"
#include "ui_wheel_picker_base.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle representing the base scroll field component.
 */
struct ui_scroll_field_base;

/**
 * @brief Creates a new unstyled scroll field base component.
 *
 * @param out_field Pointer receiving newly allocated scroll field instance.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY if allocation fails,
 *         or UI_ERROR_INVALID_ARGUMENT if out_field is null.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_scroll_field_base_create(struct ui_scroll_field_base **out_field);

/**
 * @brief Destroys a scroll field base instance.
 *
 * @param field The scroll field instance to destroy. If null, does nothing.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_scroll_field_base_destroy(struct ui_scroll_field_base *field);

/**
 * @brief Retrieves underlying ui_component handle for mounting and styling.
 *
 * @param field The scroll field instance.
 * @param out_component Pointer receiving underlying component handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_scroll_field_base_get_component(
    struct ui_scroll_field_base *field, struct ui_component **out_component);

/**
 * @brief Configures the selectable numeric range and increment step.
 *
 * @param field The scroll field instance.
 * @param min_val Minimum allowed value.
 * @param max_val Maximum allowed value.
 * @param step Increment/decrement step delta.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_scroll_field_base_set_range(
    struct ui_scroll_field_base *field, int min_val, int max_val, int step);

/**
 * @brief Retrieves the current selectable numeric range.
 *
 * @param field The scroll field instance.
 * @param out_min_val Pointer receiving minimum value.
 * @param out_max_val Pointer receiving maximum value.
 * @param out_step Pointer receiving step delta.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_scroll_field_base_get_range(
    const struct ui_scroll_field_base *field, int *out_min_val,
    int *out_max_val, int *out_step);

/**
 * @brief Sets the currently selected numeric value (clamped to range).
 *
 * @param field The scroll field instance.
 * @param value The value to select.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_scroll_field_base_set_value(struct ui_scroll_field_base *field, int value);

/**
 * @brief Retrieves the currently selected numeric value.
 *
 * @param field The scroll field instance.
 * @param out_value Pointer receiving currently selected value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_scroll_field_base_get_value(
    const struct ui_scroll_field_base *field, int *out_value);

/**
 * @brief Increments the selected value by one step.
 *
 * @param field The scroll field instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_scroll_field_base_step_up(struct ui_scroll_field_base *field);

/**
 * @brief Decrements the selected value by one step.
 *
 * @param field The scroll field instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_scroll_field_base_step_down(struct ui_scroll_field_base *field);

/**
 * @brief Enables or disables continuous circular wrap-around looping.
 *
 * @param field The scroll field instance.
 * @param looping Nonzero to enable wrap-around looping, zero to clamp at
 * bounds.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_scroll_field_base_set_looping(
    struct ui_scroll_field_base *field, int looping);

/**
 * @brief Queries whether circular wrap-around looping is currently enabled.
 *
 * @param field The scroll field instance.
 * @param out_looping Pointer receiving 1 if looping, 0 if clamping.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_scroll_field_base_is_looping(
    const struct ui_scroll_field_base *field, int *out_looping);

#ifdef __cplusplus
}
#endif

#endif /* UI_SCROLL_FIELD_BASE_H */
