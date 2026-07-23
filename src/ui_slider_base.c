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

static const char *ui_slider_base_default_css =
    "input[type=\"range\"] { "
    "background: var(--slider-track-bg, #ddd); "
    "height: var(--slider-track-height, 4px); "
    "border-radius: var(--slider-thumb-radius, 2px); "
    "-webkit-appearance: none; "
    "cursor: pointer; "
    "} "
    "input[type=\"range\"]::-webkit-slider-thumb { "
    "-webkit-appearance: none; "
    "background: var(--slider-thumb-bg, #007bff); "
    "width: var(--slider-thumb-size, 16px); "
    "height: var(--slider-thumb-size, 16px); "
    "border-radius: var(--slider-thumb-radius, 50%); "
    "} "
    "input[type=\"range\"][aria-disabled=\"true\"] { "
    "opacity: var(--slider-disabled-opacity, 0.5); "
    "cursor: not-allowed; "
    "}";

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

  enum ui_error (*cva_on_change)(union ui_signal_payload new_value,
                                 void *user_data);
  void *cva_on_change_user_data;

  enum ui_error (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data;
};

static enum ui_error update_dom_state(struct ui_slider_base *slider);

static enum ui_error update_dom_state(struct ui_slider_base *slider) {
  if (slider && slider->component && slider->component->shadow_root) {
    char buf[64];

    /* In a real implementation we'd use robust string formatting macros.
       For this structural example, we rely on standard sprintf/snprintf. */
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%f", slider->value);
#else
    sprintf(buf, "%f", slider->value);
#endif
    ui_dom_node_set_attribute(slider->component->shadow_root, "aria-valuenow",
                              buf);
    ui_dom_node_set_attribute(slider->component->shadow_root, "value", buf);

#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%f", slider->min_val);
#else
    sprintf(buf, "%f", slider->min_val);
#endif
    ui_dom_node_set_attribute(slider->component->shadow_root, "aria-valuemin",
                              buf);
    ui_dom_node_set_attribute(slider->component->shadow_root, "min", buf);

#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%f", slider->max_val);
#else
    sprintf(buf, "%f", slider->max_val);
#endif
    ui_dom_node_set_attribute(slider->component->shadow_root, "aria-valuemax",
                              buf);
    ui_dom_node_set_attribute(slider->component->shadow_root, "max", buf);

    if (slider->disabled) {
      ui_dom_node_set_attribute(slider->component->shadow_root, "disabled", "");
      ui_dom_node_set_attribute(slider->component->shadow_root, "aria-disabled",
                                "true");
    } else {
      ui_dom_node_remove_attribute(slider->component->shadow_root, "disabled");
      ui_dom_node_remove_attribute(slider->component->shadow_root,
                                   "aria-disabled");
    }
  }
  return UI_ERROR_NONE;
}

static enum ui_error trigger_cva_change(struct ui_slider_base *slider) {
  if (slider && slider->cva_on_change) {
    union ui_signal_payload payload;
    payload.float_val = slider->value;
    return slider->cva_on_change(payload, slider->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

static enum ui_error trigger_cva_touched(struct ui_slider_base *slider) {
  if (slider && slider->cva_on_touched) {
    return slider->cva_on_touched(slider->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

static enum ui_error slider_cva_write_value(void *component,
                                            union ui_signal_payload value) {
  struct ui_slider_base *slider = (struct ui_slider_base *)component;

  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;

  ui_slider_base_set_value(slider, value.float_val);
  return UI_ERROR_NONE;
}

/** \brief slider_cva_register_on_change */
static enum ui_error slider_cva_register_on_change(
    void *component,
    enum ui_error (*callback)(union ui_signal_payload new_value,
                              void *user_data),
    void *user_data) {
  struct ui_slider_base *slider = (struct ui_slider_base *)component;
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->cva_on_change = callback;
  slider->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static enum ui_error
slider_cva_register_on_touched(void *component,
                               enum ui_error (*callback)(void *user_data),
                               void *user_data) {
  struct ui_slider_base *slider = (struct ui_slider_base *)component;
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->cva_on_touched = callback;
  slider->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static enum ui_error slider_cva_set_disabled_state(void *component,
                                                   int is_disabled) {
  struct ui_slider_base *slider = (struct ui_slider_base *)component;
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  ui_slider_base_set_disabled(slider, is_disabled);
  return UI_ERROR_NONE;
}

enum ui_error ui_slider_base_create(struct ui_slider_base **out_slider,
                                    struct ui_control_value_accessor *out_cva) {
  struct ui_slider_base *slider;
  enum ui_error rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_slider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  slider = (struct ui_slider_base *)UI_MALLOC(sizeof(struct ui_slider_base));
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

  ui_component_set_default_style(slider->component, default_style);

  slider->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component now */

  update_dom_state(slider);

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
    ui_dom_node_destroy(root_node);
  }
  if (slider->gesture_recognizer) {
    ui_gesture_recognizer_destroy(slider->gesture_recognizer);
  }
  if (slider->component) {
    ui_component_destroy(slider->component);
  }
  UI_FREE(slider);
  return rc;
}

void ui_slider_base_destroy(struct ui_slider_base *slider) {
  if (!slider)
    return;
  if (slider->gesture_recognizer)
    ui_gesture_recognizer_destroy(slider->gesture_recognizer);
  if (slider->component)
    ui_component_destroy(slider->component);
  UI_FREE(slider);
}

enum ui_error ui_slider_base_set_min(struct ui_slider_base *slider, float min) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->min_val = min;
  if (slider->max_val < slider->min_val)
    slider->max_val = slider->min_val;
  if (slider->value < slider->min_val)
    ui_slider_base_set_value(slider, slider->min_val);
  update_dom_state(slider);
  return UI_ERROR_NONE;
}

enum ui_error ui_slider_base_set_max(struct ui_slider_base *slider, float max) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->max_val = max;
  if (slider->min_val > slider->max_val)
    slider->min_val = slider->max_val;
  if (slider->value > slider->max_val)
    ui_slider_base_set_value(slider, slider->max_val);
  update_dom_state(slider);
  return UI_ERROR_NONE;
}

enum ui_error ui_slider_base_set_value(struct ui_slider_base *slider,
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
    update_dom_state(slider);
    if (slider->on_change) {
      slider->on_change(slider, slider->value, slider->user_data);
    }
    (void)trigger_cva_change(slider);
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_slider_base_get_value(const struct ui_slider_base *slider,
                                       float *out_value) {
  if (!slider || !out_value)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_value = slider->value;
  return UI_ERROR_NONE;
}

enum ui_error ui_slider_base_set_step(struct ui_slider_base *slider,
                                      float step) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  if (step < 0.0f)
    step = 0.0f;
  slider->step = step;
  return ui_slider_base_set_value(slider,
                                  slider->value); /* Re-snap if necessary */
}

enum ui_error ui_slider_base_set_disabled(struct ui_slider_base *slider,
                                          int disabled) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->disabled = disabled;
  update_dom_state(slider);
  return UI_ERROR_NONE;
}

enum ui_error ui_slider_base_set_on_change(struct ui_slider_base *slider,
                                           ui_slider_on_change_t on_change,
                                           void *user_data) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->on_change = on_change;
  slider->user_data = user_data;
  return UI_ERROR_NONE;
}

enum ui_error ui_slider_base_set_normalized_value(struct ui_slider_base *slider,
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

  (void)trigger_cva_touched(slider);
  return ui_slider_base_set_value(slider, new_value);
}

enum ui_error ui_slider_base_process_event(struct ui_slider_base *slider,
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

    if (ui_bidi_normalize_horizontal_key(key, &key) != UI_ERROR_NONE) {
      /* fallback */
    }

    if (key == UI_KEY_LEFT || key == UI_KEY_DOWN) {
      (void)trigger_cva_touched(slider);
      return ui_slider_base_set_value(slider, slider->value - increment);
    } else if (key == UI_KEY_RIGHT || key == UI_KEY_UP) {
      (void)trigger_cva_touched(slider);
      return ui_slider_base_set_value(slider, slider->value + increment);
    } else if (key == UI_KEY_HOME) {
      (void)trigger_cva_touched(slider);
      return ui_slider_base_set_value(slider, slider->min_val);
    } else if (key == UI_KEY_END) {
      (void)trigger_cva_touched(slider);
      return ui_slider_base_set_value(slider, slider->max_val);
    }
  }

  return UI_ERROR_NONE;
}
/** \brief ui_error */
enum ui_error
ui_slider_base_get_component(struct ui_slider_base *slider,
                             struct ui_component **out_component) {
  if (!slider || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = slider->component;
  return UI_ERROR_NONE;
}
