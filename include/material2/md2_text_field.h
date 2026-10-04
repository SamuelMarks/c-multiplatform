/**
 * @file md2_text_field.h
 * @brief Material Design 2 Text Field component.
 */

#ifndef MATERIAL2_MD2_TEXT_FIELD_H
#define MATERIAL2_MD2_TEXT_FIELD_H

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_form_field_base.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum md2_text_field_type
 * @brief Specifies the visual variant of the Material 2 text field.
 */
enum md2_text_field_type {
  MD2_TEXT_FIELD_FILLED = 0,  /**< Filled text field (default M2). */
  MD2_TEXT_FIELD_OUTLINED = 1 /**< Outlined text field. */
};

/**
 * @struct md2_text_field
 * @brief Opaque handle to a Material 2 text field instance.
 */
struct md2_text_field;

/**
 * @brief Creates a new Material Design 2 text field.
 *
 * @param engine Pointer to ui_engine instance.
 * @param type The type of text field (filled or outlined).
 * @param out_field Pointer to receive the allocated text field instance.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_text_field_create(struct ui_engine *engine, enum md2_text_field_type type,
                      struct md2_text_field **out_field);

/**
 * @brief Destroys a Material Design 2 text field instance.
 *
 * @param field The text field to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_text_field_destroy(struct md2_text_field *field);

/**
 * @brief Retrieves the underlying form field base for a text field.
 *
 * @param field The text field.
 * @param out_base Pointer to receive the form field base.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_text_field_get_base(
    struct md2_text_field *field, struct ui_form_field_base **out_base);

/**
 * @brief Sets the floating label text.
 *
 * @param field The text field.
 * @param label The label text.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_text_field_set_label(struct md2_text_field *field, const char *label);

/**
 * @brief Sets the hint text displayed below the field.
 *
 * @param field The text field.
 * @param hint The hint text.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_text_field_set_hint(struct md2_text_field *field, const char *hint);

/**
 * @brief Sets the error text and transitions the field to an error state.
 *
 * @param field The text field.
 * @param error_msg The error message (or NULL to clear).
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_text_field_set_error(struct md2_text_field *field, const char *error_msg);

/**
 * @brief Sets a prefix component (e.g., an icon) to display before the control.
 *
 * @param field The text field.
 * @param prefix The prefix component.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_text_field_set_prefix(
    struct md2_text_field *field, struct ui_component *prefix);

/**
 * @brief Sets a suffix component (e.g., an icon) to display after the control.
 *
 * @param field The text field.
 * @param suffix The suffix component.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_text_field_set_suffix(
    struct md2_text_field *field, struct ui_component *suffix);

/**
 * @brief Sets the inner control component (e.g., a ui_input_base).
 *
 * @param field The text field.
 * @param control The underlying control component.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_text_field_set_control(
    struct md2_text_field *field, struct ui_component *control);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL2_MD2_TEXT_FIELD_H */
