/**
 * @file ui_button_base.h
 * @brief Button base component definitions.
 */
#ifndef UI_BUTTON_BASE_H
#define UI_BUTTON_BASE_H

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_gesture.h"
/* clang-format on */

struct ui_button_base;

/**
 * @brief Callback invoked when the button is clicked or tapped.
 */
typedef ui_error_t (*ui_button_on_click_t)(struct ui_button_base *button,
                                           void *user_data);

/**
 * @brief Creates a new unstyled button base component.
 *
 * CSS Custom Properties (Variable Hooks):
 * - `--btn-bg`: Background color of the button.
 * - `--btn-color`: Text color.
 * - `--btn-padding`: Padding around the text.
 * - `--btn-border`: Border definition.
 * - `--btn-border-radius`: Border radius.
 * - `--btn-cursor`: Cursor style on hover.
 * - `--btn-disabled-bg`: Background when disabled.
 * - `--btn-disabled-color`: Text color when disabled.
 *
 * @param out_button Pointer to receive the allocated button base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_button_base_create(struct ui_button_base **out_button);

/**
 * @brief Destroys a button base component.
 *
 * @param button The button to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_button_base_destroy(struct ui_button_base *button);

/**
 * @brief Sets the disabled state of the button.
 * Updates ARIA attributes and prevents click emission.
 *
 * @param button The button.
 * @param disabled 1 to disable, 0 to enable.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_button_base_set_disabled(struct ui_button_base *button,
                                       int disabled);

/**
 * @brief Sets the click handler for the button.
 *
 * @param button The button.
 * @param on_click The callback to invoke on tap/click.
 * @param user_data Opaque user data passed to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_button_base_set_on_click(struct ui_button_base *button,
                                       ui_button_on_click_t on_click,
                                       void *user_data);

/**
 * @brief Processes an incoming input event to trigger tap gestures.
 *
 * @param button The button.
 * @param event The input event.
 * @param timestamp_ms Current time in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_button_base_process_event(struct ui_button_base *button,
                                        const struct ui_event *event,
                                        double timestamp_ms);

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * @param button The button.
 * @param out_component Pointer to receive the underlying component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_button_base_get_component(struct ui_button_base *button,
                                        struct ui_component **out_component);

/**
 * @brief Binds the disabled state to a boolean signal.
 *
 * @param widget The widget.
 * @param disabled_signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_button_base_bind_disabled(struct ui_button_base *widget,
                                        struct ui_signal *disabled_signal);

/**
 * @brief Binds the text content to a string signal for dynamic
 * internationalization.
 *
 * @param widget The widget.
 * @param text_signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_button_base_bind_text(struct ui_button_base *widget,
                                    struct ui_signal *text_signal);

struct ui_ripple_state;
/**
 * @brief Gets the ripple state of the button.
 *
 * @param button The button.
 * @param out_state Pointer to receive the ripple state.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_button_base_get_ripple_state(struct ui_button_base *button,
                                           struct ui_ripple_state *out_state);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_BUTTON_BASE_H */
