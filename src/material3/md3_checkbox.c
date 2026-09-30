/**
 * @file md3_checkbox.c
 * @brief Material 3 Checkbox component implementation wrapping
 * ui_checkbox_base.
 */

/* clang-format off */
#include "material3/md3_checkbox.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

static ui_error_t md3_checkbox_cva_write_value(void *component,
                                               union ui_signal_payload value) {
  struct md3_checkbox *cb;
  int bool_val;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cb = (struct md3_checkbox *)component;
  bool_val = value.bool_val ? 1 : 0;
  return md3_checkbox_set_checked(cb, bool_val);
}

static ui_error_t md3_checkbox_cva_set_disabled_state(void *component,
                                                      ui_bool_t is_disabled) {
  struct md3_checkbox *cb;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cb = (struct md3_checkbox *)component;
  return md3_checkbox_set_disabled(cb, is_disabled ? 1 : 0);
}

/**
 * @brief Creates a Material 3 checkbox wrapping ui_checkbox_base and exports
 * CVA.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_checkbox Pointer to receive newly created checkbox.
 * @param out_cva Pointer to receive control value accessor interface (optional,
 * can be NULL).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t md3_checkbox_create(struct ui_engine *engine,
                               struct md3_checkbox **out_checkbox,
                               struct ui_control_value_accessor **out_cva) {
  struct md3_checkbox *cb;
  ui_error_t rc;

  if (!engine || !out_checkbox) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cb = (struct md3_checkbox *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_checkbox));
  if (!cb) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(cb, 0, sizeof(struct md3_checkbox));

  rc = ui_checkbox_base_create(&cb->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(cb);
    return rc;
  }

  cb->cva.component = cb;
  cb->cva.write_value = md3_checkbox_cva_write_value;
  cb->cva.set_disabled_state = md3_checkbox_cva_set_disabled_state;
  cb->cva.register_on_change = NULL;
  cb->cva.register_on_touched = NULL;

  if (out_cva) {
    *out_cva = &cb->cva;
  }
  *out_checkbox = cb;

  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a Material 3 checkbox and frees its underlying base.
 *
 * @param checkbox Checkbox to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_checkbox_destroy(struct md3_checkbox *checkbox) {
  ui_error_t rc;

  if (!checkbox) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_checkbox_base_destroy(checkbox->base);
  C_MULTIPLATFORM_FREE(checkbox);
  return rc;
}

/**
 * @brief Sets checked state of the checkbox.
 *
 * @param checkbox The checkbox.
 * @param checked 1 if checked, 0 if unchecked.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_checkbox_set_checked(struct md3_checkbox *checkbox,
                                    int checked) {
  if (!checkbox) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_checkbox_base_set_checked(checkbox->base, checked ? 1 : 0);
}

/**
 * @brief Gets checked state of the checkbox.
 *
 * @param checkbox The checkbox.
 * @param out_checked Pointer to receive checked state (1 or 0).
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_checkbox_get_checked(const struct md3_checkbox *checkbox,
                                    int *out_checked) {
  enum ui_checkbox_state state;
  ui_error_t rc;

  if (!checkbox || !out_checked) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  rc = ui_checkbox_base_get_state(checkbox->base, &state);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  *out_checked = (state == UI_CHECKBOX_STATE_CHECKED) ? 1 : 0;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets indeterminate tri-state of the checkbox.
 *
 * @param checkbox The checkbox.
 * @param is_indeterminate 1 if indeterminate, 0 if determinate.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_checkbox_set_indeterminate(struct md3_checkbox *checkbox,
                                          int is_indeterminate) {
  if (!checkbox) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  checkbox->is_indeterminate = is_indeterminate ? 1 : 0;
  if (checkbox->is_indeterminate) {
    return ui_checkbox_base_set_state(checkbox->base,
                                      UI_CHECKBOX_STATE_INDETERMINATE);
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Sets disabled state of the checkbox.
 *
 * @param checkbox The checkbox.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_checkbox_set_disabled(struct md3_checkbox *checkbox,
                                     int disabled) {
  if (!checkbox) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  checkbox->is_disabled = disabled ? 1 : 0;
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves underlying ui_checkbox_base.
 *
 * @param checkbox The checkbox.
 * @param out_base Pointer to receive ui_checkbox_base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_checkbox_get_base(struct md3_checkbox *checkbox,
                                 struct ui_checkbox_base **out_base) {
  if (!checkbox || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_base = checkbox->base;
  return UI_ERROR_NONE;
}
