/**
 * @file ui_range_slider_base.c
 * @brief ui_range_slider_base.c implementation.
 */
/*
 * \file ui_range_slider_base.c
 * \brief Implementation of the UI Range Slider Base component.
 */

/* clang-format off */
#include "ui_range_slider_base.h"
#include "ui_bidi_manager.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
#include <stdio.h>
#include <math.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_range_slider_mock_set_attribute_fail = 0;
int g_range_slider_mock_remove_attribute_fail = 0;
int g_range_slider_mock_append_child_fail = 0;
int g_range_slider_mock_parse_css_fail = 0;
int g_range_slider_mock_set_style_fail = 0;
int g_range_slider_mock_gesture_destroy_fail = 0;
int g_range_slider_mock_comp_destroy_fail = 0;
int g_range_slider_mock_bidi_fail = 0;

/**
 * @brief mock_range_slider_set_attribute.
 * @param node Node pointer.
 * @param name Attribute name.
 * @param value Attribute value.
 * @return Return value.
 */
static ui_error_t mock_range_slider_set_attribute(struct ui_dom_node *node,
                                                  const char *name,
                                                  const char *value) {
  if (g_range_slider_mock_set_attribute_fail > 0) {
    g_range_slider_mock_set_attribute_fail--;
    if (g_range_slider_mock_set_attribute_fail == 0) {
      return UI_ERROR_UNKNOWN;
    }
  }
  return (ui_dom_node_set_attribute)(node, name, value);
}
#undef ui_dom_node_set_attribute
/** @cond */
#define ui_dom_node_set_attribute mock_range_slider_set_attribute
/** @endcond */

/**
 * @brief mock_range_slider_remove_attribute.
 * @param node Node pointer.
 * @param name Attribute name.
 * @return Return value.
 */
static ui_error_t mock_range_slider_remove_attribute(struct ui_dom_node *node,
                                                     const char *name) {
  if (g_range_slider_mock_remove_attribute_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_remove_attribute)(node, name);
}
#undef ui_dom_node_remove_attribute
/** @cond */
#define ui_dom_node_remove_attribute mock_range_slider_remove_attribute
/** @endcond */

/**
 * @brief mock_range_slider_append_child.
 * @param parent Parent node.
 * @param child Child node.
 * @return Return value.
 */
static ui_error_t mock_range_slider_append_child(struct ui_dom_node *parent,
                                                 struct ui_dom_node *child) {
  if (g_range_slider_mock_append_child_fail > 0) {
    g_range_slider_mock_append_child_fail--;
    if (g_range_slider_mock_append_child_fail == 0) {
      return UI_ERROR_UNKNOWN;
    }
  }
  return (ui_dom_node_append_child)(parent, child);
}
#undef ui_dom_node_append_child
/** @cond */
#define ui_dom_node_append_child mock_range_slider_append_child
/** @endcond */

/**
 * @brief mock_range_slider_parse_css.
 * @param css CSS string.
 * @param out_sheet Output stylesheet.
 * @return Return value.
 */
static ui_error_t
mock_range_slider_parse_css(const char *css,
                            struct ui_css_stylesheet **out_sheet) {
  if (g_range_slider_mock_parse_css_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_css_parse_stylesheet)(css, out_sheet);
}
#undef ui_css_parse_stylesheet
/** @cond */
#define ui_css_parse_stylesheet mock_range_slider_parse_css
/** @endcond */

/**
 * @brief mock_range_slider_set_style.
 * @param comp Component.
 * @param sheet Stylesheet.
 * @return Return value.
 */
static ui_error_t mock_range_slider_set_style(struct ui_component *comp,
                                              struct ui_css_stylesheet *sheet) {
  if (g_range_slider_mock_set_style_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_set_default_style)(comp, sheet);
}
#undef ui_component_set_default_style
/** @cond */
#define ui_component_set_default_style mock_range_slider_set_style
/** @endcond */

/**
 * @brief mock_range_slider_gesture_destroy.
 * @param recognizer Recognizer.
 * @return Return value.
 */
static ui_error_t
mock_range_slider_gesture_destroy(struct ui_gesture_recognizer *recognizer) {
  if (g_range_slider_mock_gesture_destroy_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_gesture_recognizer_destroy)(recognizer);
}
#undef ui_gesture_recognizer_destroy
/** @cond */
#define ui_gesture_recognizer_destroy mock_range_slider_gesture_destroy
/** @endcond */

/**
 * @brief mock_range_slider_comp_destroy.
 * @param comp Component.
 * @return Return value.
 */
static ui_error_t mock_range_slider_comp_destroy(struct ui_component *comp) {
  if (g_range_slider_mock_comp_destroy_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_destroy)(comp);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_range_slider_comp_destroy
/** @endcond */

int g_range_slider_mock_dom_destroy_fail = 0;

/**
 * @brief mock_range_slider_dom_destroy.
 * @param node Node.
 * @return Return value.
 */
static ui_error_t mock_range_slider_dom_destroy(struct ui_dom_node *node) {
  if (g_range_slider_mock_dom_destroy_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_destroy)(node);
}
#undef ui_dom_node_destroy
/** @cond */
#define ui_dom_node_destroy mock_range_slider_dom_destroy
/** @endcond */

/**
 * @brief mock_range_slider_bidi.
 * @param key Key.
 * @param out_key Out key.
 * @return Return value.
 */
static ui_error_t mock_range_slider_bidi(enum ui_key_code key,
                                         enum ui_key_code *out_key) {
  if (g_range_slider_mock_bidi_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_bidi_normalize_horizontal_key)(key, out_key);
}
#undef ui_bidi_normalize_horizontal_key
/** @cond */
#define ui_bidi_normalize_horizontal_key mock_range_slider_bidi
/** @endcond */
#endif

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

/* \brief Default CSS stylesheet for the range slider */
/** @brief Default CSS stylesheet */
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

/**
 * @struct ui_range_slider_base
 * \brief Internal structure representing a range slider.
 */
struct ui_range_slider_base {
  struct ui_component *component; /**< Underlying DOM container */
  struct ui_gesture_recognizer *gesture_recognizer; /**< Gesture recognizer */
  struct ui_dom_node *thumb_low_node;    /**< Node for the low thumb */
  struct ui_dom_node *thumb_high_node;   /**< Node for the high thumb */
  float min_val;                         /**< Minimum possible value */
  float max_val;                         /**< Maximum possible value */
  float low_value;                       /**< Current low value */
  float high_value;                      /**< Current high value */
  float step;                            /**< Step increment */
  int disabled;                          /**< Non-zero if disabled */
  ui_range_slider_on_change_t on_change; /**< Change callback */
  void *user_data;                       /**< Callback user data */
};

/**
 * \brief Updates DOM attributes and styles for the slider.
 *
 * \param slider The component.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief update_dom_state.
 * @param slider Parameter slider.
 * @return Return value.
 */
static ui_error_t update_dom_state(struct ui_range_slider_base *slider) {
  char buf[64];
  float low_pct = 0.0f;
  float high_pct = 100.0f;
  float range = slider->max_val - slider->min_val;
  ui_error_t rc;

  if (range > 0.0f) {
    low_pct = ((slider->low_value - slider->min_val) / range) * 100.0f;
    high_pct = ((slider->high_value - slider->min_val) / range) * 100.0f;
  }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", slider->low_value);
#else
  sprintf(buf, "%f", slider->low_value);
#endif
  rc = ui_dom_node_set_attribute(slider->thumb_low_node, "aria-valuenow", buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", slider->high_value);
#else
  sprintf(buf, "%f", slider->high_value);
#endif
  rc = ui_dom_node_set_attribute(slider->thumb_high_node, "aria-valuenow", buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", slider->min_val);
#else
  sprintf(buf, "%f", slider->min_val);
#endif
  rc = ui_dom_node_set_attribute(slider->thumb_low_node, "aria-valuemin", buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  rc = ui_dom_node_set_attribute(slider->thumb_high_node, "aria-valuemin", buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", slider->max_val);
#else
  sprintf(buf, "%f", slider->max_val);
#endif
  rc = ui_dom_node_set_attribute(slider->thumb_low_node, "aria-valuemax", buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  rc = ui_dom_node_set_attribute(slider->thumb_high_node, "aria-valuemax", buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "left: %f%%;", low_pct);
#else
  sprintf(buf, "left: %f%%;", low_pct);
#endif
  rc = ui_dom_node_set_attribute(slider->thumb_low_node, "style", buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "left: %f%%;", high_pct);
#else
  sprintf(buf, "left: %f%%;", high_pct);
#endif
  rc = ui_dom_node_set_attribute(slider->thumb_high_node, "style", buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (slider->disabled) {
    rc = ui_dom_node_set_attribute(slider->component->shadow_root,
                                   "aria-disabled", "true");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    rc = ui_dom_node_remove_attribute(slider->component->shadow_root,
                                      "aria-disabled");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Creates a new unstyled range slider base component.
 *
 * \param out_slider Pointer to receive the allocated range slider base.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
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
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "class", "ui-range-slider");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &slider->thumb_low_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_set_tag_name(slider->thumb_low_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(slider->thumb_low_node, "class",
                                 "ui-range-slider-thumb");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(slider->thumb_low_node, "role", "slider");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(slider->thumb_low_node, "tabindex", "0");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &slider->thumb_high_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_set_tag_name(slider->thumb_high_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(slider->thumb_high_node, "class",
                                 "ui-range-slider-thumb");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(slider->thumb_high_node, "role", "slider");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(slider->thumb_high_node, "tabindex", "0");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_append_child(root_node, slider->thumb_low_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_append_child(root_node, slider->thumb_high_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc =
      ui_css_parse_stylesheet(ui_range_slider_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_component_set_default_style(slider->component, default_style);
  if (rc != UI_ERROR_NONE) {
    ui_css_stylesheet_destroy(default_style);
    goto cleanup;
  }

  slider->component->shadow_root = root_node;
  root_node = NULL;

  rc = update_dom_state(slider);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
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
  C_MULTIPLATFORM_FREE(slider);
  return rc;
}

/**
 * \brief Destroys a range slider base component.
 *
 * \param slider The range slider to destroy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_range_slider_base_destroy(struct ui_range_slider_base *slider) {
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t rc_cleanup;

  if (!slider)
    return UI_ERROR_NONE;

  rc_cleanup = ui_gesture_recognizer_destroy(slider->gesture_recognizer);
  if (rc_cleanup != UI_ERROR_NONE) {
    rc = rc_cleanup;
  }

  rc_cleanup = ui_component_destroy(slider->component);
  if (rc_cleanup != UI_ERROR_NONE) {
    rc = rc_cleanup;
  }

  C_MULTIPLATFORM_FREE(slider);
  return rc;
}

/**
 * \brief Sets the minimum value of the range slider.
 *
 * \param slider The range slider component.
 * \param min The new minimum value.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_range_slider_base_set_min(struct ui_range_slider_base *slider,
                                        float min) {
  ui_error_t rc;
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->min_val = min;
  if (slider->max_val < slider->min_val)
    slider->max_val = slider->min_val;
  if (slider->low_value < slider->min_val) {
    rc = ui_range_slider_base_set_values(slider, slider->min_val,
                                         slider->high_value);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return update_dom_state(slider);
}

/**
 * \brief Sets the maximum value of the range slider.
 *
 * \param slider The range slider component.
 * \param max The new maximum value.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_range_slider_base_set_max(struct ui_range_slider_base *slider,
                                        float max) {
  ui_error_t rc;
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->max_val = max;
  if (slider->min_val > slider->max_val)
    slider->min_val = slider->max_val;
  if (slider->high_value > slider->max_val) {
    rc = ui_range_slider_base_set_values(slider, slider->low_value,
                                         slider->max_val);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return update_dom_state(slider);
}

/**
 * \brief Sets the current values of the range slider.
 *
 * \param slider The range slider component.
 * \param low_value The new low value (clamped to bounds).
 * \param high_value The new high value (clamped to bounds).
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
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
    ui_error_t rc_dom;
    slider->low_value = new_low;
    slider->high_value = new_high;
    rc_dom = update_dom_state(slider);
    if (rc_dom != UI_ERROR_NONE) {
      return rc_dom;
    }
    if (slider->on_change) {
      ui_error_t rc_chg = slider->on_change(
          slider, slider->low_value, slider->high_value, slider->user_data);
      if (rc_chg != UI_ERROR_NONE) {
        return rc_chg;
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Gets the current values of the range slider.
 *
 * \param slider The range slider component.
 * \param out_low Pointer to receive the low value.
 * \param out_high Pointer to receive the high value.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_range_slider_base_get_values(const struct ui_range_slider_base *slider,
                                float *out_low, float *out_high) {
  if (!slider || !out_low || !out_high)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_low = slider->low_value;
  *out_high = slider->high_value;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the step increment. If 0.0, the slider is continuous.
 *
 * \param slider The range slider component.
 * \param step The step increment value.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
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

/**
 * \brief Sets the disabled state of the range slider.
 *
 * \param slider The range slider component.
 * \param disabled Non-zero to disable, 0 to enable.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_range_slider_base_set_disabled(struct ui_range_slider_base *slider,
                                  int disabled) {
  if (!slider)
    return UI_ERROR_INVALID_ARGUMENT;
  slider->disabled = disabled;
  return update_dom_state(slider);
}

/**
 * \brief Sets the change handler for the range slider.
 *
 * \param slider The range slider component.
 * \param on_change The callback invoked on value change.
 * \param user_data Opaque pointer passed to the callback.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
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

/**
 * \brief Processes an incoming input event to trigger slider interactions based
 * on normalized pointer position.
 *
 * \param slider The range slider component.
 * \param thumb Which thumb is active.
 * \param normalized_position The normalized position along the track (0.0
 * to 1.0).
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
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

/**
 * \brief Processes an incoming input event (e.g., keyboard interactions like
 * Arrow Keys).
 *
 * \param slider The range slider component.
 * \param event The input event.
 * \param active_thumb Which thumb is active for the keyboard event.
 * \param timestamp_ms Event timestamp in milliseconds.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_range_slider_base_process_event(
    struct ui_range_slider_base *slider, const struct ui_event *event,
    enum ui_range_slider_thumb active_thumb, double timestamp_ms) {
  ui_error_t rc;

  if (timestamp_ms < 0.0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!slider || !event)
    return UI_ERROR_INVALID_ARGUMENT;
  if (slider->disabled)
    return UI_ERROR_NONE;

  if (event->type == UI_EVENT_KEY_DOWN) {
    float increment = slider->step > 0.0f
                          ? slider->step
                          : (slider->max_val - slider->min_val) * 0.1f;
    enum ui_key_code key =
        (enum ui_key_code)event->event_data.keyboard.key_code;

    if (increment == 0.0f)
      increment = 1.0f;
    rc = ui_bidi_normalize_horizontal_key(key, &key);
    if (rc != UI_ERROR_NONE) {
      return rc;
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

/**
 * \brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * \param slider The range slider component.
 * \param out_component Pointer to receive the underlying component.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_range_slider_base_get_component(struct ui_range_slider_base *slider,
                                   struct ui_component **out_component) {
  if (!slider || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = slider->component;
  return UI_ERROR_NONE;
}
