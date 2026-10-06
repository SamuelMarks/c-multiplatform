/**
 * @file md2_text_field.c
 * @brief Implementation of Material Design 2 Text Field component.
 */

/* clang-format off */
#include "material2/md2_text_field.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

struct md2_text_field {
  struct ui_form_field_base *base;
  enum md2_text_field_type type;
};

ui_error_t md2_text_field_create(struct ui_engine *engine,
                                 enum md2_text_field_type type,
                                 struct md2_text_field **out_field) {
  struct md2_text_field *field;
  ui_error_t rc;

  if (engine == NULL || out_field == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  field = (struct md2_text_field *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md2_text_field));
  if (field == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_form_field_base_create(&field->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(field);
    return rc;
  }

  field->type = type;

  *out_field = field;
  return UI_ERROR_NONE;
}

ui_error_t md2_text_field_destroy(struct md2_text_field *field) {
  if (field == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_form_field_base_destroy(field->base);
  C_MULTIPLATFORM_FREE(field);
  return UI_ERROR_NONE;
}

ui_error_t md2_text_field_get_base(struct md2_text_field *field,
                                   struct ui_form_field_base **out_base) {
  if (field == NULL || out_base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = field->base;
  return UI_ERROR_NONE;
}

ui_error_t md2_text_field_set_label(struct md2_text_field *field,
                                    const char *label) {
  if (field == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_form_field_base_set_label(field->base, label);
}

ui_error_t md2_text_field_set_hint(struct md2_text_field *field,
                                   const char *hint) {
  if (field == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_form_field_base_set_hint(field->base, hint);
}

ui_error_t md2_text_field_set_error(struct md2_text_field *field,
                                    const char *error_msg) {
  if (field == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_form_field_base_set_error(field->base, error_msg);
}

ui_error_t md2_text_field_set_prefix(struct md2_text_field *field,
                                     struct ui_component *prefix) {
  if (field == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_form_field_base_set_prefix(field->base, prefix);
}

ui_error_t md2_text_field_set_suffix(struct md2_text_field *field,
                                     struct ui_component *suffix) {
  if (field == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_form_field_base_set_suffix(field->base, suffix);
}

ui_error_t md2_text_field_set_control(struct md2_text_field *field,
                                      struct ui_component *control) {
  if (field == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_form_field_base_set_control(field->base, control);
}
