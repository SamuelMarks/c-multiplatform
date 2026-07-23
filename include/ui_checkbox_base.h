#ifndef UI_CHECKBOX_BASE_H
#define UI_CHECKBOX_BASE_H

/* clang-format off */
#include "ui_error.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents the three possible states of a tri-state checkbox.
 */
enum ui_checkbox_state {
  UI_CHECKBOX_STATE_UNCHECKED = 0,
  UI_CHECKBOX_STATE_CHECKED = 1,
  UI_CHECKBOX_STATE_INDETERMINATE = 2
};

/**
 * @brief Opaque structure representing the base checkbox primitive.
 */
struct ui_checkbox_base;

/**
 * @brief Creates a new base checkbox instance.
 *
 * @param out_checkbox Pointer to receive the allocated checkbox instance.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY if allocation fails,
 * or UI_ERROR_INVALID_ARGUMENT if out_checkbox is null.
 */
enum ui_error ui_checkbox_base_create(struct ui_checkbox_base **out_checkbox);

/**
 * @brief Destroys a checkbox instance and frees its resources.
 *
 * @param checkbox The checkbox instance to destroy. If null, this function does
 * nothing.
 */
enum ui_error ui_checkbox_base_destroy(struct ui_checkbox_base *checkbox);

/**
 * @brief Retrieves the current state of the checkbox.
 *
 * @param checkbox The checkbox instance.
 * @param out_state Pointer to receive the current state.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if any
 * parameter is null.
 */
enum ui_error ui_checkbox_base_get_state(struct ui_checkbox_base *checkbox,
                                         enum ui_checkbox_state *out_state);

/**
 * @brief Sets the state of the checkbox.
 *
 * @param checkbox The checkbox instance.
 * @param state The new state to apply.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if checkbox is
 * null or state is invalid.
 */
enum ui_error ui_checkbox_base_set_state(struct ui_checkbox_base *checkbox,
                                         enum ui_checkbox_state state);

/**
 * @brief Toggles the checkbox state.
 * If unchecked or indeterminate, toggles to checked. If checked, toggles to
 * unchecked.
 *
 * @param checkbox The checkbox instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if checkbox is
 * null.
 */
/**
 * @brief Gets the CVA vtable for this component.
 * @param checkbox The checkbox.
 * @param out_cva Pointer to store the vtable.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_checkbox_base_get_cva(struct ui_checkbox_base *checkbox,
                         struct ui_control_value_accessor *out_cva);

enum ui_error ui_checkbox_base_toggle(struct ui_checkbox_base *checkbox);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CHECKBOX_BASE_H */
