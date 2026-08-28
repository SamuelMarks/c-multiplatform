/**
 * @file ui_toggle_base.h
 * @brief Defines the base logic for toggle UI components (checkbox and radio).
 */
#ifndef UI_TOGGLE_BASE_H
#define UI_TOGGLE_BASE_H

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_gesture.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The type of the toggle component.
 */
enum ui_toggle_type {
  UI_TOGGLE_TYPE_CHECKBOX = 0, /**< Checkbox type */
  UI_TOGGLE_TYPE_RADIO         /**< Radio button type */
};

/**
 * @brief Opaque structure representing the toggle base component.
 */
struct ui_toggle_base;

/**
 * @brief Callback invoked when the toggle's checked state changes.
 *
 * @param toggle The toggle component that changed state.
 * @param checked The new checked state (1 for checked, 0 for unchecked).
 * @param user_data Opaque user data provided during registration.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_toggle_on_change_t)(struct ui_toggle_base *toggle,
                                            int checked, void *user_data);

/**
 * @brief Creates a new unstyled toggle base component (checkbox or radio).
 *
 * CSS Custom Properties (Variable Hooks):
 * - `--toggle-bg`: Background of the toggle container.
 * - `--toggle-border`: Border of the toggle.
 * - `--toggle-border-radius`: Border radius.
 * - `--toggle-checked-bg`: Background when checked.
 * - `--toggle-checked-color`: Color of the checkmark/indicator.
 * - `--toggle-disabled-bg`: Background when disabled.
 * - `--toggle-disabled-color`: Color of the indicator when disabled.
 * - `--toggle-cursor`: Cursor style on hover.
 *
 * @param type The type of toggle (checkbox or radio).
 * @param out_toggle Pointer to receive the allocated toggle base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_toggle_base_create(enum ui_toggle_type type,
                                 struct ui_toggle_base **out_toggle);

/**
 * @brief Destroys a toggle base component.
 *
 * @param toggle The toggle to destroy.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_toggle_base_destroy(struct ui_toggle_base *toggle);

/**
 * @brief Sets the disabled state of the toggle.
 * Updates ARIA attributes and prevents state changes.
 *
 * @param toggle The toggle.
 * @param disabled 1 to disable, 0 to enable.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_toggle_base_set_disabled(struct ui_toggle_base *toggle,
                                       int disabled);

/**
 * @brief Gets the current checked state of the toggle.
 *
 * @param toggle The toggle.
 * @param out_is_checked Pointer to receive 1 if checked, 0 if unchecked.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_toggle_base_is_checked(const struct ui_toggle_base *toggle,
                                     int *out_is_checked);

/**
 * @brief Programmatically sets the checked state of the toggle.
 * This does NOT trigger the on_change callback.
 *
 * @param toggle The toggle.
 * @param checked 1 to check, 0 to uncheck.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_toggle_base_set_checked(struct ui_toggle_base *toggle,
                                      int checked);

/**
 * @brief Sets the group name for radio buttons.
 * Used for grouped radio exclusion (only one radio per group can be checked).
 *
 * @param toggle The toggle.
 * @param group_name The group name string.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_toggle_base_set_group_name(struct ui_toggle_base *toggle,
                                         const char *group_name);

/**
 * @brief Sets the change handler for the toggle.
 *
 * @param toggle The toggle.
 * @param on_change The callback to invoke when the state changes due to user
 * interaction.
 * @param user_data Opaque user data passed to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_toggle_base_set_on_change(struct ui_toggle_base *toggle,
                                        ui_toggle_on_change_t on_change,
                                        void *user_data);

/**
 * @brief Processes an incoming input event to trigger toggling.
 *
 * @param toggle The toggle.
 * @param event The input event.
 * @param timestamp_ms Current time in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_toggle_base_process_event(struct ui_toggle_base *toggle,
                                        const struct ui_event *event,
                                        double timestamp_ms);

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * @param toggle The toggle.
 * @param out_component Pointer to receive the underlying component.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_toggle_base_get_component(struct ui_toggle_base *toggle,
                                        struct ui_component **out_component);

/**
 * @brief Retrieves the Control Value Accessor for this toggle.
 *
 * @param toggle The toggle.
 * @param out_cva Pointer to receive the populated CVA structure.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_toggle_base_get_cva(struct ui_toggle_base *toggle,
                                  struct ui_control_value_accessor *out_cva);

#ifdef __cplusplus
}
#endif

#endif /* UI_TOGGLE_BASE_H */
