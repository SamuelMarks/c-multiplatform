/* clang-format off */
#include "ui_range_slider_base.h"
#include "ui_bidi_manager.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
#include <stdio.h>
#include <math.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char *ui_range_slider_base_default_css =
    ".ui-range-slider { "
    "position: relative; "
    "background: var(--slider-track-bg, #ddd); "
    "height: var(--slider-track-height, 4px); "
    "border-radius: var(--slider-thumb-radius, 2px); "
    "} "
    ".ui-range-slider[aria-disabled=\"true\"] { "
    "opacity: var(--slider-disabled-opacity, 0.5); "
    "cursor: not-allowed; "
    "} "
    ".ui-range-slider-thumb { "
    "position: absolute; "
    "top: 50%; "
    "transform: translateY(-50%); "
    "background: var(--slider-thumb-bg, #007bff); "
    "width: var(--slider-thumb-size, 16px); "
    "height: var(--slider-thumb-size, 16px); "
    "border-radius: var(--slider-thumb-radius, 50%); "
    "cursor: pointer; "
    "}";

/** \brief ui_range_slider_base */
struct ui_range_slider_base {
  struct ui_component *component;
  struct ui_gesture_recognizer *gesture_recognizer;
  struct ui_dom_node *thumb_low_node;
  struct ui_dom_node *thumb_high_node;
  float min_val;
  float max_val;
  float low_value;
  float high_value;
  float step;
  int disabled;
  ui_range_slider_on_change_t on_change;
  void *user_data;
};

static enum ui_error update_dom_state(struct ui_range_slider_base *slider) {
  char buf[64];
  float low_pct = 0.0f;
  float high_pct = 100.0f;
  float range = slider->max_val - slider->min_val;

  if (range > 0.0f) {
    low_pct = ((slider->low_value - slider->min_val) / range) * 100.0f;
    high_pct = ((slider->high_value - slider->min_val) / range) * 100.0f;
  }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", slider->low_value);
#else
  sprintf(buf, "%f", slider->low_value);
#endif
  ui_dom_node_set_attribute(slider->thumb_low_node, "aria-valuenow", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", slider->high_value);
#else
  sprintf(buf, "%f", slider->high_value);
#endif
  ui_dom_node_set_attribute(slider->thumb_high_node, "aria-valuenow", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", slider->min_val);
#else
  sprintf(buf, "%f", slider->min_val);
#endif
  ui_dom_node_set_attribute(slider->thumb_low_node, "aria-valuemin", buf);
  ui_dom_node_set_attribute(slider->thumb_high_node, "aria-valuemin", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", slider->max_val);
#else
  sprintf(buf, "%f", slider->max_val);
#endif
  ui_dom_node_set_attribute(slider->thumb_low_node, "aria-valuemax", buf);
  ui_dom_node_set_attribute(slider->thumb_high_node, "aria-valuemax", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "left: %f%%;", low_pct);
#else
  sprintf(buf, "left: %f%%;", low_pct);
#endif
  ui_dom_node_set_attribute(slider->thumb_low_node, "style", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "left: %f%%;", high_pct);
#else
  sprintf(buf, "left: %f%%;", high_pct);
#endif
  ui_dom_node_set_attribute(slider->thumb_high_node, "style", buf);

  if (slider->disabled) {
    ui_dom_node_set_attribute(slider->component->shadow_root, "aria-disabled",
                              "true");
  } else {
    ui_dom_node_remove_attribute(slider->component->shadow_root,
                                 "aria-disabled");
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_range_slider_base_create(struct ui_range_slider_base **out_slider) {
  struct ui_range_slider_base *slider;
  enum ui_error rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_slider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  slider = (struct ui_range_slider_base *)UI_MALLOC(
      sizeof(struct ui_range_slider_base));
  if (!slider) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  slider->component = NULL;
  slider->gesture_recognizer = NULL;
  slider->thumb_low_node = NULL;
  slider->thumb_high_node = NULL;
  slider->min_val = 0.0f;
  slider->max_val = 100.0f;
  slider->low_value = 0.0f;
  slider->high_value = 100.0f;
  slider->step = 0.0f;
  slider->disabled = 0;
  slider->on_change = NULL;
  slider->user_data = NULL;

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

  rc = ui_dom_node_set_tag_name(root_node, "div");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_set_attribute(root_node, "class", "ui-range-slider");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &slider->thumb_low_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_set_tag_name(slider->thumb_low_node, "div");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_set_attribute(slider->thumb_low_node, "class",
                                 "ui-range-slider-thumb");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_set_attribute(slider->thumb_low_node, "role", "slider");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_set_attribute(slider->thumb_low_node, "tabindex", "0");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &slider->thumb_high_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  (void)ui_dom_node_set_tag_name(slider->thumb_high_node, "div");
  (void)ui_dom_node_set_attribute(slider->thumb_high_node, "class",
                                  "ui-range-slider-thumb");
  (void)ui_dom_node_set_attribute(slider->thumb_high_node, "role", "slider");
  (void)ui_dom_node_set_attribute(slider->thumb_high_node, "tabindex", "0");
  (void)ui_dom_node_append_child(root_node, slider->thumb_low_node);
  (void)ui_dom_node_append_child(root_node, slider->thumb_high_node);
  (void)ui_css_parse_stylesheet(ui_range_slider_base_default_css,
                                &default_style);
  (void)ui_component_set_default_style(slider->component, default_style);

  slider->component->shadow_root = root_node;
  root_node = NULL;

  (void)update_dom_state(slider);

  *out_slider = slider;
  return UI_ERROR_NONE;

cleanup:
  if (slider->thumb_low_node)
    ui_dom_node_destroy(slider->thumb_low_node);
  if (slider->thumb_high_node)
    ui_dom_node_destroy(slider->thumb_high_node);
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

/** \brief ui_error */
enum ui_error
ui_range_slider_base_destroy(struct ui_range_slider_base *slider) {
  if (!slider)
    return UI_ERROR_NONE;
  ui_gesture_recognizer_destroy(slider->gesture_recognizer);
  ui_component_destroy(slider->component);
  UI_FREE(slider);
  return UI_ERROR_NONE;
}

enum ui_error ui_range_slider_base_set_min(struct ui_range_slider_base *slider,
                                           float min) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->min_val = min;
  if (slider->max_val < slider->min_val)
    slider->max_val = slider->min_val;
  if (slider->low_value < slider->min_val)
    ui_range_slider_base_set_values(slider, slider->min_val,
                                    slider->high_value);
  (void)update_dom_state(slider);
  return UI_ERROR_NONE;
}

enum ui_error ui_range_slider_base_set_max(struct ui_range_slider_base *slider,
                                           float max) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->max_val = max;
  if (slider->min_val > slider->max_val)
    slider->min_val = slider->max_val;
  if (slider->high_value > slider->max_val)
    ui_range_slider_base_set_values(slider, slider->low_value, slider->max_val);
  (void)update_dom_state(slider);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_range_slider_base_set_values(struct ui_range_slider_base *slider,
                                float low_value, float high_value) {
  float new_low, new_high;
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;

  new_low = low_value;
  new_high = high_value;

  if (new_low < slider->min_val)
    new_low = slider->min_val;
  if (new_low > slider->max_val)
    new_low = slider->max_val;

  if (new_high < slider->min_val)
    new_high = slider->min_val;
  if (new_high > slider->max_val)
    new_high = slider->max_val;

  if (new_low > new_high) {
    /* Push collision resolution */
    float temp = new_low;
    new_low = new_high;
    new_high = temp;
  }

  if (slider->step > 0.0f) {
    float low_steps = (new_low - slider->min_val) / slider->step;
    float high_steps = (new_high - slider->min_val) / slider->step;

    new_low = slider->min_val + (float)floor(low_steps + 0.5f) * slider->step;
    new_high = slider->min_val + (float)floor(high_steps + 0.5f) * slider->step;
  }

  if (slider->low_value != new_low || slider->high_value != new_high) {
    slider->low_value = new_low;
    slider->high_value = new_high;
    (void)update_dom_state(slider);
    if (slider->on_change) {
      slider->on_change(slider, slider->low_value, slider->high_value,
                        slider->user_data);
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_range_slider_base_get_values(const struct ui_range_slider_base *slider,
                                float *out_low, float *out_high) {
  if (!slider || !out_low || !out_high)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_low = slider->low_value;
  *out_high = slider->high_value;
  return UI_ERROR_NONE;
}

enum ui_error ui_range_slider_base_set_step(struct ui_range_slider_base *slider,
                                            float step) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  if (step < 0.0f)
    step = 0.0f;
  slider->step = step;
  return ui_range_slider_base_set_values(slider, slider->low_value,
                                         slider->high_value);
}

/** \brief ui_error */
enum ui_error
ui_range_slider_base_set_disabled(struct ui_range_slider_base *slider,
                                  int disabled) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->disabled = disabled;
  (void)update_dom_state(slider);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_range_slider_base_set_on_change(struct ui_range_slider_base *slider,
                                   ui_range_slider_on_change_t on_change,
                                   void *user_data) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->on_change = on_change;
  slider->user_data = user_data;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_range_slider_base_set_normalized_value(struct ui_range_slider_base *slider,
                                          enum ui_range_slider_thumb thumb,
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

  if (thumb == UI_RANGE_SLIDER_THUMB_LOW) {
    if (new_value > slider->high_value) {
      new_value =
          slider->high_value; /* Or swap, but pushing is safer for direct set */
    }
    return ui_range_slider_base_set_values(slider, new_value,
                                           slider->high_value);
  } else if (thumb == UI_RANGE_SLIDER_THUMB_HIGH) {
    if (new_value < slider->low_value) {
      new_value = slider->low_value;
    }
    return ui_range_slider_base_set_values(slider, slider->low_value,
                                           new_value);
  }

  return UI_ERROR_NONE;
}

/** \brief ui_range_slider_base_process_event */
enum ui_error ui_range_slider_base_process_event(
    struct ui_range_slider_base *slider, const struct ui_event *event,
    enum ui_range_slider_thumb active_thumb, double timestamp_ms) {
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

    if (active_thumb == UI_RANGE_SLIDER_THUMB_LOW) {
      if (key == UI_KEY_LEFT || key == UI_KEY_DOWN) {
        return ui_range_slider_base_set_values(
            slider, slider->low_value - increment, slider->high_value);
      } else if (key == UI_KEY_RIGHT || key == UI_KEY_UP) {
        return ui_range_slider_base_set_values(
            slider, slider->low_value + increment, slider->high_value);
      } else if (key == UI_KEY_HOME) {
        return ui_range_slider_base_set_values(slider, slider->min_val,
                                               slider->high_value);
      } else if (key == UI_KEY_END) {
        return ui_range_slider_base_set_values(slider, slider->high_value,
                                               slider->high_value);
      }
    } else if (active_thumb == UI_RANGE_SLIDER_THUMB_HIGH) {
      if (key == UI_KEY_LEFT || key == UI_KEY_DOWN) {
        return ui_range_slider_base_set_values(slider, slider->low_value,
                                               slider->high_value - increment);
      } else if (key == UI_KEY_RIGHT || key == UI_KEY_UP) {
        return ui_range_slider_base_set_values(slider, slider->low_value,
                                               slider->high_value + increment);
      } else if (key == UI_KEY_HOME) {
        return ui_range_slider_base_set_values(slider, slider->low_value,
                                               slider->low_value);
      } else if (key == UI_KEY_END) {
        return ui_range_slider_base_set_values(slider, slider->low_value,
                                               slider->max_val);
      }
    }
  }

  return UI_ERROR_NONE;
}
/** \brief ui_error */
enum ui_error
ui_range_slider_base_get_component(struct ui_range_slider_base *slider,
                                   struct ui_component **out_component) {
  if (!slider || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = slider->component;
  return UI_ERROR_NONE;
}
