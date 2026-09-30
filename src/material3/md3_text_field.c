/**
 * @file md3_text_field.c
 * @brief Material 3 Text Field component implementation wrapping ui_input_base
 * and ui_form_field_base.
 */

/* clang-format off */
#include "material3/md3_text_field.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

static ui_error_t
md3_text_field_cva_write_value(void *component, union ui_signal_payload value) {
  struct md3_text_field *tf;
  const char *str;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tf = (struct md3_text_field *)component;
  str = (const char *)value.ptr_val;
  return md3_text_field_set_text(tf, str ? str : "");
}

static ui_error_t md3_text_field_cva_set_disabled_state(void *component,
                                                        ui_bool_t is_disabled) {
  struct md3_text_field *tf;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tf = (struct md3_text_field *)component;
  return md3_text_field_set_disabled(tf, is_disabled ? 1 : 0);
}

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
ui_error_t md3_text_field_create(struct ui_engine *engine,
                                 enum md3_text_field_variant variant,
                                 struct md3_text_field **out_field,
                                 struct ui_control_value_accessor **out_cva) {
  struct md3_text_field *tf;
  ui_error_t rc;

  if (!engine || !out_field ||
      (unsigned)variant >= (unsigned)MD3_TEXT_FIELD_VARIANT_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tf = (struct md3_text_field *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_text_field));
  if (!tf) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(tf, 0, sizeof(struct md3_text_field));
  tf->variant = variant;

  rc = ui_input_base_create(&tf->input);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tf);
    return rc;
  }

  rc = ui_form_field_base_create(&tf->field);
  if (rc != UI_ERROR_NONE) {
    ui_input_base_destroy(tf->input);
    C_MULTIPLATFORM_FREE(tf);
    return rc;
  }

  tf->cva.component = tf;
  tf->cva.write_value = md3_text_field_cva_write_value;
  tf->cva.set_disabled_state = md3_text_field_cva_set_disabled_state;
  tf->cva.register_on_change = NULL;
  tf->cva.register_on_touched = NULL;

  if (out_cva) {
    *out_cva = &tf->cva;
  }
  *out_field = tf;

  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a Material 3 text field and underlying components.
 *
 * @param field Text field to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_text_field_destroy(struct md3_text_field *field) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (field->field) {
    rc = ui_form_field_base_destroy(field->field);
  }
  if (field->input) {
    rc = ui_input_base_destroy(field->input);
  }

  C_MULTIPLATFORM_FREE(field);
  return rc;
}

/**
 * @brief Sets current text value of the text field.
 *
 * @param field The text field.
 * @param text The text value.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_text_field_set_text(struct md3_text_field *field,
                                   const char *text) {
  ui_error_t rc;
  int has_val;

  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_input_base_set_text(field->input, text);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  has_val = (text && text[0] != '\0') ? 1 : 0;
  return ui_form_field_base_set_has_value(field->field, has_val);
}

/**
 * @brief Gets current text value of the text field.
 *
 * @param field The text field.
 * @param out_text Pointer to receive current text string pointer.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_text_field_get_text(const struct md3_text_field *field,
                                   const char **out_text) {
  if (!field || !out_text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_input_base_get_text(field->input, out_text);
}

/**
 * @brief Sets floating label text.
 *
 * @param field The text field.
 * @param label The label string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_text_field_set_label(struct md3_text_field *field,
                                    const char *label) {
  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_form_field_base_set_label(field->field, label);
}

/**
 * @brief Sets supporting hint text displayed below the field.
 *
 * @param field The text field.
 * @param hint The hint string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_text_field_set_supporting_text(struct md3_text_field *field,
                                              const char *hint) {
  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_form_field_base_set_hint(field->field, hint);
}

/**
 * @brief Sets error text, transitioning the field to error state.
 *
 * @param field The text field.
 * @param error_text The error message (or NULL to clear).
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_text_field_set_error_text(struct md3_text_field *field,
                                         const char *error_text) {
  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_form_field_base_set_error(field->field, error_text);
}

/**
 * @brief Sets prefix text.
 *
 * @param field The text field.
 * @param prefix Prefix string (e.g. "$").
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_text_field_set_prefix(struct md3_text_field *field,
                                     const char *prefix) {
  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!prefix) {
    field->prefix_text[0] = '\0';
  } else {
#if defined(_MSC_VER)
    strncpy_s(field->prefix_text, sizeof(field->prefix_text), prefix,
              sizeof(field->prefix_text) - 1);
#else
    strncpy(field->prefix_text, prefix, sizeof(field->prefix_text) - 1);
    field->prefix_text[sizeof(field->prefix_text) - 1] = '\0';
#endif
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Sets suffix text.
 *
 * @param field The text field.
 * @param suffix Suffix string (e.g. "kg").
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_text_field_set_suffix(struct md3_text_field *field,
                                     const char *suffix) {
  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!suffix) {
    field->suffix_text[0] = '\0';
  } else {
#if defined(_MSC_VER)
    strncpy_s(field->suffix_text, sizeof(field->suffix_text), suffix,
              sizeof(field->suffix_text) - 1);
#else
    strncpy(field->suffix_text, suffix, sizeof(field->suffix_text) - 1);
    field->suffix_text[sizeof(field->suffix_text) - 1] = '\0';
#endif
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Sets disabled state of the text field.
 *
 * @param field The text field.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_text_field_set_disabled(struct md3_text_field *field,
                                       int disabled) {
  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_input_base_set_disabled(field->input, disabled);
}

/**
 * @brief Retrieves underlying ui_input_base handle.
 *
 * @param field The text field.
 * @param out_input Pointer to receive ui_input_base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_text_field_get_input_base(struct md3_text_field *field,
                                         struct ui_input_base **out_input) {
  if (!field || !out_input) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_input = field->input;
  return UI_ERROR_NONE;
}
