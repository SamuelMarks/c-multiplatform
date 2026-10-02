/**
 * @file cupertino_switch.c
 * @brief Cupertino Switch component wrapping ui_slide_toggle_base
 * implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_switch.h"
#include "cupertino/cupertino_haptics.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

static ui_error_t
cupertino_switch_cva_write_value(void *component,
                                 union ui_signal_payload value) {
  struct cupertino_switch *sw;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sw = (struct cupertino_switch *)component;
  return cupertino_switch_set_checked(sw, value.bool_val ? 1 : 0);
}

static ui_error_t
cupertino_switch_cva_set_disabled_state(void *component,
                                        ui_bool_t is_disabled) {
  struct cupertino_switch *sw;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sw = (struct cupertino_switch *)component;
  return cupertino_switch_set_disabled(sw, is_disabled ? 1 : 0);
}

/**
 * @brief Creates a new Cupertino Switch wrapping ui_slide_toggle_base.
 */
ui_error_t cupertino_switch_create(struct ui_engine *engine,
                                   struct cupertino_switch **out_switch,
                                   struct ui_control_value_accessor **out_cva) {
  struct cupertino_switch *sw;
  ui_error_t rc;

  if (!engine || !out_switch) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sw = (struct cupertino_switch *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_switch));
  if (!sw) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(sw, 0, sizeof(struct cupertino_switch));
  sw->thumb_width = CUPERTINO_SWITCH_THUMB_DIAMETER;
  sw->active_color = UI_COLOR_ARGB(255, 0x34, 0xC7, 0x59); /* SystemGreen */
  sw->track_color = UI_COLOR_ARGB(255, 0xE5, 0xE5, 0xEA);  /* SystemGray5 */

  rc = ui_slide_toggle_base_create(&sw->base, NULL);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(sw);
    return rc;
  }

  sw->cva.component = sw;
  sw->cva.write_value = cupertino_switch_cva_write_value;
  sw->cva.set_disabled_state = cupertino_switch_cva_set_disabled_state;
  sw->cva.register_on_change = NULL;
  sw->cva.register_on_touched = NULL;

  if (out_cva) {
    *out_cva = &sw->cva;
  }
  *out_switch = sw;

  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a Cupertino switch and its underlying base.
 */
ui_error_t cupertino_switch_destroy(struct cupertino_switch *sw) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!sw) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (sw->base) {
    rc = ui_slide_toggle_base_destroy(sw->base);
    sw->base = NULL;
  }

  C_MULTIPLATFORM_FREE(sw);
  return rc;
}

/**
 * @brief Sets checked state of the switch.
 */
ui_error_t cupertino_switch_set_checked(struct cupertino_switch *sw,
                                        int checked) {
  int cur_checked = 0;
  int new_checked;
  ui_error_t rc;

  if (!sw) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  new_checked = checked ? 1 : 0;
  rc = ui_slide_toggle_base_get_checked(sw->base, &cur_checked);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (cur_checked != new_checked) {
    rc = cupertino_haptic_selection();
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return ui_slide_toggle_base_set_checked(sw->base, new_checked);
}

/**
 * @brief Gets checked state of the switch.
 */
ui_error_t cupertino_switch_get_checked(const struct cupertino_switch *sw,
                                        int *out_checked) {
  if (!sw || !out_checked) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_slide_toggle_base_get_checked(sw->base, out_checked);
}

/**
 * @brief Sets disabled state of the switch.
 */
ui_error_t cupertino_switch_set_disabled(struct cupertino_switch *sw,
                                         int disabled) {
  if (!sw) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_slide_toggle_base_set_disabled(sw->base, disabled ? 1 : 0);
}

/**
 * @brief Enables or disables VoiceOver on/off glyph labels ('I' and 'O').
 */
ui_error_t
cupertino_switch_set_show_accessibility_labels(struct cupertino_switch *sw,
                                               int show_labels) {
  if (!sw) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sw->show_accessibility_labels = show_labels ? 1 : 0;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets interactive dragging state, stretching the thumb horizontally.
 */
ui_error_t cupertino_switch_set_dragging(struct cupertino_switch *sw,
                                         int is_dragging) {
  if (!sw) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sw->is_dragging = is_dragging ? 1 : 0;
  if (sw->is_dragging) {
    sw->thumb_width = CUPERTINO_SWITCH_THUMB_DRAG_WIDTH;
  } else {
    sw->thumb_width = CUPERTINO_SWITCH_THUMB_DIAMETER;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves underlying ui_slide_toggle_base handle.
 */
ui_error_t cupertino_switch_get_base(struct cupertino_switch *sw,
                                     struct ui_slide_toggle_base **out_base) {
  if (!sw || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_base = sw->base;
  return UI_ERROR_NONE;
}
