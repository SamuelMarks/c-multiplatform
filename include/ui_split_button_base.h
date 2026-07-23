#ifndef UI_SPLIT_BUTTON_BASE_H
#define UI_SPLIT_BUTTON_BASE_H

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_button_base.h"
/* clang-format on */

struct ui_split_button_base;

/**
 * @brief Creates a new unstyled split button base component.
 *
 * A split button combines a primary action button and a distinct dropdown
 * trigger button.
 *
 * CSS Custom Properties (Variable Hooks):
 * - `--split-btn-gap`: Spacing between the main button and the trigger.
 * - `--split-btn-bg`: Shared background color.
 *
 * @param out_split_button Pointer to receive the allocated split button base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_split_button_base_create(struct ui_split_button_base **out_split_button);

/**
 * @brief Destroys a split button base component.
 *
 * @param split_button The split button to destroy.
 */
void ui_split_button_base_destroy(struct ui_split_button_base *split_button);

/**
 * @brief Sets the disabled state of both parts of the split button.
 *
 * @param split_button The split button.
 * @param disabled 1 to disable, 0 to enable.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_split_button_base_set_disabled(struct ui_split_button_base *split_button,
                                  int disabled);

/**
 * @brief Gets the underlying primary action button component.
 *
 * @param split_button The split button.
 * @param out_main_btn Pointer to receive the primary button.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_split_button_base_get_main_button(struct ui_split_button_base *split_button,
                                     struct ui_button_base **out_main_btn);

/**
 * @brief Gets the underlying dropdown trigger button component.
 *
 * @param split_button The split button.
 * @param out_trigger_btn Pointer to receive the trigger button.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_split_button_base_get_trigger_button(
    struct ui_split_button_base *split_button,
    struct ui_button_base **out_trigger_btn);

/**
 * @brief Gets the underlying container component instance for style injection
 * and DOM mounting.
 *
 * @param split_button The split button.
 * @return The underlying component.
 */
enum ui_error
ui_split_button_base_get_component(struct ui_split_button_base *split_button,
                                   struct ui_component **out_component);

/**
 * @brief Binds the disabled state to a boolean signal.
 *
 * @param widget The widget.
 * @param disabled_signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_split_button_base_bind_disabled(struct ui_split_button_base *widget,
                                   struct ui_signal *disabled_signal);

/**
 * @brief Binds the text content to a string signal for dynamic
 * internationalization.
 *
 * @param widget The widget.
 * @param text_signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_split_button_base_bind_text(struct ui_split_button_base *widget,
                               struct ui_signal *text_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SPLIT_BUTTON_BASE_H */
