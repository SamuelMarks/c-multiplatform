#ifndef UI_SELECT_BASE_H
#define UI_SELECT_BASE_H

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

struct ui_select_base;

/**
 * @brief Callback invoked when the select value/state changes.
 *
 * @param select The select component.
 * @param selected_index The currently selected item index (-1 if none).
 * @param user_data Opaque user data.
 */
typedef enum ui_error (*ui_select_on_change_t)(struct ui_select_base *select,
                                               int selected_index,
                                               void *user_data);

/**
 * @brief Callback invoked when the select dropdown is opened or closed.
 *
 * @param select The select component.
 * @param is_open 1 if opened, 0 if closed.
 * @param user_data Opaque user data.
 */
typedef enum ui_error (*ui_select_on_open_change_t)(
    struct ui_select_base *select, int is_open, void *user_data);

/**
 * @brief Creates a new unstyled select/dropdown base component.
 *
 * CSS Custom Properties (Variable Hooks):
 * - `--select-bg`: Background color of the select button.
 * - `--select-border`: Border of the select button.
 * - `--select-padding`: Padding inside the select button.
 * - `--select-color`: Text color.
 * - `--select-arrow-color`: Color of the dropdown indicator arrow.
 * - `--select-dropdown-bg`: Background color of the dropdown list.
 * - `--select-dropdown-border`: Border of the dropdown list.
 * - `--select-item-padding`: Padding for each list item.
 * - `--select-item-hover-bg`: Background color when hovering over an item.
 * - `--select-disabled-opacity`: Opacity when the select is disabled.
 *
 * @param out_select Pointer to receive the allocated select base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_select_base_create(struct ui_select_base **out_select);

/**
 * @brief Destroys a select base component.
 *
 * @param select The select component to destroy.
 */
void ui_select_base_destroy(struct ui_select_base *select);

/**
 * @brief Sets the disabled state of the select component.
 * Updates ARIA attributes and prevents state changes.
 *
 * @param select The select component.
 * @param disabled 1 to disable, 0 to enable.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_select_base_set_disabled(struct ui_select_base *select,
                                          int disabled);

/**
 * @brief Adds an option to the select dropdown.
 * This appends a child <option> node to the select's DOM representation.
 *
 * @param select The select component.
 * @param label The display text of the option.
 * @param value The underlying value of the option.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_select_base_add_option(struct ui_select_base *select,
                                        const char *label, const char *value);

/**
 * @brief Sets the number of items managed by the select base (for keyboard
 * navigation bounds).
 *
 * @param select The select component.
 * @param num_items The total number of selectable items.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_select_base_set_item_count(struct ui_select_base *select,
                                            int num_items);

/**
 * @brief Opens or closes the select dropdown state.
 *
 * @param select The select component.
 * @param is_open 1 to open, 0 to close.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_select_base_set_open(struct ui_select_base *select,
                                      int is_open);

/**
 * @brief Checks if the select dropdown is open.
 *
 * @param select The select component.
 * @return 1 if open, 0 if closed.
 */
enum ui_error ui_select_base_is_open(const struct ui_select_base *select,
                                     int *out_is_open);

/**
 * @brief Sets the currently highlighted index (used during keyboard
 * navigation).
 *
 * @param select The select component.
 * @param index The item index to highlight, or -1 for none.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_select_base_set_highlighted_index(struct ui_select_base *select, int index);

/**
 * @brief Gets the currently highlighted index.
 *
 * @param select The select component.
 * @return The highlighted index, or -1 if none.
 */
enum ui_error
ui_select_base_get_highlighted_index(const struct ui_select_base *select,
                                     int *out_index);

/**
 * @brief Sets the confirmed selected index.
 *
 * @param select The select component.
 * @param index The selected index, or -1 for none.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_select_base_set_selected_index(struct ui_select_base *select,
                                                int index);

/**
 * @brief Gets the currently selected index.
 *
 * @param select The select component.
 * @return The selected index, or -1 if none.
 */
enum ui_error
ui_select_base_get_selected_index(const struct ui_select_base *select,
                                  int *out_index);

/**
 * @brief Sets the change handler for the select component (selection change).
 *
 * @param select The select component.
 * @param on_change The callback to invoke when the selected value changes.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_select_base_set_on_change(struct ui_select_base *select,
                                           ui_select_on_change_t on_change,
                                           void *user_data);

/**
 * @brief Sets the handler for open state changes.
 *
 * @param select The select component.
 * @param on_open_change The callback to invoke when the open state changes.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_select_base_set_on_open_change(struct ui_select_base *select,
                                  ui_select_on_open_change_t on_open_change,
                                  void *user_data);

/**
 * @brief Processes an incoming input event to trigger dropdown interactions.
 *
 * @param select The select component.
 * @param event The input event.
 * @param timestamp_ms Current time in milliseconds.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_select_base_process_event(struct ui_select_base *select,
                                           const struct ui_event *event,
                                           double timestamp_ms);

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * @param select The select component.
 * @return The underlying component.
 */
/**
 * @brief Gets the CVA vtable for this component.
 * @param select The select.
 * @param out_cva Pointer to store the vtable.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_select_base_get_cva(struct ui_select_base *select,
                                     struct ui_control_value_accessor *out_cva);

enum ui_error ui_select_base_get_component(struct ui_select_base *select,
                                           struct ui_component **out_component);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SELECT_BASE_H */
