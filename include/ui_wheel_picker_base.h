/**
 * @file ui_wheel_picker_base.h
 * @brief Wheel picker component for selecting values from a spinning list.
 */
#ifndef UI_WHEEL_PICKER_BASE_H
#define UI_WHEEL_PICKER_BASE_H

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_event.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque structure representing a wheel picker base component.
 */
struct ui_wheel_picker_base;

/**
 * @brief Callback invoked when the wheel picker selection changes.
 *
 * @param picker The wheel picker component.
 * @param selected_index The new selected index.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_wheel_picker_on_change_t)(
    struct ui_wheel_picker_base *picker, int selected_index, void *user_data);

/**
 * @brief Creates a wheel picker base component.
 *
 * @param out_picker Pointer to receive the allocated component.
 * @param out_cva Optional pointer to receive the CVA interface.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_wheel_picker_base_create(struct ui_wheel_picker_base **out_picker,
                            struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a wheel picker base component.
 *
 * @param picker The wheel picker to destroy.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_wheel_picker_base_destroy(struct ui_wheel_picker_base *picker);

/**
 * @brief Sets the data for the picker.
 *
 * @param picker The wheel picker.
 * @param items Array of string items.
 * @param count Number of items.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_wheel_picker_base_set_items(struct ui_wheel_picker_base *picker,
                                          const char *const *items, int count);

/**
 * @brief Enables or disables infinite looping of the items.
 *
 * @param picker The wheel picker.
 * @param is_looping 1 to enable infinite loop, 0 to disable.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_wheel_picker_base_set_looping(struct ui_wheel_picker_base *picker,
                                            int is_looping);

/**
 * @brief Sets the currently selected index.
 *
 * @param picker The wheel picker.
 * @param index The index to select.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_wheel_picker_base_set_selected_index(struct ui_wheel_picker_base *picker,
                                        int index);

/**
 * @brief Gets the currently selected index.
 *
 * @param picker The wheel picker.
 * @param out_index Pointer to receive the index.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_wheel_picker_base_get_selected_index(
    const struct ui_wheel_picker_base *picker, int *out_index);

/**
 * @brief Sets the change callback.
 *
 * @param picker The wheel picker.
 * @param on_change The callback.
 * @param user_data User data.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_wheel_picker_base_set_on_change(struct ui_wheel_picker_base *picker,
                                   ui_wheel_picker_on_change_t on_change,
                                   void *user_data);

/**
 * @brief Processes input events for gestures and keyboard navigation.
 *
 * @param picker The wheel picker.
 * @param event The event to process.
 * @param timestamp_ms Current time in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_wheel_picker_base_process_event(struct ui_wheel_picker_base *picker,
                                   const struct ui_event *event,
                                   double timestamp_ms);

/**
 * @brief Updates the physics simulation (momentum scrolling, snapping).
 *
 * @param picker The wheel picker.
 * @param delta_ms Time delta since last tick in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_wheel_picker_base_on_tick(struct ui_wheel_picker_base *picker,
                                        double delta_ms);

/**
 * @brief Gets the underlying component.
 *
 * @param picker The wheel picker.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_wheel_picker_base_get_component(struct ui_wheel_picker_base *picker,
                                   struct ui_component **out_component);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_WHEEL_PICKER_BASE_H */
