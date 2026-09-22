/**
 * @file ui_wheel_picker_base.c
 * @brief Implementation of the wheel picker base component.
 */

/* clang-format off */
#include "ui_wheel_picker_base.h"
#include "ui_gesture.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

/** @cond */
#define UI_WHEEL_PICKER_ITEM_HEIGHT 40.0f
/** @endcond */
/** @cond */
#define UI_WHEEL_PICKER_DECELERATION_RATE 0.95f
/** @endcond */
/** @cond */
#define UI_WHEEL_PICKER_VELOCITY_THRESHOLD 0.1f
/** @endcond */

/* Provide a fallback for strict C90 compilers where roundf() isn't available */
/**
 * @brief ui_roundf_fallback.
 * @param number Parameter number.
 * @param out_val Parameter out_val.
 * @return Return value.
 */
static ui_error_t ui_roundf_fallback(float number, float *out_val) {
  *out_val =
      (float)(number < 0.0f ? ceil(number - 0.5f) : floor(number + 0.5f));
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
int g_wheel_mock_fail = 0;

/**
 * @brief mock_wheel_component_destroy.
 * @param comp Component.
 * @return Return value.
 */
static ui_error_t mock_wheel_component_destroy(struct ui_component *comp) {
  if (g_wheel_mock_fail == 1 || g_wheel_mock_fail == 11) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_destroy(comp);
}
/** @cond */
#define ui_component_destroy mock_wheel_component_destroy
/** @endcond */

/**
 * @brief mock_wheel_dom_node_destroy.
 * @param node Node.
 * @return Return value.
 */
static ui_error_t mock_wheel_dom_node_destroy(struct ui_dom_node *node) {
  if (g_wheel_mock_fail == 2 || g_wheel_mock_fail == 12) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dom_node_destroy(node);
}
/** @cond */
#define ui_dom_node_destroy mock_wheel_dom_node_destroy
/** @endcond */

/**
 * @brief mock_wheel_gesture_recognizer_destroy.
 * @param r Recognizer.
 * @return Return value.
 */
static ui_error_t
mock_wheel_gesture_recognizer_destroy(struct ui_gesture_recognizer *r) {
  if (g_wheel_mock_fail == 3 || g_wheel_mock_fail == 13) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_gesture_recognizer_destroy(r);
}
/** @cond */
#define ui_gesture_recognizer_destroy mock_wheel_gesture_recognizer_destroy
/** @endcond */

/**
 * @brief mock_wheel_gesture_recognizer_create.
 * @param out_r Output recognizer.
 * @return Return value.
 */
static ui_error_t
mock_wheel_gesture_recognizer_create(struct ui_gesture_recognizer **out_r) {
  if (g_wheel_mock_fail == 15) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_gesture_recognizer_create(out_r);
}
/** @cond */
#define ui_gesture_recognizer_create mock_wheel_gesture_recognizer_create
/** @endcond */

/**
 * @brief mock_wheel_gesture_recognizer_process_event.
 * @param r Recognizer.
 * @param event Event.
 * @param timestamp_ms Timestamp.
 * @param out_event Output event.
 * @return Return value.
 */
static ui_error_t mock_wheel_gesture_recognizer_process_event(
    struct ui_gesture_recognizer *r, const struct ui_event *event,
    double timestamp_ms, struct ui_gesture_event *out_event) {
  if (g_wheel_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_wheel_mock_fail == 7) {
    out_event->type = UI_GESTURE_PAN;
    out_event->state = UI_GESTURE_STATE_BEGAN;
    return UI_ERROR_NONE;
  }
  if (g_wheel_mock_fail == 8) {
    out_event->type = UI_GESTURE_PAN;
    out_event->state = UI_GESTURE_STATE_CHANGED;
    out_event->delta_y = 10.0f;
    out_event->velocity_y = 20.0f;
    return UI_ERROR_NONE;
  }
  if (g_wheel_mock_fail == 9) {
    out_event->type = UI_GESTURE_SWIPE;
    out_event->state = UI_GESTURE_STATE_ENDED;
    out_event->velocity_y = 50.0f;
    return UI_ERROR_NONE;
  }
  if (g_wheel_mock_fail == 10) {
    out_event->type = UI_GESTURE_PAN;
    out_event->state = UI_GESTURE_STATE_ENDED;
    return UI_ERROR_NONE;
  }
  return ui_gesture_recognizer_process_event(r, event, timestamp_ms, out_event);
}
/** @cond */
#define ui_gesture_recognizer_process_event                                    \
  mock_wheel_gesture_recognizer_process_event
/** @endcond */

/**
 * @brief mock_wheel_roundf_fallback.
 * @param number Parameter number.
 * @param out_val Parameter out_val.
 * @return Return value.
 */
static ui_error_t mock_wheel_roundf_fallback(float number, float *out_val) {
  if (g_wheel_mock_fail == 5) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_roundf_fallback(number, out_val);
}
/** @cond */
#define ui_roundf_fallback mock_wheel_roundf_fallback
/** @endcond */

/**
 * @brief mock_wheel_dom_node_set_attribute.
 * @param node Node.
 * @param name Name.
 * @param val Value.
 * @return Return value.
 */
static ui_error_t mock_wheel_dom_node_set_attribute(struct ui_dom_node *node,
                                                    const char *name,
                                                    const char *val) {
  if (g_wheel_mock_fail == 6) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_wheel_mock_fail == 14 && strcmp(name, "tabindex") == 0) {
    return UI_ERROR_UNKNOWN;
  }
  if ((g_wheel_mock_fail == 11 || g_wheel_mock_fail == 12 ||
       g_wheel_mock_fail == 13) &&
      strcmp(name, "aria-valuenow") == 0) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dom_node_set_attribute(node, name, val);
}
/** @cond */
#define ui_dom_node_set_attribute mock_wheel_dom_node_set_attribute
/** @endcond */
#endif

/**
 * @struct ui_wheel_picker_base
 * @struct ui_wheel_picker_base
 * @brief Internal state for the wheel picker component.
 */
struct ui_wheel_picker_base {
  struct ui_component *component;                   /**< component */
  struct ui_gesture_recognizer *gesture_recognizer; /**< gesture_recognizer */

  char **items;   /**< items */
  int item_count; /**< item_count */
  int is_looping; /**< is_looping */

  int selected_index;                        /**< selected_index */
  float scroll_offset; /**< scroll_offset */ /* continuous offset */
  float velocity;                            /**< velocity */
  int is_dragging;                           /**< is_dragging */

  ui_wheel_picker_on_change_t on_change; /**< on_change */
  void *on_change_user_data;             /**< on_change_user_data */

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data); /**< user_data) */
  void *cva_on_change_user_data;                /**< cva_on_change_user_data */

  ui_error_t (*cva_on_touched)(void *user_data); /**< user_data) */
  void *cva_on_touched_user_data; /**< cva_on_touched_user_data */

  int is_disabled; /**< is_disabled */
};

/**
 * @brief update_dom_state.
 * @param picker Parameter picker.
 * @return Return value.
 */
static ui_error_t update_dom_state(struct ui_wheel_picker_base *picker) {
  if (picker->component && picker->component->shadow_root) {
    char buf[64];
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%d", picker->selected_index);
#else
    sprintf(buf, "%d", picker->selected_index);
#endif
    return ui_dom_node_set_attribute(picker->component->shadow_root,
                                     "aria-valuenow", buf);
  }
  return UI_ERROR_NONE;
}

/**
 * @brief trigger_cva_change.
 * @param picker Parameter picker.
 * @return Return value.
 */
static ui_error_t trigger_cva_change(struct ui_wheel_picker_base *picker) {
  if (picker->cva_on_change) {
    union ui_signal_payload payload;
    payload.int_val = picker->selected_index;
    return picker->cva_on_change(payload, picker->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

/**
 * @brief trigger_cva_touched.
 * @param picker Parameter picker.
 * @return Return value.
 */
static ui_error_t trigger_cva_touched(struct ui_wheel_picker_base *picker) {
  if (picker->cva_on_touched) {
    return picker->cva_on_touched(picker->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

/**
 * @brief wheel_picker_cva_write_value.
 * @param component Parameter component.
 * @param value Parameter value.
 * @return Return value.
 */
static ui_error_t wheel_picker_cva_write_value(void *component,
                                               union ui_signal_payload value) {
  struct ui_wheel_picker_base *picker =
      (struct ui_wheel_picker_base *)component;

  if (!picker)
    return UI_ERROR_INVALID_ARGUMENT;

  return ui_wheel_picker_base_set_selected_index(picker, value.int_val);
}

/**
 * @brief wheel_picker_cva_register_on_change.
 * @param component Parameter component.
 * @param callback Parameter callback.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
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

/**
 * @brief wheel_picker_cva_register_on_touched.
 * @param component Parameter component.
 * @param callback Parameter callback.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
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

/**
 * @brief wheel_picker_cva_set_disabled_state.
 * @param component Parameter component.
 * @param is_disabled Parameter is_disabled.
 * @return Return value.
 */
static ui_error_t wheel_picker_cva_set_disabled_state(void *component,
                                                      int is_disabled) {
  struct ui_wheel_picker_base *picker =
      (struct ui_wheel_picker_base *)component;
  if (!picker)
    return UI_ERROR_INVALID_ARGUMENT;
  picker->is_disabled = is_disabled;
  return ui_dom_node_set_attribute(picker->component->shadow_root,
                                   "aria-disabled",
                                   is_disabled ? "true" : "false");
}

ui_error_t
ui_wheel_picker_base_create(struct ui_wheel_picker_base **out_picker,
                            struct ui_control_value_accessor *out_cva) {
  ui_error_t rc;
  ui_error_t rc_cleanup;
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
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(root_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_set_attribute(root_node, "role", "listbox");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_set_attribute(root_node, "tabindex", "0");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  picker->component->shadow_root = root_node;

  rc = ui_gesture_recognizer_create(&picker->gesture_recognizer);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  picker->is_disabled = 0;

  if (out_cva) {
    out_cva->write_value = wheel_picker_cva_write_value;
    out_cva->register_on_change = wheel_picker_cva_register_on_change;
    out_cva->register_on_touched = wheel_picker_cva_register_on_touched;
    out_cva->set_disabled_state = wheel_picker_cva_set_disabled_state;
  }

  rc = update_dom_state(picker);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  *out_picker = picker;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    rc_cleanup = ui_dom_node_destroy(root_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  if (picker->gesture_recognizer) {
    rc_cleanup = ui_gesture_recognizer_destroy(picker->gesture_recognizer);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  if (picker->component) {
    picker->component->shadow_root = NULL;
    rc_cleanup = ui_component_destroy(picker->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  C_MULTIPLATFORM_FREE(picker);
  return rc;
}

ui_error_t ui_wheel_picker_base_destroy(struct ui_wheel_picker_base *picker) {
  int i;
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t rc_cleanup;
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
    rc_cleanup = ui_gesture_recognizer_destroy(picker->gesture_recognizer);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }

  if (picker->component) {
    if (picker->component->shadow_root) {
      rc_cleanup = ui_dom_node_destroy(picker->component->shadow_root);
      if (rc_cleanup != UI_ERROR_NONE) {
        rc = rc_cleanup;
      }
      picker->component->shadow_root = NULL;
    }
    rc_cleanup = ui_component_destroy(picker->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  C_MULTIPLATFORM_FREE(picker);
  return rc;
}

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
  picker->item_count = 0;

  if (count > 0) {
    picker->items =
        (char **)C_MULTIPLATFORM_MALLOC(sizeof(char *) * (size_t)count);
    if (!picker->items) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    for (i = 0; i < count; i++) {
      picker->items[i] = C_MULTIPLATFORM_STRDUP(items[i]);
      if (!picker->items[i]) {
        int j;
        for (j = 0; j < i; j++) {
          C_MULTIPLATFORM_FREE(picker->items[j]);
        }
        C_MULTIPLATFORM_FREE(picker->items);
        picker->items = NULL;
        return UI_ERROR_OUT_OF_MEMORY;
      }
    }
  }

  picker->item_count = count;

  if (picker->selected_index >= count) {
    picker->selected_index = count > 0 ? count - 1 : 0;
    picker->scroll_offset =
        (float)picker->selected_index * UI_WHEEL_PICKER_ITEM_HEIGHT;
  }

  return update_dom_state(picker);
}

ui_error_t ui_wheel_picker_base_set_looping(struct ui_wheel_picker_base *picker,
                                            int is_looping) {
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  picker->is_looping = is_looping;
  return UI_ERROR_NONE;
}

ui_error_t
ui_wheel_picker_base_set_selected_index(struct ui_wheel_picker_base *picker,
                                        int index) {
  ui_error_t rc;

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
    rc = update_dom_state(picker);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (picker->on_change) {
      ui_error_t change_rc = picker->on_change(picker, picker->selected_index,
                                               picker->on_change_user_data);
      if (change_rc != UI_ERROR_NONE)
        return change_rc;
    }
  }
  return UI_ERROR_NONE;
}

/* \brief ui_wheel_picker_base_get_selected_index
 */
ui_error_t ui_wheel_picker_base_get_selected_index(
    const struct ui_wheel_picker_base *picker, int *out_index) {
  if (!picker || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_index = picker->selected_index;
  return UI_ERROR_NONE;
}

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

ui_error_t
ui_wheel_picker_base_process_event(struct ui_wheel_picker_base *picker,
                                   const struct ui_event *event,
                                   double timestamp_ms) {
  struct ui_gesture_event ge;
  ui_error_t rc;

  memset(&ge, 0, sizeof(ge));

  if (!picker || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (picker->is_disabled) {
    return UI_ERROR_NONE;
  }

  rc = trigger_cva_touched(picker);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Keyboard Support */
  if (event->type == UI_EVENT_KEY_DOWN) {
    if (event->event_data.keyboard.key_code == UI_KEY_UP) {
      return ui_wheel_picker_base_set_selected_index(
          picker, picker->selected_index - 1);
    } else if (event->event_data.keyboard.key_code == UI_KEY_DOWN) {
      return ui_wheel_picker_base_set_selected_index(
          picker, picker->selected_index + 1);
    }
  }

  rc = ui_gesture_recognizer_process_event(picker->gesture_recognizer, event,
                                           timestamp_ms, &ge);
  if (rc != UI_ERROR_NONE)
    return rc;

  if (ge.type == UI_GESTURE_PAN || ge.type == UI_GESTURE_SWIPE) {
    if (ge.state == UI_GESTURE_STATE_BEGAN) {
      picker->is_dragging = 1;
      picker->velocity = 0.0f;
    } else if (ge.state == UI_GESTURE_STATE_CHANGED) {
      picker->scroll_offset -= ge.delta_y; /* dragging up increases offset */
      picker->velocity = -ge.velocity_y;
    } else {
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
  ui_error_t rc;

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
        rc = ui_roundf_fallback(picker->scroll_offset /
                                    UI_WHEEL_PICKER_ITEM_HEIGHT,
                                &rounded_index);
        if (rc != UI_ERROR_NONE)
          return rc;
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
          rc = update_dom_state(picker);
          if (rc != UI_ERROR_NONE)
            return rc;
          if (picker->on_change) {
            ui_error_t change_rc = picker->on_change(
                picker, picker->selected_index, picker->on_change_user_data);
            if (change_rc != UI_ERROR_NONE)
              return change_rc;
          }
          rc = trigger_cva_change(picker);
          if (rc != UI_ERROR_NONE)
            return rc;
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

ui_error_t
ui_wheel_picker_base_get_component(struct ui_wheel_picker_base *picker,
                                   struct ui_component **out_component) {
  if (!picker || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = picker->component;
  return UI_ERROR_NONE;
}
