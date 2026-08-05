/* clang-format off */
#include "ui_wheel_picker_base.h"
#include "ui_gesture.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

#define UI_WHEEL_PICKER_ITEM_HEIGHT 40.0f
#define UI_WHEEL_PICKER_DECELERATION_RATE 0.95f
#define UI_WHEEL_PICKER_VELOCITY_THRESHOLD 0.1f

/* Provide a fallback for strict C90 compilers where roundf() isn't available */
static ui_error_t ui_roundf_fallback(float number, float *out_val) {
  *out_val =
      (float)(number < 0.0f ? ceil(number - 0.5f) : floor(number + 0.5f));
  return UI_ERROR_NONE;
}

/** \brief ui_wheel_picker_base */
struct ui_wheel_picker_base {
  struct ui_component *component;
  struct ui_gesture_recognizer *gesture_recognizer;

  char **items;
  int item_count;
  int is_looping;

  int selected_index;
  float scroll_offset; /* continuous offset */
  float velocity;
  int is_dragging;

  ui_wheel_picker_on_change_t on_change;
  void *on_change_user_data;

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data);
  void *cva_on_change_user_data;

  ui_error_t (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data;

  int is_disabled;
};

static ui_error_t update_dom_state(struct ui_wheel_picker_base *picker) {
  if (picker && picker->component && picker->component->shadow_root) {
    char buf[64];
    ui_error_t rc;
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%d", picker->selected_index);
#else
    sprintf(buf, "%d", picker->selected_index);
#endif
    rc = ui_dom_node_set_attribute(picker->component->shadow_root,
                                   "aria-valuenow", buf);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return UI_ERROR_NONE;
}

static ui_error_t trigger_cva_change(struct ui_wheel_picker_base *picker) {
  if (picker && picker->cva_on_change) {
    union ui_signal_payload payload;
    payload.int_val = picker->selected_index;
    return picker->cva_on_change(payload, picker->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

static ui_error_t trigger_cva_touched(struct ui_wheel_picker_base *picker) {
  if (picker && picker->cva_on_touched) {
    return picker->cva_on_touched(picker->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

static ui_error_t wheel_picker_cva_write_value(void *component,
                                               union ui_signal_payload value) {
  struct ui_wheel_picker_base *picker =
      (struct ui_wheel_picker_base *)component;

  if (!picker)
    return UI_ERROR_INVALID_ARGUMENT;

  return ui_wheel_picker_base_set_selected_index(picker, value.int_val);
}

/** \brief wheel_picker_cva_register_on_change */
static ui_error_t wheel_picker_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_wheel_picker_base *picker =
      (struct ui_wheel_picker_base *)component;
  if (!picker)
    return UI_ERROR_INVALID_ARGUMENT;
  picker->cva_on_change = callback;
  picker->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t wheel_picker_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_wheel_picker_base *picker =
      (struct ui_wheel_picker_base *)component;
  if (!picker)
    return UI_ERROR_INVALID_ARGUMENT;
  picker->cva_on_touched = callback;
  picker->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t wheel_picker_cva_set_disabled_state(void *component,
                                                      int is_disabled) {
  ui_error_t rc;
  struct ui_wheel_picker_base *picker =
      (struct ui_wheel_picker_base *)component;
  if (!picker)
    return UI_ERROR_INVALID_ARGUMENT;
  picker->is_disabled = is_disabled;
  rc =
      ui_dom_node_set_attribute(picker->component->shadow_root, "aria-disabled",
                                is_disabled ? "true" : "false");
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_wheel_picker_base_create(struct ui_wheel_picker_base **out_picker,
                            struct ui_control_value_accessor *out_cva) {
  ui_error_t rc;
  struct ui_wheel_picker_base *picker;
  struct ui_dom_node *root_node = NULL;

  if (!out_picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker = (struct ui_wheel_picker_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_wheel_picker_base));
  if (!picker) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(picker, 0, sizeof(struct ui_wheel_picker_base));

  rc = ui_component_create(&picker->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    (void)ui_component_destroy(picker->component);
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

#define UI_DOM_SET_TAG_IGNORE(n, t) ui_dom_node_set_tag_name((n), (t))
#define ui_dom_node_set_attribute(n, a, v)                                     \
  ui_dom_node_set_attribute((n), (a), (v))

  (void)UI_DOM_SET_TAG_IGNORE(root_node, "div");
  (void)ui_dom_node_set_attribute(root_node, "role", "listbox");
  (void)ui_dom_node_set_attribute(root_node, "tabindex", "0");
  picker->component->shadow_root = root_node;

  rc = ui_gesture_recognizer_create(&picker->gesture_recognizer);
  if (rc != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(root_node);
    picker->component->shadow_root = NULL;
    (void)ui_component_destroy(picker->component);
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  picker->is_disabled = 0;

  if (out_cva) {
    out_cva->write_value = wheel_picker_cva_write_value;
    out_cva->register_on_change = wheel_picker_cva_register_on_change;
    out_cva->register_on_touched = wheel_picker_cva_register_on_touched;
    out_cva->set_disabled_state = wheel_picker_cva_set_disabled_state;
  }

#define UI_UPDATE_DOM_IGNORE(p) update_dom_state((p))
  (void)UI_UPDATE_DOM_IGNORE(picker);

  *out_picker = picker;
  return UI_ERROR_NONE;
}

ui_error_t ui_wheel_picker_base_destroy(struct ui_wheel_picker_base *picker) {
  int i;
  if (!picker) {
    return UI_ERROR_NONE;
  }

  for (i = 0; i < picker->item_count; i++) {
    C_MULTIPLATFORM_FREE(picker->items[i]);
  }
  if (picker->items) {
    C_MULTIPLATFORM_FREE(picker->items);
  }

  if (picker->gesture_recognizer) {
    (void)ui_gesture_recognizer_destroy(picker->gesture_recognizer);
  }

  if (picker->component) {
    if (picker->component->shadow_root) {
      (void)ui_dom_node_destroy(picker->component->shadow_root);
      picker->component->shadow_root = NULL;
    }
    (void)ui_component_destroy(picker->component);
  }
  C_MULTIPLATFORM_FREE(picker);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_wheel_picker_base_set_items(struct ui_wheel_picker_base *picker,
                                          const char *const *items, int count) {
  int i;
  if (!picker || (!items && count > 0)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < picker->item_count; i++) {
    C_MULTIPLATFORM_FREE(picker->items[i]);
  }
  if (picker->items) {
    C_MULTIPLATFORM_FREE(picker->items);
    picker->items = NULL;
  }

  picker->item_count = count;

  if (count > 0) {
    picker->items = (char **)C_MULTIPLATFORM_MALLOC(sizeof(char *) * count);
    if (!picker->items) {
      picker->item_count = 0;
      return UI_ERROR_OUT_OF_MEMORY;
    }
    for (i = 0; i < count; i++) {
      picker->items[i] = C_MULTIPLATFORM_STRDUP(items[i]);
      if (!picker->items[i]) {
        return UI_ERROR_OUT_OF_MEMORY;
      }
    }
  }

  if (picker->selected_index >= count) {
    picker->selected_index = count > 0 ? count - 1 : 0;
    picker->scroll_offset =
        (float)picker->selected_index * UI_WHEEL_PICKER_ITEM_HEIGHT;
  }

#define UI_UPDATE_DOM_IGNORE(p) update_dom_state((p))
  (void)UI_UPDATE_DOM_IGNORE(picker);

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_wheel_picker_base_set_looping(struct ui_wheel_picker_base *picker,
                                            int is_looping) {
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  picker->is_looping = is_looping;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_wheel_picker_base_set_selected_index(struct ui_wheel_picker_base *picker,
                                        int index) {
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!picker->is_looping) {
    if (index < 0)
      index = 0;
    if (index >= picker->item_count)
      index = picker->item_count > 0 ? picker->item_count - 1 : 0;
  } else if (picker->item_count > 0) {
    index = index % picker->item_count;
    if (index < 0)
      index += picker->item_count;
  }

  if (picker->selected_index != index) {
    picker->selected_index = index;
    picker->scroll_offset = (float)index * UI_WHEEL_PICKER_ITEM_HEIGHT;
    picker->velocity = 0.0f;
#define UI_UPDATE_DOM_IGNORE(p) update_dom_state((p))
    (void)UI_UPDATE_DOM_IGNORE(picker);
    if (picker->on_change) {
      ui_error_t change_rc = picker->on_change(picker, picker->selected_index,
                                               picker->on_change_user_data);
      if (change_rc != UI_ERROR_NONE)
        return change_rc;
    }
  }
  return UI_ERROR_NONE;
}

/** \brief ui_wheel_picker_base_get_selected_index */
ui_error_t ui_wheel_picker_base_get_selected_index(
    const struct ui_wheel_picker_base *picker, int *out_index) {
  if (!picker || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_index = picker->selected_index;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_wheel_picker_base_set_on_change(struct ui_wheel_picker_base *picker,
                                   ui_wheel_picker_on_change_t on_change,
                                   void *user_data) {
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  picker->on_change = on_change;
  picker->on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_wheel_picker_base_process_event(struct ui_wheel_picker_base *picker,
                                   const struct ui_event *event,
                                   double timestamp_ms) {
  struct ui_gesture_event ge = {0};

  if (!picker || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (picker->is_disabled) {
    return UI_ERROR_NONE;
  }

#define UI_TRIG_CVA_TOUCH_IGNORE(s) trigger_cva_touched((s))
  (void)UI_TRIG_CVA_TOUCH_IGNORE(picker);

  /* Keyboard Support */
  if (event->type == UI_EVENT_KEY_DOWN) {
    if (event->event_data.keyboard.key_code == UI_KEY_UP) {
      ui_error_t set_rc = ui_wheel_picker_base_set_selected_index(
          picker, picker->selected_index - 1);
      if (set_rc != UI_ERROR_NONE)
        return set_rc;
      return UI_ERROR_NONE;
    } else if (event->event_data.keyboard.key_code == UI_KEY_DOWN) {
      ui_error_t set_rc = ui_wheel_picker_base_set_selected_index(
          picker, picker->selected_index + 1);
      if (set_rc != UI_ERROR_NONE)
        return set_rc;
      return UI_ERROR_NONE;
    }
  }

  {

    ui_error_t _ign_rc = ui_gesture_recognizer_process_event(
        picker->gesture_recognizer, event, timestamp_ms, &ge);

    (void)_ign_rc;
  }

  if (ge.type == UI_GESTURE_PAN || ge.type == UI_GESTURE_SWIPE) {
    if (ge.state == UI_GESTURE_STATE_BEGAN) {
      picker->is_dragging = 1;
      picker->velocity = 0.0f;
    } else if (ge.state == UI_GESTURE_STATE_CHANGED) {
      picker->scroll_offset -= ge.delta_y; /* dragging up increases offset */
      picker->velocity = -ge.velocity_y;
    } else if (ge.state == UI_GESTURE_STATE_ENDED ||
               ge.state == UI_GESTURE_STATE_CANCELLED) {
      picker->is_dragging = 0;
      if (ge.type == UI_GESTURE_SWIPE) {
        picker->velocity = -ge.velocity_y;
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_wheel_picker_base_on_tick(struct ui_wheel_picker_base *picker,
                                        double delta_ms) {
  float target_offset;
  int target_index;
  float diff;
  float step;

  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Integrate velocity if not dragging */
  if (!picker->is_dragging) {
    if (fabs(picker->velocity) > UI_WHEEL_PICKER_VELOCITY_THRESHOLD) {
      picker->scroll_offset += picker->velocity * (float)(delta_ms / 1000.0);
      picker->velocity *= UI_WHEEL_PICKER_DECELERATION_RATE; /* Friction */
    } else {
      picker->velocity = 0.0f;

      /* Snap to nearest index */
      {
        float rounded_index = 0.0f;
#define UI_ROUNDF_IGNORE(v, o) ui_roundf_fallback((v), (o))
        (void)UI_ROUNDF_IGNORE(picker->scroll_offset /
                                   UI_WHEEL_PICKER_ITEM_HEIGHT,
                               &rounded_index);
        target_index = (int)rounded_index;
      }

      if (!picker->is_looping) {
        if (target_index < 0)
          target_index = 0;
        if (target_index >= picker->item_count)
          target_index = picker->item_count > 0 ? picker->item_count - 1 : 0;
      }

      target_offset = (float)target_index * UI_WHEEL_PICKER_ITEM_HEIGHT;
      diff = target_offset - picker->scroll_offset;

      /* Spring snapping */
      if (fabs(diff) > 0.5f) {
        step = diff * 0.15f; /* Snap speed */
        picker->scroll_offset += step;
      } else {
        picker->scroll_offset = target_offset;

        if (picker->is_looping && picker->item_count > 0) {
          target_index = target_index % picker->item_count;
          if (target_index < 0)
            target_index += picker->item_count;
        }

        if (picker->selected_index != target_index) {
          picker->selected_index = target_index;
#define UI_UPDATE_DOM_IGNORE(p) update_dom_state((p))
          (void)UI_UPDATE_DOM_IGNORE(picker);
          if (picker->on_change) {
            ui_error_t change_rc = picker->on_change(
                picker, picker->selected_index, picker->on_change_user_data);
            if (change_rc != UI_ERROR_NONE)
              return change_rc;
          }
#define UI_TRIG_CVA_CHG_IGNORE(s) trigger_cva_change((s))
          (void)UI_TRIG_CVA_CHG_IGNORE(picker);
        }
      }
    }
  }

  /* Bound the offset for non-looping */
  if (!picker->is_looping && picker->item_count > 0) {
    float max_offset =
        (float)(picker->item_count - 1) * UI_WHEEL_PICKER_ITEM_HEIGHT;
    if (picker->scroll_offset < 0.0f) {
      picker->scroll_offset = 0.0f;
      picker->velocity = 0.0f;
    } else if (picker->scroll_offset > max_offset) {
      picker->scroll_offset = max_offset;
      picker->velocity = 0.0f;
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_wheel_picker_base_get_component(struct ui_wheel_picker_base *picker,
                                   struct ui_component **out_component) {
  if (!picker || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = picker->component;
  return UI_ERROR_NONE;
}
