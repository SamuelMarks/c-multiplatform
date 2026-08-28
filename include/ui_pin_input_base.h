/**
 * @file ui_pin_input_base.h
 * @brief Base component for one-time passcode (OTP) and PIN input fields.
 */

#ifndef UI_PIN_INPUT_BASE_H
#define UI_PIN_INPUT_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

/**
 * @brief Opaque structure representing a pin input base component.
 */
struct ui_pin_input_base;

/**
 * @brief Creates a new unstyled pin input base component.
 *
 * @param out_pin_input Pointer to receive the allocated component.
 * @param length The number of pin digit inputs.
 * @param out_cva Optional pointer to receive the CVA interface.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_pin_input_base_create(struct ui_pin_input_base **out_pin_input,
                                    int length,
                                    struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a pin input base component.
 *
 * @param pin_input The component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_pin_input_base_destroy(struct ui_pin_input_base *pin_input);

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * @param pin_input The pin input component.
 * @param out_component Output pointer for the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_pin_input_base_get_component(struct ui_pin_input_base *pin_input,
                                           struct ui_component **out_component);

/**
 * @brief Handles a character input event, auto-advancing focus.
 *
 * @param pin_input The component.
 * @param index The index of the input box receiving the character.
 * @param c The character string.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_pin_input_base_on_input(struct ui_pin_input_base *pin_input,
                                      int index, const char *c);

/**
 * @brief Handles a backspace event, cascading focus backwards.
 *
 * @param pin_input The component.
 * @param index The index of the input box where backspace was pressed.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_pin_input_base_on_backspace(struct ui_pin_input_base *pin_input,
                                          int index);

/**
 * @brief Handles a paste event, splitting string across inputs.
 *
 * @param pin_input The component.
 * @param pasted_text The pasted text string.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_pin_input_base_on_paste(struct ui_pin_input_base *pin_input,
                                      const char *pasted_text);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_PIN_INPUT_BASE_H */
