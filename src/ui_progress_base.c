/**
 * \file ui_progress_base.c
 * \brief Implementation of the UI Progress Base component.
 */

/* clang-format off */
#include "ui_progress_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

/**
 * \brief Internal structure representing a progress component.
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
 * \brief Updates the DOM node attributes according to the progress state.
 *
 * \param progress The progress component to update.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t update_dom_state(struct ui_progress_base *progress) {
  char buf[64];

  if (progress->is_indeterminate) {
    {
      (void)ui_dom_node_remove_attribute(progress->component->shadow_root,
                                         "aria-valuenow");
    }
    {
      (void)ui_dom_node_remove_attribute(progress->component->shadow_root,
                                         "aria-valuemin");
    }
    {
      (void)ui_dom_node_remove_attribute(progress->component->shadow_root,
                                         "aria-valuemax");
    }
    {
      (void)ui_dom_node_set_attribute(progress->component->shadow_root,
                                      "data-state", "indeterminate");
    }
  } else {
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%f", progress->value);
#else
    sprintf(buf, "%f", progress->value);
#endif
    {
      (void)ui_dom_node_set_attribute(progress->component->shadow_root,
                                      "aria-valuenow", buf);
    }

#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%f", progress->min_val);
#else
    sprintf(buf, "%f", progress->min_val);
#endif
    {
      (void)ui_dom_node_set_attribute(progress->component->shadow_root,
                                      "aria-valuemin", buf);
    }

#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%f", progress->max_val);
#else
    sprintf(buf, "%f", progress->max_val);
#endif
    {
      (void)ui_dom_node_set_attribute(progress->component->shadow_root,
                                      "aria-valuemax", buf);
    }

    {
      (void)ui_dom_node_set_attribute(progress->component->shadow_root,
                                      "data-state", "determinate");
    }

    /* We could also inject inline styles to drive the width of a child progress
       bar if needed, but data-state + aria attributes are sufficient for
       headless testing. */
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Creates a new progress base component.
 *
 * \param out_progress Pointer to receive the allocated progress component.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_progress_base_create(struct ui_progress_base **out_progress) {
  struct ui_progress_base *progress;
  ui_error_t rc;
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

  (void)update_dom_state(progress);

  *out_progress = progress;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    (void)ui_dom_node_destroy(root_node);
  }
  if (progress->component) {
    (void)ui_component_destroy(progress->component);
  }
  C_MULTIPLATFORM_FREE(progress);
  return rc;
}

/**
 * \brief Destroys a progress component.
 *
 * \param progress The progress component to destroy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_progress_base_destroy(struct ui_progress_base *progress) {
  if (progress) {
    (void)ui_component_destroy(progress->component);
    C_MULTIPLATFORM_FREE(progress);
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the component to determinate mode and updates the value.
 *
 * \param progress The progress component.
 * \param value The progress value (will be clamped between min and max).
 * \param min The minimum possible value (e.g., 0.0f).
 * \param max The maximum possible value (e.g., 100.0f).
 * \return UI_ERROR_NONE on success, or an appropriate error code.
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
 * \brief Sets the component to indeterminate mode.
 *
 * \param progress The progress component.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
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
 * \brief Retrieves the underlying UI component.
 *
 * \param progress The progress component.
 * \param out_component Pointer to receive the underlying component.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
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
 * \brief Gets the current normalized percentage [0.0, 1.0].
 * If the progress is indeterminate, this returns 0.0.
 *
 * \param progress The progress component.
 * \param out_percentage Pointer to receive the normalized percentage.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
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
 * \brief Checks if the progress is currently indeterminate.
 *
 * \param progress The progress component.
 * \param out_is_indeterminate Pointer to receive the indeterminate state.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
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
 * \brief Binds the value property.
 *
 * \param widget The progress component.
 * \param signal The signal to bind to.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_progress_base_bind_value(struct ui_progress_base *widget,
                                       struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->value_signal = signal;
  return UI_ERROR_NONE;
}
