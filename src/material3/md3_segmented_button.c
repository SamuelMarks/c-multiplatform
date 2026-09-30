/**
 * @file md3_segmented_button.c
 * @brief Material 3 Segmented Button component implementation.
 */

/* clang-format off */
#include "material3/md3_segmented_button.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

static ui_error_t
md3_segmented_button_cva_write_value(void *component,
                                     union ui_signal_payload payload) {
  struct md3_segmented_button *btn;
  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  btn = (struct md3_segmented_button *)component;
  return md3_segmented_button_select_segment(btn, payload.int_val);
}

static ui_error_t md3_segmented_button_cva_set_disabled(void *component,
                                                        ui_bool_t is_disabled) {
  struct md3_segmented_button *btn;
  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  btn = (struct md3_segmented_button *)component;
  btn->disabled = is_disabled ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t
md3_segmented_button_create(struct ui_engine *engine,
                            enum ui_segmented_control_mode mode,
                            struct md3_segmented_button **out_button,
                            struct ui_control_value_accessor **out_cva) {
  struct md3_segmented_button *btn;
  ui_error_t rc;

  if (!engine || !out_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  btn = (struct md3_segmented_button *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_segmented_button));
  if (!btn) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(btn, 0, sizeof(struct md3_segmented_button));
  btn->mode = mode;
  btn->selected_segment_id = -1;

  rc = ui_segmented_control_base_create(&btn->base, NULL);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(btn);
    return rc;
  }

  rc = ui_segmented_control_base_set_mode(btn->base, mode);

  btn->cva.component = btn;
  btn->cva.write_value = md3_segmented_button_cva_write_value;
  btn->cva.set_disabled_state = md3_segmented_button_cva_set_disabled;
  btn->cva.register_on_change = NULL;
  btn->cva.register_on_touched = NULL;

  if (out_cva) {
    *out_cva = &btn->cva;
  }

  *out_button = btn;
  return rc;
}

ui_error_t md3_segmented_button_destroy(struct md3_segmented_button *button) {
  size_t i;
  ui_error_t rc = UI_ERROR_NONE;

  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < button->segment_count; i++) {
    if (button->segments[i].base) {
      rc = ui_segmented_button_base_destroy(button->segments[i].base);
      button->segments[i].base = NULL;
    }
  }

  rc = ui_segmented_control_base_destroy(button->base);
  C_MULTIPLATFORM_FREE(button);
  return rc;
}

ui_error_t md3_segmented_button_add_segment(
    struct md3_segmented_button *button, const char *label, const char *icon,
    int segment_id, struct ui_segmented_button_base **out_segment) {
  struct ui_segmented_button_base *seg;
  ui_error_t rc;
  size_t idx;

  if (!button || button->segment_count >= MD3_SEGMENTED_BUTTON_MAX_SEGMENTS) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_segmented_button_base_create(&seg);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_segmented_control_base_append_segment(button->base, seg);
  if (rc != UI_ERROR_NONE) {
    ui_segmented_button_base_destroy(seg);
    return rc;
  }

  idx = button->segment_count++;
  button->segments[idx].base = seg;
  button->segments[idx].id = segment_id;

  if (label) {
#if defined(_MSC_VER)
    strncpy_s(button->segments[idx].label, sizeof(button->segments[idx].label),
              label, sizeof(button->segments[idx].label) - 1);
#else
    strncpy(button->segments[idx].label, label,
            sizeof(button->segments[idx].label) - 1);
    button->segments[idx].label[sizeof(button->segments[idx].label) - 1] = '\0';
#endif
  }

  if (icon) {
#if defined(_MSC_VER)
    strncpy_s(button->segments[idx].icon, sizeof(button->segments[idx].icon),
              icon, sizeof(button->segments[idx].icon) - 1);
#else
    strncpy(button->segments[idx].icon, icon,
            sizeof(button->segments[idx].icon) - 1);
    button->segments[idx].icon[sizeof(button->segments[idx].icon) - 1] = '\0';
#endif
  }

  if (out_segment) {
    *out_segment = seg;
  }

  return UI_ERROR_NONE;
}

ui_error_t
md3_segmented_button_select_segment(struct md3_segmented_button *button,
                                    int segment_id) {
  size_t i;
  int found;
  ui_error_t rc = UI_ERROR_NONE;

  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  found = 0;
  for (i = 0; i < button->segment_count; i++) {
    if (button->segments[i].id == segment_id) {
      found = 1;
      if (button->mode == UI_SEGMENTED_CONTROL_MODE_SINGLE) {
        button->selected_segment_id = segment_id;
        rc = ui_segmented_button_base_set_selected(button->segments[i].base, 1);
      } else {
        int current_sel = 0;
        rc = ui_segmented_button_base_get_selected(button->segments[i].base,
                                                   &current_sel);
        rc = ui_segmented_button_base_set_selected(button->segments[i].base,
                                                   !current_sel);
      }
    } else if (button->mode == UI_SEGMENTED_CONTROL_MODE_SINGLE) {
      rc = ui_segmented_button_base_set_selected(button->segments[i].base, 0);
    }
  }

  if (!found) {
    return UI_ERROR_NOT_FOUND;
  }

  return rc;
}

ui_error_t
md3_segmented_button_is_selected(const struct md3_segmented_button *button,
                                 int segment_id, int *out_selected) {
  size_t i;

  if (!button || !out_selected) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < button->segment_count; i++) {
    if (button->segments[i].id == segment_id) {
      return ui_segmented_button_base_get_selected(button->segments[i].base,
                                                   out_selected);
    }
  }

  return UI_ERROR_NOT_FOUND;
}

ui_error_t
md3_segmented_button_get_base(struct md3_segmented_button *button,
                              struct ui_segmented_control_base **out_base) {
  if (!button || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = button->base;
  return UI_ERROR_NONE;
}
