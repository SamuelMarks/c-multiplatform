#ifndef UI_CONTROL_VALUE_ACCESSOR_H
#define UI_CONTROL_VALUE_ACCESSOR_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_signal.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declaration of the component.
 */
struct ui_component;

/**
 * @brief Function pointer for writing a value to the view (component).
 * @param component The UI component.
 * @param value The value to write.
 * @return ui_error_t
 */
typedef ui_error_t (*ui_cva_write_value_fn)(void *component,
                                            union ui_signal_payload value);

/**
 * @brief Function pointer for registering a callback when the view value
 * changes.
 * @param component The UI component.
 * @param callback The callback to invoke on change.
 * @param user_data Opaque user data for the callback (typically the form
 * control).
 * @return ui_error_t
 */
typedef ui_error_t (*ui_cva_register_on_change_fn)(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data);

/**
 * @brief Function pointer for registering a callback when the view is touched.
 * @param component The UI component.
 * @param callback The callback to invoke on touch.
 * @param user_data Opaque user data for the callback.
 * @return ui_error_t
 */
typedef ui_error_t (*ui_cva_register_on_touched_fn)(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data);

/**
 * @brief Function pointer for setting the disabled state of the view.
 * @param component The UI component.
 * @param is_disabled Whether it is disabled.
 * @return ui_error_t
 */
typedef ui_error_t (*ui_cva_set_disabled_state_fn)(void *component,
                                                   ui_bool_t is_disabled);

/**
 * @brief Control Value Accessor vtable interface.
 * Bridges a UI component and a form control.
 */
struct ui_control_value_accessor {
  ui_cva_write_value_fn
      write_value; /**< Function to write a value to the component */
  ui_cva_register_on_change_fn
      register_on_change; /**< Function to register an on-change callback */
  ui_cva_register_on_touched_fn
      register_on_touched; /**< Function to register an on-touched callback */
  ui_cva_set_disabled_state_fn
      set_disabled_state; /**< Function to set the disabled state */
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CONTROL_VALUE_ACCESSOR_H */
