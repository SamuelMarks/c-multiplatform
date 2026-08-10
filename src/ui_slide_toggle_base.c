/* clang-format off */
#include "ui_slide_toggle_base.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
/* clang-format on */

struct ui_slide_toggle_base {
  int checked;
  int disabled;

  struct ui_gesture_recognizer *recognizer;
  int is_dragging;
  float drag_offset_x;
  float drag_start_offset_x;

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data);
  void *cva_on_change_user_data;

  ui_error_t (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data;
};

static ui_error_t trigger_cva_change(struct ui_slide_toggle_base *toggle) {
  if (toggle->cva_on_change) {
    union ui_signal_payload payload;
    payload.bool_val = toggle->checked;
    return toggle->cva_on_change(payload, toggle->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

static ui_error_t trigger_cva_touched(struct ui_slide_toggle_base *toggle) {
  if (toggle->cva_on_touched) {
    return toggle->cva_on_touched(toggle->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

static ui_error_t slide_toggle_cva_write_value(void *component,
                                               union ui_signal_payload value) {
  struct ui_slide_toggle_base *toggle =
      (struct ui_slide_toggle_base *)component;

  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toggle->checked = value.bool_val;
  return UI_ERROR_NONE;
}

/** \brief slide_toggle_cva_register_on_change */
static ui_error_t slide_toggle_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_slide_toggle_base *toggle =
      (struct ui_slide_toggle_base *)component;
  if (!toggle)
    return UI_ERROR_INVALID_ARGUMENT;
  toggle->cva_on_change = callback;
  toggle->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t slide_toggle_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_slide_toggle_base *toggle =
      (struct ui_slide_toggle_base *)component;
  if (!toggle)
    return UI_ERROR_INVALID_ARGUMENT;
  toggle->cva_on_touched = callback;
  toggle->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t slide_toggle_cva_set_disabled_state(void *component,
                                                      int is_disabled) {
  struct ui_slide_toggle_base *toggle =
      (struct ui_slide_toggle_base *)component;
  if (!toggle)
    return UI_ERROR_INVALID_ARGUMENT;
  toggle->disabled = is_disabled;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_slide_toggle_base_create(struct ui_slide_toggle_base **out_toggle,
                            struct ui_control_value_accessor *out_cva) {
  struct ui_slide_toggle_base *toggle;
  ui_error_t rc = UI_ERROR_NONE;

  if (!out_toggle) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  toggle = (struct ui_slide_toggle_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_slide_toggle_base));
  if (!toggle) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  toggle->checked = 0;
  toggle->disabled = 0;
  toggle->is_dragging = 0;
  toggle->drag_offset_x = 0.0f;
  toggle->drag_start_offset_x = 0.0f;

  toggle->cva_on_change = NULL;
  toggle->cva_on_change_user_data = NULL;
  toggle->cva_on_touched = NULL;
  toggle->cva_on_touched_user_data = NULL;

  rc = ui_gesture_recognizer_create(&toggle->recognizer);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(toggle);
    goto cleanup;
  }

  if (out_cva) {
    out_cva->write_value = slide_toggle_cva_write_value;
    out_cva->register_on_change = slide_toggle_cva_register_on_change;
    out_cva->register_on_touched = slide_toggle_cva_register_on_touched;
    out_cva->set_disabled_state = slide_toggle_cva_set_disabled_state;
  }

  *out_toggle = toggle;

cleanup:
  return rc;
}

ui_error_t ui_slide_toggle_base_destroy(struct ui_slide_toggle_base *toggle) {
  if (!toggle) {
    return UI_ERROR_NONE;
  }
  (void)ui_gesture_recognizer_destroy(toggle->recognizer);
  C_MULTIPLATFORM_FREE(toggle);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_slide_toggle_base_set_checked(struct ui_slide_toggle_base *toggle,
                                            int checked) {
  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  toggle->checked = (checked != 0);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_slide_toggle_base_get_checked(const struct ui_slide_toggle_base *toggle,
                                 int *out_checked) {
  if (!toggle || !out_checked) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_checked = toggle->checked;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_slide_toggle_base_set_disabled(struct ui_slide_toggle_base *toggle,
                                  int disabled) {
  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  toggle->disabled = (disabled != 0);
  if (toggle->disabled) {
    toggle->is_dragging = 0;
    toggle->drag_offset_x = 0.0f;
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_slide_toggle_base_get_disabled(const struct ui_slide_toggle_base *toggle,
                                  int *out_disabled) {
  if (!toggle || !out_disabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_disabled = toggle->disabled;
  return UI_ERROR_NONE;
}

ui_error_t ui_slide_toggle_base_toggle(struct ui_slide_toggle_base *toggle) {
  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!toggle->disabled) {
    toggle->checked = !toggle->checked;
    {
      ui_error_t rc1 = trigger_cva_touched(toggle);
      if (rc1 != UI_ERROR_NONE)
        return rc1;
    }
    {
      ui_error_t rc2 = trigger_cva_change(toggle);
      if (rc2 != UI_ERROR_NONE)
        return rc2;
    }
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_slide_toggle_base_process_event(struct ui_slide_toggle_base *toggle,
                                   const struct ui_event *event,
                                   double timestamp_ms) {
  ui_error_t rc = UI_ERROR_NONE;
  struct ui_gesture_event gesture_event;

  if (!toggle || !event) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  if (toggle->disabled) {
    goto cleanup;
  }

  rc = ui_gesture_recognizer_process_event(toggle->recognizer, event,
                                           timestamp_ms, &gesture_event);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  if (gesture_event.type == UI_GESTURE_TAP) {
    toggle->checked = !toggle->checked;
    toggle->is_dragging = 0;
    toggle->drag_offset_x = 0.0f;
    {
      ui_error_t rc1 = trigger_cva_touched(toggle);
      if (rc1 != UI_ERROR_NONE)
        return rc1;
    }
    {
      ui_error_t rc2 = trigger_cva_change(toggle);
      if (rc2 != UI_ERROR_NONE)
        return rc2;
    }
  } else if (gesture_event.type == UI_GESTURE_PAN) {
    if (gesture_event.state == UI_GESTURE_STATE_BEGAN) {
      toggle->is_dragging = 1;
      toggle->drag_start_offset_x =
          toggle->checked ? 20.0f : 0.0f; /* Assumes approx 20px track */
      toggle->drag_offset_x = toggle->drag_start_offset_x;
    } else if (gesture_event.state == UI_GESTURE_STATE_CHANGED) {
      toggle->drag_offset_x += gesture_event.delta_x;
    } else if (gesture_event.state == UI_GESTURE_STATE_ENDED) {
      toggle->drag_offset_x += gesture_event.delta_x;
      toggle->is_dragging = 0;

      /* If dragged past halfway point (10px), flip state based on drag
       * direction */
      {
        int changed = 0;
        float drag_diff = toggle->drag_offset_x - toggle->drag_start_offset_x;

        if (toggle->checked) {
          if (drag_diff < -10.0f) {
            toggle->checked = 0;
            changed = 1;
          }
        } else {
          if (drag_diff > 10.0f) {
            toggle->checked = 1;
            changed = 1;
          }
        }

        if (changed) {
          {
            ui_error_t rc1 = trigger_cva_touched(toggle);
            if (rc1 != UI_ERROR_NONE)
              return rc1;
          }
          {
            ui_error_t rc2 = trigger_cva_change(toggle);
            if (rc2 != UI_ERROR_NONE)
              return rc2;
          }
        }
      }
      toggle->drag_offset_x = 0.0f;
    } else {
      /* UI_GESTURE_STATE_CANCELLED */
      toggle->drag_offset_x += gesture_event.delta_x;
      toggle->is_dragging = 0;
      toggle->drag_offset_x = 0.0f;
    }
  }

cleanup:
  return rc;
}

/** \brief ui_error */
ui_error_t
ui_slide_toggle_base_get_drag_offset(const struct ui_slide_toggle_base *toggle,
                                     float *out_offset_x) {
  if (!toggle || !out_offset_x) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_offset_x = toggle->drag_offset_x;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_slide_toggle_base_is_dragging(const struct ui_slide_toggle_base *toggle,
                                 int *out_is_dragging) {
  if (!toggle || !out_is_dragging) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_dragging = toggle->is_dragging;
  return UI_ERROR_NONE;
}
