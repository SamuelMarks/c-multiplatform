#ifndef UI_SLIDE_TOGGLE_BASE_H
#define UI_SLIDE_TOGGLE_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
#include "ui_gesture.h"
#include "ui_control_value_accessor.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief State structure for the slide toggle (switch).
 */
struct ui_slide_toggle_base;

/**
 * @brief Creates a new unstyled slide toggle base component.
 *
 * @param out_toggle Pointer to receive the allocated slide toggle base.
 * @param out_cva Optional pointer to receive the CVA interface.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY on allocation
 * failure.
 */
enum ui_error
ui_slide_toggle_base_create(struct ui_slide_toggle_base **out_toggle,
                            struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a slide toggle base component.
 *
 * @param toggle The slide toggle to destroy.
 */
void ui_slide_toggle_base_destroy(struct ui_slide_toggle_base *toggle);

/**
 * @brief Sets the boolean state of the toggle.
 *
 * @param toggle The slide toggle.
 * @param checked Non-zero for true/checked, zero for false/unchecked.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
enum ui_error
ui_slide_toggle_base_set_checked(struct ui_slide_toggle_base *toggle,
                                 int checked);

/**
 * @brief Gets the boolean state of the toggle.
 *
 * @param toggle The slide toggle.
 * @param out_checked Pointer to receive the checked state (1 or 0).
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
enum ui_error
ui_slide_toggle_base_get_checked(const struct ui_slide_toggle_base *toggle,
                                 int *out_checked);

/**
 * @brief Sets whether the toggle is disabled.
 *
 * @param toggle The slide toggle.
 * @param disabled Non-zero to disable interaction.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
enum ui_error
ui_slide_toggle_base_set_disabled(struct ui_slide_toggle_base *toggle,
                                  int disabled);

/**
 * @brief Gets whether the toggle is disabled.
 *
 * @param toggle The slide toggle.
 * @param out_disabled Pointer to receive the disabled state.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
enum ui_error
ui_slide_toggle_base_get_disabled(const struct ui_slide_toggle_base *toggle,
                                  int *out_disabled);

/**
 * @brief Toggles the current state if not disabled.
 *
 * @param toggle The slide toggle.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
enum ui_error ui_slide_toggle_base_toggle(struct ui_slide_toggle_base *toggle);

/**
 * @brief Processes an input event (click, touch, or drag) through the internal
 * gesture recognizer.
 *
 * Updates the state (checked or unchecked) if a tap or definitive pan gesture
 * is completed.
 *
 * @param toggle The slide toggle.
 * @param event The raw input event.
 * @param timestamp_ms Current time in milliseconds.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
enum ui_error
ui_slide_toggle_base_process_event(struct ui_slide_toggle_base *toggle,
                                   const struct ui_event *event,
                                   double timestamp_ms);

/**
 * @brief Gets the visual drag offset if the user is currently panning the
 * thumb.
 *
 * Used by the renderer to visually move the thumb before the state locks in.
 *
 * @param toggle The slide toggle.
 * @param out_offset_x Pointer to receive the pan offset in pixels.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
enum ui_error
ui_slide_toggle_base_get_drag_offset(const struct ui_slide_toggle_base *toggle,
                                     float *out_offset_x);

/**
 * @brief Indicates whether the user is actively dragging the thumb.
 *
 * @param toggle The slide toggle.
 * @param out_is_dragging Pointer to receive the status (1 for dragging, 0 for
 * idle).
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
enum ui_error
ui_slide_toggle_base_is_dragging(const struct ui_slide_toggle_base *toggle,
                                 int *out_is_dragging);

#ifdef __cplusplus
}
#endif

#endif /* UI_SLIDE_TOGGLE_BASE_H */
