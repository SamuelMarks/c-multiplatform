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
static enum ui_error ui_roundf_fallback(float number, float *out_val) {
  if (!out_val)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_val =
      (float)(number < 0.0f ? ceil(number - 0.5f) : floor(number + 0.5f));
  return UI_ERROR_NONE;
}

static enum ui_error internal_strdup(const char *src, char **out_str) {
  size_t len;
  char *copy;
  if (!src) {
    *out_str = NULL;
    return UI_ERROR_NONE;
  }
  len = strlen(src);
  copy = (char *)UI_MALLOC(len + 1);
  if (!copy) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  strcpy(copy, src);
  *out_str = copy;
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

  enum ui_error (*cva_on_change)(union ui_signal_payload new_value,
                                 void *user_data);
  void *cva_on_change_user_data;

  enum ui_error (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data;

  int is_disabled;
};

static enum ui_error update_dom_state(struct ui_wheel_picker_base *picker) {
  if (picker && picker->component && picker->component->shadow_root) {
    char buf[64];
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%d", picker->selected_index);
#else
    sprintf(buf, "%d", picker->selected_index);
#endif
    ui_dom_node_set_attribute(picker->component->shadow_root, "aria-valuenow",
                              buf);
  }
  return UI_ERROR_NONE;
}

static enum ui_error trigger_cva_change(struct ui_wheel_picker_base *picker) {
  if (picker && picker->cva_on_change) {
    union ui_signal_payload payload;
    payload.int_val = picker->selected_index;
    return picker->cva_on_change(payload, picker->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

static enum ui_error trigger_cva_touched(struct ui_wheel_picker_base *picker) {
  if (picker && picker->cva_on_touched) {
    return picker->cva_on_touched(picker->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

static enum ui_error
wheel_picker_cva_write_value(void *component, union ui_signal_payload value) {
  struct ui_wheel_picker_base *picker =
      (struct ui_wheel_picker_base *)component;

  if (!picker)
    return UI_ERROR_INVALID_ARGUMENT;

  return ui_wheel_picker_base_set_selected_index(picker, value.int_val);
}

/** \brief wheel_picker_cva_register_on_change */
static enum ui_error wheel_picker_cva_register_on_change(
    void *component,
    enum ui_error (*callback)(union ui_signal_payload new_value,
                              void *user_data),
    void *user_data) {
  struct ui_wheel_picker_base *picker =
      (struct ui_wheel_picker_base *)component;
  if (!picker)
    return UI_ERROR_INVALID_ARGUMENT;
  picker->cva_on_change = callback;
  picker->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static enum ui_error
wheel_picker_cva_register_on_touched(void *component,
                                     enum ui_error (*callback)(void *user_data),
                                     void *user_data) {
  struct ui_wheel_picker_base *picker =
      (struct ui_wheel_picker_base *)component;
  if (!picker)
    return UI_ERROR_INVALID_ARGUMENT;
  picker->cva_on_touched = callback;
  picker->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static enum ui_error wheel_picker_cva_set_disabled_state(void *component,
                                                         int is_disabled) {
  struct ui_wheel_picker_base *picker =
      (struct ui_wheel_picker_base *)component;
  if (!picker)
    return UI_ERROR_INVALID_ARGUMENT;
  picker->is_disabled = is_disabled;
  ui_dom_node_set_attribute(picker->component->shadow_root, "aria-disabled",
                            is_disabled ? "true" : "false");
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_wheel_picker_base_create(struct ui_wheel_picker_base **out_picker,
                            struct ui_control_value_accessor *out_cva) {
  enum ui_error rc;
  struct ui_wheel_picker_base *picker;
  struct ui_dom_node *root_node = NULL;

  if (!out_picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker = (struct ui_wheel_picker_base *)UI_MALLOC(
      sizeof(struct ui_wheel_picker_base));
  if (!picker) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(picker, 0, sizeof(struct ui_wheel_picker_base));

  rc = ui_component_create(&picker->component);
  if (rc != UI_ERROR_NONE) {
    UI_FREE(picker);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(picker->component);
    UI_FREE(picker);
    return rc;
  }

  ui_dom_node_set_tag_name(root_node, "div");
  ui_dom_node_set_attribute(root_node, "role", "listbox");
  ui_dom_node_set_attribute(root_node, "tabindex", "0");
  picker->component->shadow_root = root_node;

  rc = ui_gesture_recognizer_create(&picker->gesture_recognizer);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root_node);
    picker->component->shadow_root = NULL;
    ui_component_destroy(picker->component);
    UI_FREE(picker);
    return rc;
  }

  picker->is_disabled = 0;

  if (out_cva) {
    out_cva->write_value = wheel_picker_cva_write_value;
    out_cva->register_on_change = wheel_picker_cva_register_on_change;
    out_cva->register_on_touched = wheel_picker_cva_register_on_touched;
    out_cva->set_disabled_state = wheel_picker_cva_set_disabled_state;
  }

  (void)update_dom_state(picker);

  *out_picker = picker;
  return UI_ERROR_NONE;
}

void ui_wheel_picker_base_destroy(struct ui_wheel_picker_base *picker) {
  int i;
  if (!picker) {
    return;
  }

  for (i = 0; i < picker->item_count; i++) {
    UI_FREE(picker->items[i]);
  }
  if (picker->items) {
    UI_FREE(picker->items);
  }

  if (picker->gesture_recognizer) {
    ui_gesture_recognizer_destroy(picker->gesture_recognizer);
  }

  if (picker->component) {
    if (picker->component->shadow_root) {
      ui_dom_node_destroy(picker->component->shadow_root);
      picker->component->shadow_root = NULL;
    }
    ui_component_destroy(picker->component);
  }
  UI_FREE(picker);
}

/** \brief ui_error */
enum ui_error
ui_wheel_picker_base_set_items(struct ui_wheel_picker_base *picker,
                               const char *const *items, int count) {
  int i;
  if (!picker || (!items && count > 0)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < picker->item_count; i++) {
    UI_FREE(picker->items[i]);
  }
  if (picker->items) {
    UI_FREE(picker->items);
    picker->items = NULL;
  }

  picker->item_count = count;

  if (count > 0) {
    picker->items = (char **)UI_MALLOC(sizeof(char *) * count);
    if (!picker->items) {
      picker->item_count = 0;
      return UI_ERROR_OUT_OF_MEMORY;
    }
    for (i = 0; i < count; i++) {
      internal_strdup(items[i], &picker->items[i]);
    }
  }

  if (picker->selected_index >= count) {
    picker->selected_index = count > 0 ? count - 1 : 0;
    picker->scroll_offset =
        (float)picker->selected_index * UI_WHEEL_PICKER_ITEM_HEIGHT;
  }

  (void)update_dom_state(picker);

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_wheel_picker_base_set_looping(struct ui_wheel_picker_base *picker,
                                 int is_looping) {
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  picker->is_looping = is_looping;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
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
    (void)update_dom_state(picker);
    if (picker->on_change) {
      enum ui_error change_rc = picker->on_change(
          picker, picker->selected_index, picker->on_change_user_data);
      if (change_rc != UI_ERROR_NONE)
        return change_rc;
    }
  }
  return UI_ERROR_NONE;
}

/** \brief ui_wheel_picker_base_get_selected_index */
enum ui_error ui_wheel_picker_base_get_selected_index(
    const struct ui_wheel_picker_base *picker, int *out_index) {
  if (!picker || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_index = picker->selected_index;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
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
enum ui_error
ui_wheel_picker_base_process_event(struct ui_wheel_picker_base *picker,
                                   const struct ui_event *event,
                                   double timestamp_ms) {
  struct ui_gesture_event ge = {0};
  enum ui_error rc;

  if (!picker || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (picker->is_disabled) {
    return UI_ERROR_NONE;
  }

  (void)trigger_cva_touched(picker);

  /* Keyboard Support */
  if (event->type == UI_EVENT_KEY_DOWN) {
    if (event->event_data.keyboard.key_code == UI_KEY_UP) {
      ui_wheel_picker_base_set_selected_index(picker,
                                              picker->selected_index - 1);
      return UI_ERROR_NONE;
    } else if (event->event_data.keyboard.key_code == UI_KEY_DOWN) {
      ui_wheel_picker_base_set_selected_index(picker,
                                              picker->selected_index + 1);
      return UI_ERROR_NONE;
    }
  }

  rc = ui_gesture_recognizer_process_event(picker->gesture_recognizer, event,
                                           timestamp_ms, &ge);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (ge.type == UI_GESTURE_PAN) {
    if (ge.state == UI_GESTURE_STATE_BEGAN) {
      picker->is_dragging = 1;
      picker->velocity = 0.0f;
    } else if (ge.state == UI_GESTURE_STATE_CHANGED) {
      picker->scroll_offset -= ge.delta_y; /* dragging up increases offset */
      picker->velocity = -ge.velocity_y;
    } else if (ge.state == UI_GESTURE_STATE_ENDED ||
               ge.state == UI_GESTURE_STATE_CANCELLED) {
      picker->is_dragging = 0;
    }
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_wheel_picker_base_on_tick(struct ui_wheel_picker_base *picker,
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
        ui_roundf_fallback(picker->scroll_offset / UI_WHEEL_PICKER_ITEM_HEIGHT,
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
          (void)update_dom_state(picker);
          if (picker->on_change) {
            enum ui_error change_rc = picker->on_change(
                picker, picker->selected_index, picker->on_change_user_data);
            if (change_rc != UI_ERROR_NONE)
              return change_rc;
          }
          (void)trigger_cva_change(picker);
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
enum ui_error
ui_wheel_picker_base_get_component(struct ui_wheel_picker_base *picker,
                                   struct ui_component **out_component) {
  if (!picker || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = picker->component;
  return UI_ERROR_NONE;
}
