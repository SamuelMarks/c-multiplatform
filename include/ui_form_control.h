/**
 * @file ui_form_control.h
 * @brief Core form control definitions for reactive forms.
 */

#ifndef UI_FORM_CONTROL_H
#define UI_FORM_CONTROL_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_arena.h"
#include "ui_signal.h"
#include "ui_form_validators.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents the validation status of a form control, group, or array.
 */
enum ui_form_status {
  UI_FORM_STATUS_VALID = 0,
  UI_FORM_STATUS_INVALID = 1,
  UI_FORM_STATUS_PENDING = 2,
  UI_FORM_STATUS_DISABLED = 3
};

/**
 * @brief Opaque handle to a form control.
 */
typedef struct ui_form_control ui_form_control_t;

/**
 * @brief Creates a new form control.
 *
 * @param arena The arena to allocate from.
 * @param initial_value The initial value of the control.
 * @param type The type of the signal payload.
 * @param equality_fn The equality function.
 * @param destructor_fn The destructor function.
 * @param mode The signal mode.
 * @param out_control The pointer to store the created form control.
 * @return ui_error_t
 */
ui_error_t ui_form_control_create(struct ui_arena *arena,
                                  union ui_signal_payload initial_value,
                                  enum ui_signal_type type,
                                  ui_equality_fn equality_fn,
                                  ui_destructor_fn destructor_fn,
                                  enum ui_signal_mode mode,
                                  ui_form_control_t **out_control);

/**
 * @brief Sets the value of the form control, updating its state.
 *
 * @param control The form control.
 * @param new_value The new value to set.
 * @return ui_error_t
 */
ui_error_t ui_form_control_set_value(ui_form_control_t *control,
                                     union ui_signal_payload new_value);

/**
 * @brief Patches the value of the form control.
 * For a leaf node (form control), this is typically equivalent to set_value.
 *
 * @param control The form control.
 * @param new_value The new value to set.
 * @return ui_error_t
 */
ui_error_t ui_form_control_patch_value(ui_form_control_t *control,
                                       union ui_signal_payload new_value);

/**
 * @brief Marks the form control as touched.
 *
 * @param control The form control.
 * @return ui_error_t
 */
ui_error_t ui_form_control_mark_as_touched(ui_form_control_t *control);

/**
 * @brief Disables the form control.
 *
 * @param control The form control.
 * @return ui_error_t
 */
ui_error_t ui_form_control_disable(ui_form_control_t *control);

/**
 * @brief Enables the form control.
 *
 * @param control The form control.
 * @return ui_error_t
 */
ui_error_t ui_form_control_enable(ui_form_control_t *control);

/**
 * @brief Gets the value signal of the form control.
 *
 * @param control The form control.
 * @param out_signal The pointer to store the signal.
 * @return ui_error_t
 */
ui_error_t ui_form_control_get_value_signal(ui_form_control_t *control,
                                            ui_signal_t **out_signal);

/**
 * @brief Gets the status signal of the form control.
 * Signal payload will be an int32 representing enum ui_form_status.
 *
 * @param control The form control.
 * @param out_signal The pointer to store the signal.
 * @return ui_error_t
 */
ui_error_t ui_form_control_get_status_signal(ui_form_control_t *control,
                                             ui_signal_t **out_signal);

/**
 * @brief Gets the touched signal of the form control.
 * Signal payload will be a bool.
 *
 * @param control The form control.
 * @param out_signal The pointer to store the signal.
 * @return ui_error_t
 */
ui_error_t ui_form_control_get_touched_signal(ui_form_control_t *control,
                                              ui_signal_t **out_signal);

/**
 * @brief Gets the dirty signal of the form control.
 * Signal payload will be a bool.
 *
 * @param control The form control.
 * @param out_signal The pointer to store the signal.
 * @return ui_error_t
 */
ui_error_t ui_form_control_get_dirty_signal(ui_form_control_t *control,
                                            ui_signal_t **out_signal);

/**
 * @brief Gets the errors signal of the form control.
 * Signal payload will be a ptr_val to an error string, or NULL if none.
 *
 * @param control The form control.
 * @param out_signal The pointer to store the signal.
 * @return ui_error_t
 */
ui_error_t ui_form_control_get_errors_signal(ui_form_control_t *control,
                                             ui_signal_t **out_signal);

/**
 * @brief Sets the error string on the form control.
 *
 * @param control The form control.
 * @param error_msg The error message (or NULL).
 * @return ui_error_t
 */
ui_error_t ui_form_control_set_error(ui_form_control_t *control,
                                     const char *error_msg);

/**
 * @brief Destroys the form control.
 *
 * @param control The form control.
 * @return ui_error_t
 */
ui_error_t ui_form_control_destroy(ui_form_control_t *control);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
 * @brief Adds a synchronous validator to the form control.
 *
 * @param control The form control.
 * @param validator The validator function.
 * @param user_data Opaque data for the validator.
 * @return ui_error_t
 */
ui_error_t ui_form_control_add_validator(ui_form_control_t *control,
                                         ui_validator_fn validator,
                                         void *user_data);

/**
 * @brief Adds an asynchronous validator to the form control.
 *
 * @param control The form control.
 * @param validator The asynchronous validator function.
 * @param user_data Opaque data for the validator.
 * @param thread_pool The background thread pool to dispatch work to.
 * @param reactor The reactor to receive completion events.
 * @return ui_error_t
 */
ui_error_t ui_form_control_add_async_validator(
    ui_form_control_t *control, ui_async_validator_fn validator,
    void *user_data, struct ui_thread_pool *thread_pool,
    struct ui_reactor *reactor);
#endif /* UI_FORM_CONTROL_H */
