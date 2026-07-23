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

  enum ui_error (*cva_on_change)(union ui_signal_payload new_value,
                                 void *user_data);
  void *cva_on_change_user_data;

  enum ui_error (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data;
};

static enum ui_error trigger_cva_change(struct ui_slide_toggle_base *toggle) {
  if (toggle && toggle->cva_on_change) {
    union ui_signal_payload payload;
    payload.bool_val = toggle->checked;
    return toggle->cva_on_change(payload, toggle->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

static enum ui_error trigger_cva_touched(struct ui_slide_toggle_base *toggle) {
  if (toggle && toggle->cva_on_touched) {
    return toggle->cva_on_touched(toggle->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

static enum ui_error
slide_toggle_cva_write_value(void *component, union ui_signal_payload value) {
  struct ui_slide_toggle_base *toggle =
      (struct ui_slide_toggle_base *)component;

  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toggle->checked = value.bool_val;
  return UI_ERROR_NONE;
}

/** \brief slide_toggle_cva_register_on_change */
static enum ui_error slide_toggle_cva_register_on_change(
    void *component,
    enum ui_error (*callback)(union ui_signal_payload new_value,
                              void *user_data),
    void *user_data) {
  struct ui_slide_toggle_base *toggle =
      (struct ui_slide_toggle_base *)component;
  if (!toggle)
    return UI_ERROR_INVALID_ARGUMENT;
  toggle->cva_on_change = callback;
  toggle->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static enum ui_error
slide_toggle_cva_register_on_touched(void *component,
                                     enum ui_error (*callback)(void *user_data),
                                     void *user_data) {
  struct ui_slide_toggle_base *toggle =
      (struct ui_slide_toggle_base *)component;
  if (!toggle)
    return UI_ERROR_INVALID_ARGUMENT;
  toggle->cva_on_touched = callback;
  toggle->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static enum ui_error slide_toggle_cva_set_disabled_state(void *component,
                                                         int is_disabled) {
  struct ui_slide_toggle_base *toggle =
      (struct ui_slide_toggle_base *)component;
  if (!toggle)
    return UI_ERROR_INVALID_ARGUMENT;
  toggle->disabled = is_disabled;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_slide_toggle_base_create(struct ui_slide_toggle_base **out_toggle,
                            struct ui_control_value_accessor *out_cva) {
  struct ui_slide_toggle_base *toggle;
  enum ui_error rc = UI_ERROR_NONE;

  if (!out_toggle) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  toggle = (struct ui_slide_toggle_base *)UI_MALLOC(
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
    UI_FREE(toggle);
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

void ui_slide_toggle_base_destroy(struct ui_slide_toggle_base *toggle) {
  if (!toggle) {
    return;
  }
  if (toggle->recognizer) {
    ui_gesture_recognizer_destroy(toggle->recognizer);
  }
  UI_FREE(toggle);
}

/** \brief ui_error */
enum ui_error
ui_slide_toggle_base_set_checked(struct ui_slide_toggle_base *toggle,
                                 int checked) {
  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  toggle->checked = (checked != 0);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_slide_toggle_base_get_checked(const struct ui_slide_toggle_base *toggle,
                                 int *out_checked) {
  if (!toggle || !out_checked) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_checked = toggle->checked;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
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
enum ui_error
ui_slide_toggle_base_get_disabled(const struct ui_slide_toggle_base *toggle,
                                  int *out_disabled) {
  if (!toggle || !out_disabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_disabled = toggle->disabled;
  return UI_ERROR_NONE;
}

enum ui_error ui_slide_toggle_base_toggle(struct ui_slide_toggle_base *toggle) {
  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!toggle->disabled) {
    toggle->checked = !toggle->checked;
    (void)trigger_cva_touched(toggle);
    (void)trigger_cva_change(toggle);
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_slide_toggle_base_process_event(struct ui_slide_toggle_base *toggle,
                                   const struct ui_event *event,
                                   double timestamp_ms) {
  enum ui_error rc = UI_ERROR_NONE;
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

  if (gesture_event.type == UI_GESTURE_TAP &&
      gesture_event.state == UI_GESTURE_STATE_ENDED) {
    toggle->checked = !toggle->checked;
    toggle->is_dragging = 0;
    toggle->drag_offset_x = 0.0f;
    (void)trigger_cva_touched(toggle);
    (void)trigger_cva_change(toggle);
  } else if (gesture_event.type == UI_GESTURE_PAN) {
    if (gesture_event.state == UI_GESTURE_STATE_BEGAN) {
      toggle->is_dragging = 1;
      toggle->drag_start_offset_x =
          toggle->checked ? 20.0f : 0.0f; /* Assumes approx 20px track */
      toggle->drag_offset_x = toggle->drag_start_offset_x;
    } else if (gesture_event.state == UI_GESTURE_STATE_CHANGED) {
      toggle->drag_offset_x += gesture_event.delta_x;
    } else if (gesture_event.state == UI_GESTURE_STATE_ENDED ||
               gesture_event.state == UI_GESTURE_STATE_CANCELLED) {
      toggle->drag_offset_x += gesture_event.delta_x;
      toggle->is_dragging = 0;

      /* If dragged past halfway point (10px), flip state based on drag
       * direction */
      if (gesture_event.state == UI_GESTURE_STATE_ENDED) {
        int changed = 0;
        if (toggle->checked &&
            (toggle->drag_offset_x - toggle->drag_start_offset_x) < -10.0f) {
          toggle->checked = 0;
          changed = 1;
        } else if (!toggle->checked && (toggle->drag_offset_x -
                                        toggle->drag_start_offset_x) > 10.0f) {
          toggle->checked = 1;
          changed = 1;
        }
        if (changed) {
          (void)trigger_cva_touched(toggle);
          (void)trigger_cva_change(toggle);
        }
      }
      toggle->drag_offset_x = 0.0f;
    }
  }

cleanup:
  return rc;
}

/** \brief ui_error */
enum ui_error
ui_slide_toggle_base_get_drag_offset(const struct ui_slide_toggle_base *toggle,
                                     float *out_offset_x) {
  if (!toggle || !out_offset_x) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_offset_x = toggle->drag_offset_x;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_slide_toggle_base_is_dragging(const struct ui_slide_toggle_base *toggle,
                                 int *out_is_dragging) {
  if (!toggle || !out_is_dragging) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_dragging = toggle->is_dragging;
  return UI_ERROR_NONE;
}
