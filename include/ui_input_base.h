#ifndef UI_INPUT_BASE_H
#define UI_INPUT_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_gesture.h"
#include "ui_event.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

struct ui_input_base;

/**
 * @brief Callback invoked when the input's text state changes.
 */
typedef ui_error_t (*ui_input_on_change_t)(struct ui_input_base *input,
                                           const char *text, void *user_data);

/**
 * @brief Creates a new unstyled input base component.
 *
 * CSS Custom Properties (Variable Hooks):
 * - `--input-bg`: Background color of the input field.
 * - `--input-color`: Text color.
 * - `--input-border`: Border of the input.
 * - `--input-border-radius`: Border radius.
 * - `--input-padding`: Padding inside the input.
 * - `--input-cursor-color`: Color of the text caret/cursor.
 * - `--input-placeholder-color`: Color of the placeholder text.
 * - `--input-disabled-bg`: Background when disabled.
 * - `--input-disabled-color`: Text color when disabled.
 *
 * @param out_input Pointer to receive the allocated input base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_input_base_create(struct ui_input_base **out_input);

/**
 * @brief Destroys an input base component.
 *
 * @param input The input to destroy.
 */
ui_error_t ui_input_base_destroy(struct ui_input_base *input);

/**
 * @brief Sets the text of the input.
 *
 * @param input The input.
 * @param text The text string.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_input_base_set_text(struct ui_input_base *input,
                                  const char *text);

/**
 * @brief Gets the current text of the input.
 *
 * @param input The input.
 * @return The current text.
 */
ui_error_t ui_input_base_get_text(const struct ui_input_base *input,
                                  const char **out_text);

/**
 * @brief Sets the input type (e.g., "password", "number", "email", "tel").
 * This updates the DOM attribute to trigger the correct native behavior and
 * keyboards on mobile.
 *
 * @param input The input.
 * @param type The input type string.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_input_base_set_type(struct ui_input_base *input,
                                  const char *type);

/**
 * @brief Sets the placeholder text.
 *
 * @param input The input.
 * @param placeholder The placeholder string.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_input_base_set_placeholder(struct ui_input_base *input,
                                         const char *placeholder);

/**
 * @brief Sets the disabled state of the input.
 * Updates ARIA attributes and prevents state changes.
 *
 * @param input The input.
 * @param disabled 1 to disable, 0 to enable.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_input_base_set_disabled(struct ui_input_base *input,
                                      int disabled);

/**
 * @brief Sets the change handler for the input.
 *
 * @param input The input.
 * @param on_change The callback to invoke when the text changes.
 * @param user_data Opaque user data passed to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_input_base_set_on_change(struct ui_input_base *input,
                                       ui_input_on_change_t on_change,
                                       void *user_data);

/**
 * @brief Processes an incoming input event to trigger typing.
 *
 * @param input The input.
 * @param event The input event.
 * @param timestamp_ms Current time in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_input_base_process_event(struct ui_input_base *input,
                                       const struct ui_event *event,
                                       double timestamp_ms);

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * @param input The input.
 * @return The underlying component.
 */
/**
 * @brief Gets the CVA vtable for this component.
 * @param input The input.
 * @param out_cva Pointer to store the vtable.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_input_base_get_cva(struct ui_input_base *input,
                                 struct ui_control_value_accessor *out_cva);

ui_error_t ui_input_base_get_component(struct ui_input_base *input,
                                       struct ui_component **out_component);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_INPUT_BASE_H */
