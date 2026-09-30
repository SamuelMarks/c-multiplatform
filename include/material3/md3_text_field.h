/**
 * @file md3_text_field.h
 * @brief Material 3 Text Field component wrapping ui_input_base and
 * ui_form_field_base.
 */

#ifndef MATERIAL3_MD3_TEXT_FIELD_H
#define MATERIAL3_MD3_TEXT_FIELD_H

/* clang-format off */
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_form_field_base.h"
#include "ui_input_base.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum md3_text_field_variant
 * @brief Material 3 Text Field variants (Filled and Outlined).
 */
enum md3_text_field_variant {
  MD3_TEXT_FIELD_FILLED = 0,
  MD3_TEXT_FIELD_OUTLINED,
  MD3_TEXT_FIELD_VARIANT_COUNT
};

/**
 * @struct md3_text_field
 * @brief Material 3 Text Field composite combining input and form field
 * styling.
 */
struct md3_text_field {
  struct ui_input_base *input;
  struct ui_form_field_base *field;
  struct ui_control_value_accessor cva;
  enum md3_text_field_variant variant;
  char prefix_text[32];
  char suffix_text[32];
  size_t max_length;
};

/**
 * @brief Creates a Material 3 text field wrapping ui_input_base &
 * ui_form_field_base.
 *
 * @param engine Pointer to ui_engine instance.
 * @param variant Visual variant (Filled or Outlined).
 * @param out_field Pointer to receive newly created text field.
 * @param out_cva Optional pointer to receive CVA interface.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_text_field_create(
    struct ui_engine *engine, enum md3_text_field_variant variant,
    struct md3_text_field **out_field,
    struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Material 3 text field and underlying components.
 *
 * @param field Text field to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_text_field_destroy(struct md3_text_field *field);

/**
 * @brief Sets current text value of the text field.
 *
 * @param field The text field.
 * @param text The text value.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_text_field_set_text(struct md3_text_field *field, const char *text);

/**
 * @brief Gets current text value of the text field.
 *
 * @param field The text field.
 * @param out_text Pointer to receive current text string pointer.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_text_field_get_text(
    const struct md3_text_field *field, const char **out_text);

/**
 * @brief Sets floating label text.
 *
 * @param field The text field.
 * @param label The label string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_text_field_set_label(struct md3_text_field *field, const char *label);

/**
 * @brief Sets supporting hint text displayed below the field.
 *
 * @param field The text field.
 * @param hint The hint string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_text_field_set_supporting_text(
    struct md3_text_field *field, const char *hint);

/**
 * @brief Sets error text, transitioning the field to error state.
 *
 * @param field The text field.
 * @param error_text The error message (or NULL to clear).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_text_field_set_error_text(
    struct md3_text_field *field, const char *error_text);

/**
 * @brief Sets prefix text.
 *
 * @param field The text field.
 * @param prefix Prefix string (e.g. "$").
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_text_field_set_prefix(struct md3_text_field *field, const char *prefix);

/**
 * @brief Sets suffix text.
 *
 * @param field The text field.
 * @param suffix Suffix string (e.g. "kg").
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_text_field_set_suffix(struct md3_text_field *field, const char *suffix);

/**
 * @brief Sets disabled state of the text field.
 *
 * @param field The text field.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_text_field_set_disabled(struct md3_text_field *field, int disabled);

/**
 * @brief Retrieves underlying ui_input_base handle.
 *
 * @param field The text field.
 * @param out_input Pointer to receive ui_input_base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_text_field_get_input_base(
    struct md3_text_field *field, struct ui_input_base **out_input);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_TEXT_FIELD_H */
