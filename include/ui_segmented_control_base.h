#ifndef UI_SEGMENTED_CONTROL_BASE_H
#define UI_SEGMENTED_CONTROL_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

struct ui_segmented_control_base;
struct ui_segmented_button_base;

/**
 * @brief Selection mode for the segmented control.
 */
enum ui_segmented_control_mode {
  UI_SEGMENTED_CONTROL_MODE_SINGLE,
  UI_SEGMENTED_CONTROL_MODE_MULTI
};

/**
 * @brief Creates a new unstyled segmented control component.
 *
 * @param out_control Pointer to receive the allocated control base.
 * @param out_cva Optional pointer to receive the CVA interface.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_segmented_control_base_create(struct ui_segmented_control_base **out_control,
                                 struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a segmented control component.
 *
 * @param control The control to destroy.
 */
void ui_segmented_control_base_destroy(
    struct ui_segmented_control_base *control);

/**
 * @brief Gets the underlying component for the segmented control.
 *
 * @param control The control.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_segmented_control_base_get_component(
    struct ui_segmented_control_base *control,
    struct ui_component **out_component);

/**
 * @brief Sets the selection mode (single vs multi).
 *
 * @param control The control.
 * @param mode The selection mode.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_segmented_control_base_set_mode(struct ui_segmented_control_base *control,
                                   enum ui_segmented_control_mode mode);

/**
 * @brief Gets the current selection mode.
 *
 * @param control The control.
 * @param out_mode Pointer to receive the mode.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_segmented_control_base_get_mode(struct ui_segmented_control_base *control,
                                   enum ui_segmented_control_mode *out_mode);

/**
 * @brief Appends a segment button to the control.
 *
 * @param control The control.
 * @param button The button to append.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_segmented_control_base_append_segment(
    struct ui_segmented_control_base *control,
    struct ui_segmented_button_base *button);

/**
 * @brief Creates a new unstyled segmented button component.
 *
 * @param out_button Pointer to receive the allocated button base.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_segmented_button_base_create(struct ui_segmented_button_base **out_button);

/**
 * @brief Destroys a segmented button component.
 *
 * @param button The button to destroy.
 */
void ui_segmented_button_base_destroy(struct ui_segmented_button_base *button);

/**
 * @brief Gets the underlying component for the segmented button.
 *
 * @param button The button.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_segmented_button_base_get_component(struct ui_segmented_button_base *button,
                                       struct ui_component **out_component);

/**
 * @brief Sets the selected state of the button.
 *
 * @param button The button.
 * @param selected True if selected, false otherwise.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_segmented_button_base_set_selected(struct ui_segmented_button_base *button,
                                      int selected);

/**
 * @brief Gets the selected state of the button.
 *
 * @param button The button.
 * @param out_selected Pointer to receive the selected state.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_segmented_button_base_get_selected(struct ui_segmented_button_base *button,
                                      int *out_selected);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SEGMENTED_CONTROL_BASE_H */
