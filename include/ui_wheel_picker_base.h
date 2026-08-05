#ifndef UI_WHEEL_PICKER_BASE_H
#define UI_WHEEL_PICKER_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_event.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

struct ui_wheel_picker_base;

/**
 * @brief Callback invoked when the wheel picker selection changes.
 */
typedef ui_error_t (*ui_wheel_picker_on_change_t)(
    struct ui_wheel_picker_base *picker, int selected_index, void *user_data);

/**
 * @brief Creates a wheel picker base component.
 *
 * @param out_picker Pointer to receive the allocated component.
 * @param out_cva Optional pointer to receive the CVA interface.
 */
ui_error_t
ui_wheel_picker_base_create(struct ui_wheel_picker_base **out_picker,
                            struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a wheel picker base component.
 */
ui_error_t ui_wheel_picker_base_destroy(struct ui_wheel_picker_base *picker);

/**
 * @brief Sets the data for the picker.
 *
 * @param picker The wheel picker.
 * @param items Array of string items.
 * @param count Number of items.
 */
ui_error_t ui_wheel_picker_base_set_items(struct ui_wheel_picker_base *picker,
                                          const char *const *items, int count);

/**
 * @brief Enables or disables infinite looping of the items.
 */
ui_error_t ui_wheel_picker_base_set_looping(struct ui_wheel_picker_base *picker,
                                            int is_looping);

/**
 * @brief Sets the currently selected index.
 */
ui_error_t
ui_wheel_picker_base_set_selected_index(struct ui_wheel_picker_base *picker,
                                        int index);

/**
 * @brief Gets the currently selected index.
 */
ui_error_t ui_wheel_picker_base_get_selected_index(
    const struct ui_wheel_picker_base *picker, int *out_index);

/**
 * @brief Sets the change callback.
 */
ui_error_t
ui_wheel_picker_base_set_on_change(struct ui_wheel_picker_base *picker,
                                   ui_wheel_picker_on_change_t on_change,
                                   void *user_data);

/**
 * @brief Processes input events for gestures and keyboard navigation.
 */
ui_error_t
ui_wheel_picker_base_process_event(struct ui_wheel_picker_base *picker,
                                   const struct ui_event *event,
                                   double timestamp_ms);

/**
 * @brief Updates the physics simulation (momentum scrolling, snapping).
 */
ui_error_t ui_wheel_picker_base_on_tick(struct ui_wheel_picker_base *picker,
                                        double delta_ms);

/**
 * @brief Gets the underlying component.
 */
ui_error_t
ui_wheel_picker_base_get_component(struct ui_wheel_picker_base *picker,
                                   struct ui_component **out_component);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_WHEEL_PICKER_BASE_H */
