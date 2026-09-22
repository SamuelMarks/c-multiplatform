/**
 * @file ui_progress_base.c
 * @brief Implementation of the UI Progress Base component.
 */

/* clang-format off */
#include "ui_progress_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_progress_mock_remove_fail = 0;
int g_progress_mock_set_fail = 0;
int g_progress_mock_destroy_node_fail = 0;
int g_progress_mock_destroy_comp_fail = 0;

/**
 * @brief Mock implementation of ui_dom_node_remove_attribute for error
 * injection.
 * @param node Target DOM node.
 * @param name Attribute name.
 * @return Error code.
 */
static ui_error_t
mock_progress_dom_node_remove_attribute(struct ui_dom_node *node,
                                        const char *name) {
  if (g_progress_mock_remove_fail == 1) {
    g_progress_mock_remove_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  if (g_progress_mock_remove_fail == 2) {
    g_progress_mock_remove_fail = 20;
  } else if (g_progress_mock_remove_fail == 20) {
    g_progress_mock_remove_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  if (g_progress_mock_remove_fail == 3) {
    g_progress_mock_remove_fail = 30;
  } else if (g_progress_mock_remove_fail == 30) {
    g_progress_mock_remove_fail = 31;
  } else if (g_progress_mock_remove_fail == 31) {
    g_progress_mock_remove_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_remove_attribute)(node, name);
}
#undef ui_dom_node_remove_attribute
/** @cond */
#define ui_dom_node_remove_attribute mock_progress_dom_node_remove_attribute
/** @endcond */

/**
 * @brief Mock implementation of ui_dom_node_set_attribute for error injection.
 * @param node Target DOM node.
 * @param name Attribute name.
 * @param value Attribute value.
 * @return Error code.
 */
static ui_error_t mock_progress_dom_node_set_attribute(struct ui_dom_node *node,
                                                       const char *name,
                                                       const char *value) {
  if (g_progress_mock_set_fail == 1) {
    g_progress_mock_set_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  if (g_progress_mock_set_fail == 2) {
    g_progress_mock_set_fail = 20;
  } else if (g_progress_mock_set_fail == 20) {
    g_progress_mock_set_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  if (g_progress_mock_set_fail == 3) {
    g_progress_mock_set_fail = 30;
  } else if (g_progress_mock_set_fail == 30) {
    g_progress_mock_set_fail = 31;
  } else if (g_progress_mock_set_fail == 31) {
    g_progress_mock_set_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  if (g_progress_mock_set_fail == 4) {
    g_progress_mock_set_fail = 40;
  } else if (g_progress_mock_set_fail == 40) {
    g_progress_mock_set_fail = 41;
  } else if (g_progress_mock_set_fail == 41) {
    g_progress_mock_set_fail = 42;
  } else if (g_progress_mock_set_fail == 42) {
    g_progress_mock_set_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  if (g_progress_mock_set_fail == 5) {
    g_progress_mock_set_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_set_attribute)(node, name, value);
}
#undef ui_dom_node_set_attribute
/** @cond */
#define ui_dom_node_set_attribute mock_progress_dom_node_set_attribute
/** @endcond */

/**
 * @brief Mock implementation of ui_dom_node_destroy for error injection.
 * @param node Target DOM node.
 * @return Error code.
 */
static ui_error_t mock_progress_dom_node_destroy(struct ui_dom_node *node) {
  if (g_progress_mock_destroy_node_fail > 0) {
    g_progress_mock_destroy_node_fail--;
    (ui_dom_node_destroy)(node);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_destroy)(node);
}
#undef ui_dom_node_destroy
/** @cond */
#define ui_dom_node_destroy mock_progress_dom_node_destroy
/** @endcond */

/**
 * @brief Mock implementation of ui_component_destroy for error injection.
 * @param comp Target UI component.
 * @return Error code.
 */
static ui_error_t mock_progress_component_destroy(struct ui_component *comp) {
  if (g_progress_mock_destroy_comp_fail > 0) {
    g_progress_mock_destroy_comp_fail--;
    (ui_component_destroy)(comp);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_destroy)(comp);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_progress_component_destroy
/** @endcond */
#endif

/**
 * @struct ui_progress_base
 * @brief Internal structure representing a progress component.
 */
struct ui_progress_base {
  struct ui_component *component; /**< Underlying component */
  int is_indeterminate;           /**< Non-zero if in indeterminate mode */
  float value;                    /**< Current value */
  float min_val;                  /**< Minimum value */
  float max_val;                  /**< Maximum value */
  struct ui_signal *value_signal; /**< Signal bound to the value */
};

/**
 * @brief Updates the DOM node attributes according to the progress state.
 * @param progress The progress component to update.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t update_dom_state(struct ui_progress_base *progress) {
  char buf[64];
  ui_error_t rc;

  if (progress->is_indeterminate) {
    rc = ui_dom_node_remove_attribute(progress->component->shadow_root,
                                      "aria-valuenow");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = ui_dom_node_remove_attribute(progress->component->shadow_root,
                                      "aria-valuemin");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = ui_dom_node_remove_attribute(progress->component->shadow_root,
                                      "aria-valuemax");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = ui_dom_node_set_attribute(progress->component->shadow_root,
                                   "data-state", "indeterminate");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%f", progress->value);
#else
    sprintf(buf, "%f", progress->value);
#endif
    rc = ui_dom_node_set_attribute(progress->component->shadow_root,
                                   "aria-valuenow", buf);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%f", progress->min_val);
#else
    sprintf(buf, "%f", progress->min_val);
#endif
    rc = ui_dom_node_set_attribute(progress->component->shadow_root,
                                   "aria-valuemin", buf);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%f", progress->max_val);
#else
    sprintf(buf, "%f", progress->max_val);
#endif
    rc = ui_dom_node_set_attribute(progress->component->shadow_root,
                                   "aria-valuemax", buf);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    rc = ui_dom_node_set_attribute(progress->component->shadow_root,
                                   "data-state", "determinate");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Creates a new progress base component.
 * @param out_progress Pointer to receive the allocated progress component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_progress_base_create(struct ui_progress_base **out_progress) {
  struct ui_progress_base *progress;
  ui_error_t rc;
  ui_error_t rc_cleanup;
  struct ui_dom_node *root_node = NULL;

  if (!out_progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  progress = (struct ui_progress_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_progress_base));
  if (!progress) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  progress->component = NULL;
  progress->is_indeterminate = 1;
  progress->value = 0.0f;
  progress->min_val = 0.0f;
  progress->max_val = 100.0f;
  progress->value_signal = NULL;

  rc = ui_component_create(&progress->component);
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

  rc = ui_dom_node_set_attribute(root_node, "role", "progressbar");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  progress->component->shadow_root = root_node;
  root_node = NULL;

  rc = update_dom_state(progress);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  *out_progress = progress;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    rc_cleanup = ui_dom_node_destroy(root_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* Preserve primary error */
    }
  }
  if (progress->component) {
    rc_cleanup = ui_component_destroy(progress->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* Preserve primary error */
    }
  }
  C_MULTIPLATFORM_FREE(progress);
  return rc;
}

/**
 * @brief Destroys a progress component.
 * @param progress The progress component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_progress_base_destroy(struct ui_progress_base *progress) {
  ui_error_t rc = UI_ERROR_NONE;
  if (progress) {
    rc = ui_component_destroy(progress->component);
    C_MULTIPLATFORM_FREE(progress);
  }
  return rc;
}

/**
 * @brief Sets the component to determinate mode and updates the value.
 * @param progress The progress component.
 * @param value The progress value (will be clamped between min and max).
 * @param min The minimum possible value (e.g., 0.0f).
 * @param max The maximum possible value (e.g., 100.0f).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_progress_base_set_determinate(struct ui_progress_base *progress,
                                            float value, float min, float max) {
  if (!progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (min > max) {
    float temp = min;
    min = max;
    max = temp;
  }

  if (value < min) {
    value = min;
  }
  if (value > max) {
    value = max;
  }

  progress->is_indeterminate = 0;
  progress->value = value;
  progress->min_val = min;
  progress->max_val = max;

  return update_dom_state(progress);
}

/**
 * @brief Sets the component to indeterminate mode.
 * @param progress The progress component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_progress_base_set_indeterminate(struct ui_progress_base *progress) {
  if (!progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  progress->is_indeterminate = 1;
  return update_dom_state(progress);
}

/**
 * @brief Retrieves the underlying UI component.
 * @param progress The progress component.
 * @param out_component Pointer to receive the underlying component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_progress_base_get_component(struct ui_progress_base *progress,
                                          struct ui_component **out_component) {
  if (!progress || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = progress->component;
  return UI_ERROR_NONE;
}

/**
 * @brief Gets the current normalized percentage [0.0, 1.0].
 * @param progress The progress component.
 * @param out_percentage Pointer to receive the normalized percentage.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_progress_base_get_normalized_percentage(
    const struct ui_progress_base *progress, float *out_percentage) {
  float range;
  if (!progress || !out_percentage) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_percentage = 0.0f;
  if (progress->is_indeterminate) {
    return UI_ERROR_NONE;
  }
  range = progress->max_val - progress->min_val;
  if (range > 0.0f) {
    *out_percentage = (progress->value - progress->min_val) / range;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Checks if the progress is currently indeterminate.
 * @param progress The progress component.
 * @param out_is_indeterminate Pointer to receive the indeterminate state.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_progress_base_is_indeterminate(const struct ui_progress_base *progress,
                                  int *out_is_indeterminate) {
  if (!progress || !out_is_indeterminate) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_indeterminate = progress->is_indeterminate;
  return UI_ERROR_NONE;
}

/**
 * @brief Binds the value property.
 * @param widget The progress component.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_progress_base_bind_value(struct ui_progress_base *widget,
                                       struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->value_signal = signal;
  return UI_ERROR_NONE;
}
