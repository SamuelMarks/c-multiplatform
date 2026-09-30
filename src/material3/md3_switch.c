/**
 * @file md3_switch.c
 * @brief Material 3 Switch component implementation wrapping
 * ui_slide_toggle_base.
 */

/* clang-format off */
#include "material3/md3_switch.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

static ui_error_t md3_switch_cva_write_value(void *component,
                                             union ui_signal_payload value) {
  struct md3_switch *sw;
  int bool_val;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sw = (struct md3_switch *)component;
  bool_val = value.bool_val ? 1 : 0;
  return md3_switch_set_checked(sw, bool_val);
}

static ui_error_t md3_switch_cva_set_disabled_state(void *component,
                                                    ui_bool_t is_disabled) {
  struct md3_switch *sw;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sw = (struct md3_switch *)component;
  return md3_switch_set_disabled(sw, is_disabled ? 1 : 0);
}

/**
 * @brief Creates a Material 3 switch wrapping ui_slide_toggle_base.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_switch Pointer to receive newly created switch.
 * @param out_cva Optional pointer to receive CVA interface.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t md3_switch_create(struct ui_engine *engine,
                             struct md3_switch **out_switch,
                             struct ui_control_value_accessor **out_cva) {
  struct md3_switch *sw;
  ui_error_t rc;

  if (!engine || !out_switch) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sw = (struct md3_switch *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_switch));
  if (!sw) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(sw, 0, sizeof(struct md3_switch));

  rc = ui_slide_toggle_base_create(&sw->base, NULL);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(sw);
    return rc;
  }

  sw->cva.component = sw;
  sw->cva.write_value = md3_switch_cva_write_value;
  sw->cva.set_disabled_state = md3_switch_cva_set_disabled_state;
  sw->cva.register_on_change = NULL;
  sw->cva.register_on_touched = NULL;

  if (out_cva) {
    *out_cva = &sw->cva;
  }
  *out_switch = sw;

  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a Material 3 switch and its underlying base.
 *
 * @param sw Switch to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_switch_destroy(struct md3_switch *sw) {
  ui_error_t rc;

  if (!sw) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_slide_toggle_base_destroy(sw->base);
  C_MULTIPLATFORM_FREE(sw);
  return rc;
}

/**
 * @brief Sets checked state of the switch.
 *
 * @param sw The switch.
 * @param checked 1 if selected, 0 if unselected.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_switch_set_checked(struct md3_switch *sw, int checked) {
  if (!sw) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_slide_toggle_base_set_checked(sw->base, checked ? 1 : 0);
}

/**
 * @brief Gets checked state of the switch.
 *
 * @param sw The switch.
 * @param out_checked Pointer to receive checked state (1 or 0).
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_switch_get_checked(const struct md3_switch *sw,
                                  int *out_checked) {
  if (!sw || !out_checked) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_slide_toggle_base_get_checked(sw->base, out_checked);
}

/**
 * @brief Sets disabled state of the switch.
 *
 * @param sw The switch.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_switch_set_disabled(struct md3_switch *sw, int disabled) {
  if (!sw) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_slide_toggle_base_set_disabled(sw->base, disabled ? 1 : 0);
}

/**
 * @brief Enables or disables thumb icon display (checkmark / cross).
 *
 * @param sw The switch.
 * @param show_icon 1 to show thumb icon, 0 to hide.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_switch_set_show_icon(struct md3_switch *sw, int show_icon) {
  if (!sw) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sw->show_icon = show_icon ? 1 : 0;
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves underlying ui_slide_toggle_base handle.
 *
 * @param sw The switch.
 * @param out_base Pointer to receive ui_slide_toggle_base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_switch_get_base(struct md3_switch *sw,
                               struct ui_slide_toggle_base **out_base) {
  if (!sw || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_base = sw->base;
  return UI_ERROR_NONE;
}
