#ifndef UI_FORM_FIELD_BASE_H
#define UI_FORM_FIELD_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_form_control.h"
#include "ui_reactor.h"
/* clang-format on */

/**
 * @brief Opaque structure representing the base form field wrapper.
 */
struct ui_form_field_base;

/**
 * @brief Creates a new base form field instance.
 *
 * @param out_field Pointer to receive the allocated form field instance.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_form_field_base_create(struct ui_form_field_base **out_field);

/**
 * @brief Destroys a form field instance.
 *
 * @param field The form field.
 */
void ui_form_field_base_destroy(struct ui_form_field_base *field);

/**
 * @brief Sets the floating label text.
 *
 * @param field The form field.
 * @param label The label text.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_form_field_base_set_label(struct ui_form_field_base *field,
                                           const char *label);

/**
 * @brief Sets the hint text displayed below the field.
 *
 * @param field The form field.
 * @param hint The hint text.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_form_field_base_set_hint(struct ui_form_field_base *field,
                                          const char *hint);

/**
 * @brief Sets the error text and transitions the field to an error state.
 *
 * @param field The form field.
 * @param error_msg The error message (or NULL to clear the error state).
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_form_field_base_set_error(struct ui_form_field_base *field,
                                           const char *error_msg);

/**
 * @brief Sets a prefix component (e.g., an icon) to display before the control.
 *
 * @param field The form field.
 * @param prefix The prefix component.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_form_field_base_set_prefix(struct ui_form_field_base *field,
                                            struct ui_component *prefix);

/**
 * @brief Sets a suffix component (e.g., an icon) to display after the control.
 *
 * @param field The form field.
 * @param suffix The suffix component.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_form_field_base_set_suffix(struct ui_form_field_base *field,
                                            struct ui_component *suffix);

/**
 * @brief Sets the inner control component (e.g., a ui_input_base or
 * ui_select_base).
 *
 * @param field The form field.
 * @param control The underlying control component.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_form_field_base_set_control(struct ui_form_field_base *field,
                                             struct ui_component *control);

/**
 * @brief Coordinates the focused state (animates floating label).
 *
 * @param field The form field.
 * @param is_focused 1 if focused, 0 if blurred.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_form_field_base_set_focused(struct ui_form_field_base *field,
                                             int is_focused);

/**
 * @brief Coordinates the value state (keeps label floated if true).
 *
 * @param field The form field.
 * @param has_value 1 if the inner control has a value, 0 if empty.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_form_field_base_set_has_value(struct ui_form_field_base *field,
                                               int has_value);

/**
 * @brief Retrieves the underlying component instance for style injection and
 * DOM mounting.
 *
 * @param field The form field.
 * @return The underlying component.
 */

struct ui_form_control;
struct ui_reactor;

/**
 * @brief Binds a form control to the field base, wiring up validation errors.
 *
 * @param field The form field.
 * @param form_control The form control.
 * @param reactor The reactor for signal updates.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_form_field_base_bind_form_control(struct ui_form_field_base *field,
                                     struct ui_form_control *form_control,
                                     struct ui_reactor *reactor);

/** \brief ui_error */
enum ui_error
ui_form_field_base_get_component(struct ui_form_field_base *field,
                                 struct ui_component **out_component);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_form_field_base_bind_data(struct ui_form_field_base *widget,
                                           struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_FORM_FIELD_BASE_H */
