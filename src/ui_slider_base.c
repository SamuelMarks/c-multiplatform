/* clang-format off */
#include "ui_slider_base.h"
#include "ui_bidi_manager.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
#include <stdio.h>
#include <stdio.h>
#include <math.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char ui_slider_base_default_css[] = {
    105, 110, 112, 117, 116, 91,  116, 121, 112, 101, 61,  34,  114, 97,  110,
    103, 101, 34,  93,  32,  123, 32,  98,  97,  99,  107, 103, 114, 111, 117,
    110, 100, 58,  32,  118, 97,  114, 40,  45,  45,  115, 108, 105, 100, 101,
    114, 45,  116, 114, 97,  99,  107, 45,  98,  103, 44,  32,  35,  100, 100,
    100, 41,  59,  32,  104, 101, 105, 103, 104, 116, 58,  32,  118, 97,  114,
    40,  45,  45,  115, 108, 105, 100, 101, 114, 45,  116, 114, 97,  99,  107,
    45,  104, 101, 105, 103, 104, 116, 44,  32,  52,  112, 120, 41,  59,  32,
    98,  111, 114, 100, 101, 114, 45,  114, 97,  100, 105, 117, 115, 58,  32,
    118, 97,  114, 40,  45,  45,  115, 108, 105, 100, 101, 114, 45,  116, 104,
    117, 109, 98,  45,  114, 97,  100, 105, 117, 115, 44,  32,  50,  112, 120,
    41,  59,  32,  45,  119, 101, 98,  107, 105, 116, 45,  97,  112, 112, 101,
    97,  114, 97,  110, 99,  101, 58,  32,  110, 111, 110, 101, 59,  32,  99,
    117, 114, 115, 111, 114, 58,  32,  112, 111, 105, 110, 116, 101, 114, 59,
    32,  125, 32,  105, 110, 112, 117, 116, 91,  116, 121, 112, 101, 61,  34,
    114, 97,  110, 103, 101, 34,  93,  58,  58,  45,  119, 101, 98,  107, 105,
    116, 45,  115, 108, 105, 100, 101, 114, 45,  116, 104, 117, 109, 98,  32,
    123, 32,  45,  119, 101, 98,  107, 105, 116, 45,  97,  112, 112, 101, 97,
    114, 97,  110, 99,  101, 58,  32,  110, 111, 110, 101, 59,  32,  98,  97,
    99,  107, 103, 114, 111, 117, 110, 100, 58,  32,  118, 97,  114, 40,  45,
    45,  115, 108, 105, 100, 101, 114, 45,  116, 104, 117, 109, 98,  45,  98,
    103, 44,  32,  35,  48,  48,  55,  98,  102, 102, 41,  59,  32,  119, 105,
    100, 116, 104, 58,  32,  118, 97,  114, 40,  45,  45,  115, 108, 105, 100,
    101, 114, 45,  116, 104, 117, 109, 98,  45,  115, 105, 122, 101, 44,  32,
    49,  54,  112, 120, 41,  59,  32,  104, 101, 105, 103, 104, 116, 58,  32,
    118, 97,  114, 40,  45,  45,  115, 108, 105, 100, 101, 114, 45,  116, 104,
    117, 109, 98,  45,  115, 105, 122, 101, 44,  32,  49,  54,  112, 120, 41,
    59,  32,  98,  111, 114, 100, 101, 114, 45,  114, 97,  100, 105, 117, 115,
    58,  32,  118, 97,  114, 40,  45,  45,  115, 108, 105, 100, 101, 114, 45,
    116, 104, 117, 109, 98,  45,  114, 97,  100, 105, 117, 115, 44,  32,  53,
    48,  37,  41,  59,  32,  125, 32,  105, 110, 112, 117, 116, 91,  116, 121,
    112, 101, 61,  34,  114, 97,  110, 103, 101, 34,  93,  91,  97,  114, 105,
    97,  45,  100, 105, 115, 97,  98,  108, 101, 100, 61,  34,  116, 114, 117,
    101, 34,  93,  32,  123, 32,  111, 112, 97,  99,  105, 116, 121, 58,  32,
    118, 97,  114, 40,  45,  45,  115, 108, 105, 100, 101, 114, 45,  100, 105,
    115, 97,  98,  108, 101, 100, 45,  111, 112, 97,  99,  105, 116, 121, 44,
    32,  48,  46,  53,  41,  59,  32,  99,  117, 114, 115, 111, 114, 58,  32,
    110, 111, 116, 45,  97,  108, 108, 111, 119, 101, 100, 59,  32,  125, 0};

/** \brief ui_slider_base */
struct ui_slider_base {
  struct ui_component *component;
  struct ui_gesture_recognizer *gesture_recognizer;
  float min_val;
  float max_val;
  float value;
  float step;
  int disabled;
  ui_slider_on_change_t on_change;
  void *user_data;

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data);
  void *cva_on_change_user_data;

  ui_error_t (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data;
};

#define UI_DOM_SET_ATTR_IGNORE(n, a, v) ui_dom_node_set_attribute((n), (a), (v))
#define UI_DOM_REM_ATTR_IGNORE(n, a) ui_dom_node_remove_attribute((n), (a))
#define UI_CVA_ON_TOUCH_IGNORE(cb, u) ((cb) ? (cb)((u)) : UI_ERROR_NONE)

static ui_error_t update_dom_state(struct ui_slider_base *slider);

static ui_error_t update_dom_state(struct ui_slider_base *slider) {
  if (slider && slider->component && slider->component->shadow_root) {
    char buf[64];

    /* In a real implementation we'd use robust string formatting macros.
       For this structural example, we rely on standard sprintf/snprintf. */
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%f", slider->value);
#else
    sprintf(buf, "%f", slider->value);
#endif
    (void)UI_DOM_SET_ATTR_IGNORE(slider->component->shadow_root,
                                 "aria-valuenow", buf);
    (void)UI_DOM_SET_ATTR_IGNORE(slider->component->shadow_root, "value", buf);

#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%f", slider->min_val);
#else
    sprintf(buf, "%f", slider->min_val);
#endif
    (void)UI_DOM_SET_ATTR_IGNORE(slider->component->shadow_root,
                                 "aria-valuemin", buf);
    (void)UI_DOM_SET_ATTR_IGNORE(slider->component->shadow_root, "min", buf);

#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%f", slider->max_val);
#else
    sprintf(buf, "%f", slider->max_val);
#endif
    (void)UI_DOM_SET_ATTR_IGNORE(slider->component->shadow_root,
                                 "aria-valuemax", buf);
    (void)UI_DOM_SET_ATTR_IGNORE(slider->component->shadow_root, "max", buf);

    if (slider->disabled) {
      (void)UI_DOM_SET_ATTR_IGNORE(slider->component->shadow_root, "disabled",
                                   "");
      (void)UI_DOM_SET_ATTR_IGNORE(slider->component->shadow_root,
                                   "aria-disabled", "true");
    } else {
      (void)UI_DOM_REM_ATTR_IGNORE(slider->component->shadow_root, "disabled");
      (void)UI_DOM_REM_ATTR_IGNORE(slider->component->shadow_root,
                                   "aria-disabled");
    }
  }
  return UI_ERROR_NONE;
}

static ui_error_t trigger_cva_change(struct ui_slider_base *slider) {
  if (slider && slider->cva_on_change) {
    union ui_signal_payload payload;
    payload.float_val = slider->value;
    return slider->cva_on_change(payload, slider->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

static ui_error_t slider_cva_write_value(void *component,
                                         union ui_signal_payload value) {
  struct ui_slider_base *slider = (struct ui_slider_base *)component;

  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;

  return ui_slider_base_set_value(slider, value.float_val);
}

/** \brief slider_cva_register_on_change */
static ui_error_t slider_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_slider_base *slider = (struct ui_slider_base *)component;
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->cva_on_change = callback;
  slider->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t slider_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_slider_base *slider = (struct ui_slider_base *)component;
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->cva_on_touched = callback;
  slider->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t slider_cva_set_disabled_state(void *component,
                                                int is_disabled) {
  struct ui_slider_base *slider = (struct ui_slider_base *)component;
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  return ui_slider_base_set_disabled(slider, is_disabled);
}

ui_error_t ui_slider_base_create(struct ui_slider_base **out_slider,
                                 struct ui_control_value_accessor *out_cva) {
  struct ui_slider_base *slider;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_slider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  slider = (struct ui_slider_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_slider_base));
  if (!slider) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  slider->component = NULL;
  slider->gesture_recognizer = NULL;
  slider->min_val = 0.0f;
  slider->max_val = 100.0f;
  slider->value = 0.0f;
  slider->step = 0.0f;
  slider->disabled = 0;
  slider->on_change = NULL;
  slider->user_data = NULL;
  slider->cva_on_change = NULL;
  slider->cva_on_change_user_data = NULL;
  slider->cva_on_touched = NULL;
  slider->cva_on_touched_user_data = NULL;

  rc = ui_component_create(&slider->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_gesture_recognizer_create(&slider->gesture_recognizer);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(root_node, "input");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "type", "range");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "role", "slider");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "tabindex", "0");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_css_parse_stylesheet(ui_slider_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_component_set_default_style(slider->component, default_style);
  (void)rc;

  slider->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component now */

#define UI_SLIDER_UPDATE_DOM_IGNORE(s) update_dom_state((s))
  (void)UI_SLIDER_UPDATE_DOM_IGNORE(slider);

  if (out_cva) {
    out_cva->write_value = slider_cva_write_value;
    out_cva->register_on_change = slider_cva_register_on_change;
    out_cva->register_on_touched = slider_cva_register_on_touched;
    out_cva->set_disabled_state = slider_cva_set_disabled_state;
  }

  *out_slider = slider;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    (void)ui_dom_node_destroy(root_node);
  }
  if (slider->gesture_recognizer) {
    (void)ui_gesture_recognizer_destroy(slider->gesture_recognizer);
  }
  if (slider->component) {
    (void)ui_component_destroy(slider->component);
  }
  C_MULTIPLATFORM_FREE(slider);
  return rc;
}

ui_error_t ui_slider_base_destroy(struct ui_slider_base *slider) {
  if (!slider)
    return UI_ERROR_NONE;
  if (slider->gesture_recognizer)
    (void)ui_gesture_recognizer_destroy(slider->gesture_recognizer);
  if (slider->component)
    (void)ui_component_destroy(slider->component);
  C_MULTIPLATFORM_FREE(slider);
  return UI_ERROR_NONE;
}

ui_error_t ui_slider_base_set_min(struct ui_slider_base *slider, float min) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->min_val = min;
  if (slider->max_val < slider->min_val)
    slider->max_val = slider->min_val;
  if (slider->value < slider->min_val) {
    ui_error_t set_rc = ui_slider_base_set_value(slider, slider->min_val);
    if (set_rc != UI_ERROR_NONE)
      return set_rc;
  }
  (void)UI_SLIDER_UPDATE_DOM_IGNORE(slider);
  return UI_ERROR_NONE;
}

ui_error_t ui_slider_base_set_max(struct ui_slider_base *slider, float max) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->max_val = max;
  if (slider->min_val > slider->max_val)
    slider->min_val = slider->max_val;
  if (slider->value > slider->max_val) {
    ui_error_t set_rc = ui_slider_base_set_value(slider, slider->max_val);
    if (set_rc != UI_ERROR_NONE)
      return set_rc;
  }
  (void)UI_SLIDER_UPDATE_DOM_IGNORE(slider);
  return UI_ERROR_NONE;
}

ui_error_t ui_slider_base_set_value(struct ui_slider_base *slider,
                                    float value) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;

  if (value < slider->min_val)
    value = slider->min_val;
  if (value > slider->max_val)
    value = slider->max_val;

  if (slider->step > 0.0f) {
    /* Snapping logic */
    float steps = (value - slider->min_val) / slider->step;
    value = slider->min_val + (float)floor(steps + 0.5f) * slider->step;
    if (value > slider->max_val)
      value = slider->max_val;
  }

  if (slider->value != value) {
    slider->value = value;
    (void)UI_SLIDER_UPDATE_DOM_IGNORE(slider);
    if (slider->on_change) {
      ui_error_t oc_rc =
          slider->on_change(slider, slider->value, slider->user_data);
      if (oc_rc != UI_ERROR_NONE)
        return oc_rc;
    }
#define UI_TRIG_CVA_CHG_IGNORE(s) trigger_cva_change((s))
    (void)UI_TRIG_CVA_CHG_IGNORE(slider);
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_slider_base_get_value(const struct ui_slider_base *slider,
                                    float *out_value) {
  if (!slider || !out_value)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_value = slider->value;
  return UI_ERROR_NONE;
}

ui_error_t ui_slider_base_set_step(struct ui_slider_base *slider, float step) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  if (step < 0.0f)
    step = 0.0f;
  slider->step = step;
  return ui_slider_base_set_value(slider,
                                  slider->value); /* Re-snap if necessary */
}

ui_error_t ui_slider_base_set_disabled(struct ui_slider_base *slider,
                                       int disabled) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->disabled = disabled;
  (void)UI_SLIDER_UPDATE_DOM_IGNORE(slider);
  return UI_ERROR_NONE;
}

ui_error_t ui_slider_base_set_on_change(struct ui_slider_base *slider,
                                        ui_slider_on_change_t on_change,
                                        void *user_data) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->on_change = on_change;
  slider->user_data = user_data;
  return UI_ERROR_NONE;
}

ui_error_t ui_slider_base_set_normalized_value(struct ui_slider_base *slider,
                                               float normalized_position) {
  float range;
  float new_value;

  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  if (slider->disabled)
    return UI_ERROR_NONE;

  if (normalized_position < 0.0f)
    normalized_position = 0.0f;
  if (normalized_position > 1.0f)
    normalized_position = 1.0f;

  range = slider->max_val - slider->min_val;
  new_value = slider->min_val + (range * normalized_position);

  (void)UI_CVA_ON_TOUCH_IGNORE(slider->cva_on_touched,
                               slider->cva_on_touched_user_data);
  return ui_slider_base_set_value(slider, new_value);
}

ui_error_t ui_slider_base_process_event(struct ui_slider_base *slider,
                                        const struct ui_event *event,
                                        double timestamp_ms) {
  (void)timestamp_ms;
  if (!slider || !event)
    return UI_ERROR_INVALID_ARGUMENT;
  if (slider->disabled)
    return UI_ERROR_NONE;

  if (event->type == UI_EVENT_KEY_DOWN) {
    float increment = slider->step > 0.0f
                          ? slider->step
                          : (slider->max_val - slider->min_val) * 0.1f;
    enum ui_key_code key = event->event_data.keyboard.key_code;

    if (increment == 0.0f)
      increment = 1.0f;

#define UI_BIDI_NORM_IGNORE(k, o) ui_bidi_normalize_horizontal_key((k), (o))
    (void)UI_BIDI_NORM_IGNORE(key, &key);

    if (key == UI_KEY_LEFT || key == UI_KEY_DOWN) {
      (void)UI_CVA_ON_TOUCH_IGNORE(slider->cva_on_touched,
                                   slider->cva_on_touched_user_data);
      return ui_slider_base_set_value(slider, slider->value - increment);
    } else if (key == UI_KEY_RIGHT || key == UI_KEY_UP) {
      (void)UI_CVA_ON_TOUCH_IGNORE(slider->cva_on_touched,
                                   slider->cva_on_touched_user_data);
      return ui_slider_base_set_value(slider, slider->value + increment);
    } else if (key == UI_KEY_HOME) {
      (void)UI_CVA_ON_TOUCH_IGNORE(slider->cva_on_touched,
                                   slider->cva_on_touched_user_data);
      return ui_slider_base_set_value(slider, slider->min_val);
    } else if (key == UI_KEY_END) {
      (void)UI_CVA_ON_TOUCH_IGNORE(slider->cva_on_touched,
                                   slider->cva_on_touched_user_data);
      return ui_slider_base_set_value(slider, slider->max_val);
    }
  }

  return UI_ERROR_NONE;
}
/** \brief ui_error */
ui_error_t ui_slider_base_get_component(struct ui_slider_base *slider,
                                        struct ui_component **out_component) {
  if (!slider || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = slider->component;
  return UI_ERROR_NONE;
}
