/**
 * @file cupertino_radio.c
 * @brief Cupertino Radio selection control implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_radio.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

static ui_error_t
cupertino_radio_cva_write_value(void *component,
                                union ui_signal_payload value) {
  struct cupertino_radio *radio;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  radio = (struct cupertino_radio *)component;
  return cupertino_radio_set_checked(radio, value.bool_val ? 1 : 0);
}

static ui_error_t
cupertino_radio_cva_set_disabled_state(void *component, ui_bool_t is_disabled) {
  struct cupertino_radio *radio;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  radio = (struct cupertino_radio *)component;
  return cupertino_radio_set_disabled(radio, is_disabled ? 1 : 0);
}

/**
 * @brief Creates a new Cupertino Radio component.
 */
ui_error_t cupertino_radio_create(struct ui_engine *engine,
                                  struct cupertino_radio **out_radio,
                                  struct ui_control_value_accessor **out_cva) {
  struct cupertino_radio *radio;
  ui_error_t rc;

  if (!engine || !out_radio) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  radio = (struct cupertino_radio *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_radio));
  if (!radio) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(radio, 0, sizeof(struct cupertino_radio));
  radio->scale = 1.0f;
  radio->outer_diameter = CUPERTINO_RADIO_OUTER_DIAMETER;
  radio->inner_dot_diameter = CUPERTINO_RADIO_INNER_DOT_DIAMETER;
  radio->active_color = UI_COLOR_ARGB(255, 0x00, 0x7A, 0xFF); /* SystemBlue */
  radio->border_color = UI_COLOR_ARGB(255, 0xD1, 0xD1, 0xD6); /* SystemGray4 */

  rc = ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &radio->toggle);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(radio);
    return rc;
  }

  radio->cva.component = radio;
  radio->cva.write_value = cupertino_radio_cva_write_value;
  radio->cva.set_disabled_state = cupertino_radio_cva_set_disabled_state;
  radio->cva.register_on_change = NULL;
  radio->cva.register_on_touched = NULL;

  if (out_cva) {
    *out_cva = &radio->cva;
  }
  *out_radio = radio;

  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a Cupertino radio and its underlying toggle primitive.
 */
ui_error_t cupertino_radio_destroy(struct cupertino_radio *radio) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!radio) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (radio->group && radio->toggle) {
    rc = ui_radio_group_base_remove_toggle(radio->group, radio->toggle);
    radio->group = NULL;
  }

  if (radio->toggle) {
    rc = ui_toggle_base_destroy(radio->toggle);
    radio->toggle = NULL;
  }

  C_MULTIPLATFORM_FREE(radio);
  return rc;
}

/**
 * @brief Sets selection state of the radio, triggering spring scale pop.
 */
ui_error_t cupertino_radio_set_checked(struct cupertino_radio *radio,
                                       int checked) {
  ui_error_t rc;

  if (!radio) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  radio->is_checked = checked ? 1 : 0;
  radio->scale = radio->is_checked ? 1.15f : 1.0f;

  rc = ui_toggle_base_set_checked(radio->toggle, radio->is_checked);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Gets current selection state.
 */
ui_error_t cupertino_radio_get_checked(const struct cupertino_radio *radio,
                                       int *out_checked) {
  if (!radio || !out_checked) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_toggle_base_is_checked(radio->toggle, out_checked);
}

/**
 * @brief Sets disabled state of the radio.
 */
ui_error_t cupertino_radio_set_disabled(struct cupertino_radio *radio,
                                        int disabled) {
  ui_error_t rc;

  if (!radio) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  radio->is_disabled = disabled ? 1 : 0;
  rc = ui_toggle_base_set_disabled(radio->toggle, radio->is_disabled);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Binds the radio button to a radio group for mutual exclusion.
 */
ui_error_t cupertino_radio_bind_group(struct cupertino_radio *radio,
                                      struct ui_radio_group_base *group) {
  ui_error_t rc;

  if (!radio || !group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_radio_group_base_add_toggle(group, radio->toggle);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  radio->group = group;
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves underlying ui_toggle_base handle.
 */
ui_error_t cupertino_radio_get_toggle(struct cupertino_radio *radio,
                                      struct ui_toggle_base **out_toggle) {
  if (!radio || !out_toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_toggle = radio->toggle;
  return UI_ERROR_NONE;
}
