/**
 * @file cupertino_text_field.h
 * @brief Cupertino Text Field & Text View components adhering to Apple
 * Human Interface Guidelines (HIG).
 */

#ifndef CUPERTINO_CUPERTINO_TEXT_FIELD_H
#define CUPERTINO_CUPERTINO_TEXT_FIELD_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_component.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_form_control.h"
#include "ui_input_base.h"
#include "ui_reactor.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum cupertino_overlay_visibility_mode
 * @brief Visibility mode for clear buttons, prefix, and suffix views.
 */
enum cupertino_overlay_visibility_mode {
  CUPERTINO_OVERLAY_NEVER = 0,      /**< Never visible. */
  CUPERTINO_OVERLAY_WHILE_EDITING,  /**< Visible only while focused/editing. */
  CUPERTINO_OVERLAY_UNLESS_EDITING, /**< Visible when not editing. */
  CUPERTINO_OVERLAY_ALWAYS,         /**< Always visible. */
  CUPERTINO_OVERLAY_VISIBILITY_COUNT
};

/**
 * @struct cupertino_text_field
 * @brief Cupertino Text Field wrapping ui_input_base.
 */
struct cupertino_text_field {
  struct ui_input_base *input; /**< Wrapped CDK input component. */
  enum cupertino_overlay_visibility_mode
      clear_button_mode;        /**< Clear button display mode. */
  int is_secure_text_entry;     /**< Obscure text (1 = password, 0 = plain). */
  int is_editing;               /**< Focus/editing state flag. */
  int is_multiline;             /**< Multi-line text view mode flag. */
  char placeholder[128];        /**< Placeholder hint label. */
  char prefix_text[64];         /**< Leading prefix text. */
  char suffix_text[64];         /**< Trailing suffix text. */
  ui_color_t background_color;  /**< Fill color (SystemGray6). */
  ui_color_t text_color;        /**< Foreground color (Label). */
  ui_color_t placeholder_color; /**< Placeholder color (TertiaryLabel). */
  float corner_radius; /**< Continuous squircle radius (8pt standard). */
  size_t max_lines;    /**< Maximum lines for auto-resizing text view. */
  size_t min_lines;    /**< Minimum lines for text view. */
  struct ui_control_value_accessor
      cva; /**< CVA interface for reactive forms. */
};

/* --- Cupertino Text Field Functions --- */

/**
 * @brief Creates a new Cupertino Text Field component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_field Pointer to receive newly created text field.
 * @param out_cva Optional pointer to receive Control Value Accessor.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_field_create(
    struct ui_engine *engine, struct cupertino_text_field **out_field,
    struct ui_control_value_accessor **out_cva);

/**
 * @brief Creates a multi-line Cupertino Text View component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param min_lines Minimum line height.
 * @param max_lines Maximum line height for auto-sizing.
 * @param out_view Pointer to receive newly created text view.
 * @param out_cva Optional pointer to receive Control Value Accessor.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_view_create(
    struct ui_engine *engine, size_t min_lines, size_t max_lines,
    struct cupertino_text_field **out_view,
    struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Cupertino Text Field/View and releases allocated resources.
 *
 * @param field Pointer to text field/view to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_text_field_destroy(struct cupertino_text_field *field);

/**
 * @brief Sets current text value of text field.
 *
 * @param field Pointer to text field.
 * @param text New text content.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_field_set_text(
    struct cupertino_text_field *field, const char *text);

/**
 * @brief Gets current text value of text field.
 *
 * @param field Pointer to text field.
 * @param out_text Pointer to receive current text string pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_field_get_text(
    const struct cupertino_text_field *field, const char **out_text);

/**
 * @brief Sets placeholder text displayed when field is empty.
 *
 * @param field Pointer to text field.
 * @param placeholder Placeholder string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_field_set_placeholder(
    struct cupertino_text_field *field, const char *placeholder);

/**
 * @brief Sets clear button display mode (Never, WhileEditing, UnlessEditing,
 * Always).
 *
 * @param field Pointer to text field.
 * @param mode Clear button visibility mode.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_text_field_set_clear_button_mode(
    struct cupertino_text_field *field,
    enum cupertino_overlay_visibility_mode mode);

/**
 * @brief Queries whether the clear button is currently visible.
 *
 * @param field Pointer to text field.
 * @param out_visible Pointer to receive visibility (1 = visible, 0 = hidden).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_text_field_is_clear_button_visible(
    const struct cupertino_text_field *field, int *out_visible);

/**
 * @brief Clears current text content if clear button is triggered.
 *
 * @param field Pointer to text field.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_text_field_trigger_clear(struct cupertino_text_field *field);

/**
 * @brief Sets secure text entry mode (password masking).
 *
 * @param field Pointer to text field.
 * @param is_secure 1 to mask characters, 0 for normal text.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_text_field_set_secure_text_entry(struct cupertino_text_field *field,
                                           int is_secure);

/**
 * @brief Sets editing (focus) state of text field.
 *
 * @param field Pointer to text field.
 * @param is_editing 1 if editing/focused, 0 if blurred.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_field_set_editing(
    struct cupertino_text_field *field, int is_editing);

/**
 * @brief Sets leading prefix label text.
 *
 * @param field Pointer to text field.
 * @param prefix Prefix string, or NULL to remove prefix.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_field_set_prefix(
    struct cupertino_text_field *field, const char *prefix);

/**
 * @brief Sets trailing suffix label text.
 *
 * @param field Pointer to text field.
 * @param suffix Suffix string, or NULL to remove suffix.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_field_set_suffix(
    struct cupertino_text_field *field, const char *suffix);

/**
 * @brief Sets disabled state of text field.
 *
 * @param field Pointer to text field.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_field_set_disabled(
    struct cupertino_text_field *field, int disabled);

/**
 * @brief Retrieves underlying ui_input_base component handle.
 *
 * @param field Pointer to text field.
 * @param out_base Pointer to receive ui_input_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_field_get_base(
    struct cupertino_text_field *field, struct ui_input_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_TEXT_FIELD_H */
