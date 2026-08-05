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

static const char ui_range_slider_base_default_css[] = {
    46,  117, 105, 45,  114, 97,  110, 103, 101, 45,  115, 108, 105, 100, 101,
    114, 32,  123, 32,  112, 111, 115, 105, 116, 105, 111, 110, 58,  32,  114,
    101, 108, 97,  116, 105, 118, 101, 59,  32,  98,  97,  99,  107, 103, 114,
    111, 117, 110, 100, 58,  32,  118, 97,  114, 40,  45,  45,  115, 108, 105,
    100, 101, 114, 45,  116, 114, 97,  99,  107, 45,  98,  103, 44,  32,  35,
    100, 100, 100, 41,  59,  32,  104, 101, 105, 103, 104, 116, 58,  32,  118,
    97,  114, 40,  45,  45,  115, 108, 105, 100, 101, 114, 45,  116, 114, 97,
    99,  107, 45,  104, 101, 105, 103, 104, 116, 44,  32,  52,  112, 120, 41,
    59,  32,  98,  111, 114, 100, 101, 114, 45,  114, 97,  100, 105, 117, 115,
    58,  32,  118, 97,  114, 40,  45,  45,  115, 108, 105, 100, 101, 114, 45,
    116, 104, 117, 109, 98,  45,  114, 97,  100, 105, 117, 115, 44,  32,  50,
    112, 120, 41,  59,  32,  125, 32,  46,  117, 105, 45,  114, 97,  110, 103,
    101, 45,  115, 108, 105, 100, 101, 114, 91,  97,  114, 105, 97,  45,  100,
    105, 115, 97,  98,  108, 101, 100, 61,  34,  116, 114, 117, 101, 34,  93,
    32,  123, 32,  111, 112, 97,  99,  105, 116, 121, 58,  32,  118, 97,  114,
    40,  45,  45,  115, 108, 105, 100, 101, 114, 45,  100, 105, 115, 97,  98,
    108, 101, 100, 45,  111, 112, 97,  99,  105, 116, 121, 44,  32,  48,  46,
    53,  41,  59,  32,  99,  117, 114, 115, 111, 114, 58,  32,  110, 111, 116,
    45,  97,  108, 108, 111, 119, 101, 100, 59,  32,  125, 32,  46,  117, 105,
    45,  114, 97,  110, 103, 101, 45,  115, 108, 105, 100, 101, 114, 45,  116,
    104, 117, 109, 98,  32,  123, 32,  112, 111, 115, 105, 116, 105, 111, 110,
    58,  32,  97,  98,  115, 111, 108, 117, 116, 101, 59,  32,  116, 111, 112,
    58,  32,  53,  48,  37,  59,  32,  116, 114, 97,  110, 115, 102, 111, 114,
    109, 58,  32,  116, 114, 97,  110, 115, 108, 97,  116, 101, 89,  40,  45,
    53,  48,  37,  41,  59,  32,  98,  97,  99,  107, 103, 114, 111, 117, 110,
    100, 58,  32,  118, 97,  114, 40,  45,  45,  115, 108, 105, 100, 101, 114,
    45,  116, 104, 117, 109, 98,  45,  98,  103, 44,  32,  35,  48,  48,  55,
    98,  102, 102, 41,  59,  32,  119, 105, 100, 116, 104, 58,  32,  118, 97,
    114, 40,  45,  45,  115, 108, 105, 100, 101, 114, 45,  116, 104, 117, 109,
    98,  45,  115, 105, 122, 101, 44,  32,  49,  54,  112, 120, 41,  59,  32,
    104, 101, 105, 103, 104, 116, 58,  32,  118, 97,  114, 40,  45,  45,  115,
    108, 105, 100, 101, 114, 45,  116, 104, 117, 109, 98,  45,  115, 105, 122,
    101, 44,  32,  49,  54,  112, 120, 41,  59,  32,  98,  111, 114, 100, 101,
    114, 45,  114, 97,  100, 105, 117, 115, 58,  32,  118, 97,  114, 40,  45,
    45,  115, 108, 105, 100, 101, 114, 45,  116, 104, 117, 109, 98,  45,  114,
    97,  100, 105, 117, 115, 44,  32,  53,  48,  37,  41,  59,  32,  99,  117,
    114, 115, 111, 114, 58,  32,  112, 111, 105, 110, 116, 101, 114, 59,  32,
    125, 0};

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

static ui_error_t update_dom_state(struct ui_range_slider_base *slider) {
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
  {
    ui_error_t attr_rc =
        ui_dom_node_set_attribute(slider->thumb_low_node, "aria-valuenow", buf);
    if (attr_rc != UI_ERROR_NONE)
      return attr_rc;
  }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", slider->high_value);
#else
  sprintf(buf, "%f", slider->high_value);
#endif
  {
    ui_error_t attr_rc = ui_dom_node_set_attribute(slider->thumb_high_node,
                                                   "aria-valuenow", buf);
    if (attr_rc != UI_ERROR_NONE)
      return attr_rc;
  }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", slider->min_val);
#else
  sprintf(buf, "%f", slider->min_val);
#endif
  {
    ui_error_t attr_rc =
        ui_dom_node_set_attribute(slider->thumb_low_node, "aria-valuemin", buf);
    if (attr_rc != UI_ERROR_NONE)
      return attr_rc;
  }
  {
    ui_error_t attr_rc = ui_dom_node_set_attribute(slider->thumb_high_node,
                                                   "aria-valuemin", buf);
    if (attr_rc != UI_ERROR_NONE)
      return attr_rc;
  }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", slider->max_val);
#else
  sprintf(buf, "%f", slider->max_val);
#endif
  {
    ui_error_t attr_rc =
        ui_dom_node_set_attribute(slider->thumb_low_node, "aria-valuemax", buf);
    if (attr_rc != UI_ERROR_NONE)
      return attr_rc;
  }
  {
    ui_error_t attr_rc = ui_dom_node_set_attribute(slider->thumb_high_node,
                                                   "aria-valuemax", buf);
    if (attr_rc != UI_ERROR_NONE)
      return attr_rc;
  }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "left: %f%%;", low_pct);
#else
  sprintf(buf, "left: %f%%;", low_pct);
#endif
  {
    ui_error_t attr_rc =
        ui_dom_node_set_attribute(slider->thumb_low_node, "style", buf);
    if (attr_rc != UI_ERROR_NONE)
      return attr_rc;
  }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "left: %f%%;", high_pct);
#else
  sprintf(buf, "left: %f%%;", high_pct);
#endif
  {
    ui_error_t attr_rc =
        ui_dom_node_set_attribute(slider->thumb_high_node, "style", buf);
    if (attr_rc != UI_ERROR_NONE)
      return attr_rc;
  }

  if (slider->disabled) {
    {
      ui_error_t set_rc = ui_dom_node_set_attribute(
          slider->component->shadow_root, "aria-disabled", "true");
      if (set_rc != UI_ERROR_NONE) {
        if (0)
          return set_rc;
      }
    }
  } else {
    {
      ui_error_t rem_rc = ui_dom_node_remove_attribute(
          slider->component->shadow_root, "aria-disabled");
      if (rem_rc != UI_ERROR_NONE && rem_rc != UI_ERROR_NOT_FOUND) {
        if (0)
          return rem_rc;
      }
    }
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_range_slider_base_create(struct ui_range_slider_base **out_slider) {
  struct ui_range_slider_base *slider;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_slider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  slider = (struct ui_range_slider_base *)C_MULTIPLATFORM_MALLOC(
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

  {
    rc = ui_dom_node_set_tag_name(slider->thumb_high_node, "div");
    if (rc != UI_ERROR_NONE)
      goto cleanup;
  }
  {
    ui_error_t _ign_rc = ui_dom_node_set_attribute(
        slider->thumb_high_node, "class", "ui-range-slider-thumb");
    (void)_ign_rc;
  }
  {
    rc = ui_dom_node_set_attribute(slider->thumb_high_node, "role", "slider");
    if (rc != UI_ERROR_NONE)
      goto cleanup;
  }
  {
    rc = ui_dom_node_set_attribute(slider->thumb_high_node, "tabindex", "0");
    if (rc != UI_ERROR_NONE)
      goto cleanup;
  }
  {
    rc = ui_dom_node_append_child(root_node, slider->thumb_low_node);
    if (rc != UI_ERROR_NONE)
      goto cleanup;
  }
  {
    rc = ui_dom_node_append_child(root_node, slider->thumb_high_node);
    if (rc != UI_ERROR_NONE)
      goto cleanup;
  }
  {
    ui_error_t _ign_rc = ui_css_parse_stylesheet(
        ui_range_slider_base_default_css, &default_style);
    (void)_ign_rc;
  }
  {
    rc = ui_component_set_default_style(slider->component, default_style);
    if (rc != UI_ERROR_NONE) {
      ui_css_stylesheet_destroy(default_style);
      goto cleanup;
    }
  }

  slider->component->shadow_root = root_node;
  root_node = NULL;

  {
    ui_error_t update_rc = update_dom_state(slider);
    if (update_rc != UI_ERROR_NONE)
      return update_rc;
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

/** \brief ui_error */
ui_error_t ui_range_slider_base_destroy(struct ui_range_slider_base *slider) {
  if (!slider)
    return UI_ERROR_NONE;
  (void)ui_gesture_recognizer_destroy(slider->gesture_recognizer);
  (void)ui_component_destroy(slider->component);
  C_MULTIPLATFORM_FREE(slider);
  return UI_ERROR_NONE;
}

ui_error_t ui_range_slider_base_set_min(struct ui_range_slider_base *slider,
                                        float min) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->min_val = min;
  if (slider->max_val < slider->min_val)
    slider->max_val = slider->min_val;
  if (slider->low_value < slider->min_val) {
    ui_error_t set_rc = ui_range_slider_base_set_values(slider, slider->min_val,
                                                        slider->high_value);
    if (set_rc != UI_ERROR_NONE)
      return set_rc;
  }
  {
    ui_error_t update_rc = update_dom_state(slider);
    if (update_rc != UI_ERROR_NONE)
      return update_rc;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_range_slider_base_set_max(struct ui_range_slider_base *slider,
                                        float max) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->max_val = max;
  if (slider->min_val > slider->max_val)
    slider->min_val = slider->max_val;
  if (slider->high_value > slider->max_val) {
    ui_error_t set_rc = ui_range_slider_base_set_values(
        slider, slider->low_value, slider->max_val);
    if (set_rc != UI_ERROR_NONE)
      return set_rc;
  }
  {
    ui_error_t update_rc = update_dom_state(slider);
    if (update_rc != UI_ERROR_NONE)
      return update_rc;
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_range_slider_base_set_values(struct ui_range_slider_base *slider,
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
    {
      ui_error_t update_rc = update_dom_state(slider);
      if (update_rc != UI_ERROR_NONE)
        return update_rc;
    }
    if (slider->on_change) {
      {
        ui_error_t cb_rc = slider->on_change(
            slider, slider->low_value, slider->high_value, slider->user_data);
        if (cb_rc != UI_ERROR_NONE) {
          if (0)
            return cb_rc;
        }
      }
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_range_slider_base_get_values(const struct ui_range_slider_base *slider,
                                float *out_low, float *out_high) {
  if (!slider || !out_low || !out_high)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_low = slider->low_value;
  *out_high = slider->high_value;
  return UI_ERROR_NONE;
}

ui_error_t ui_range_slider_base_set_step(struct ui_range_slider_base *slider,
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
ui_error_t
ui_range_slider_base_set_disabled(struct ui_range_slider_base *slider,
                                  int disabled) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->disabled = disabled;
  {
    ui_error_t update_rc = update_dom_state(slider);
    if (update_rc != UI_ERROR_NONE)
      return update_rc;
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
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
ui_error_t
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
ui_error_t ui_range_slider_base_process_event(
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
    {
      ui_error_t bidi_rc = ui_bidi_normalize_horizontal_key(key, &key);
      if (bidi_rc != UI_ERROR_NONE)
        return bidi_rc;
    }
    if (0) {
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
ui_error_t
ui_range_slider_base_get_component(struct ui_range_slider_base *slider,
                                   struct ui_component **out_component) {
  if (!slider || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = slider->component;
  return UI_ERROR_NONE;
}
