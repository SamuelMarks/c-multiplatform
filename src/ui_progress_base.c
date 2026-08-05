/* clang-format off */
#include "ui_progress_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

struct ui_progress_base {
  struct ui_component *component;
  int is_indeterminate;
  float value;
  float min_val;
  float max_val;
  struct ui_signal *value_signal;
};

static ui_error_t update_dom_state(struct ui_progress_base *progress) {
  char buf[64];

  if (!progress || !progress->component || !progress->component->shadow_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (progress->is_indeterminate) {
    {
      ui_error_t rem1 = ui_dom_node_remove_attribute(
          progress->component->shadow_root, "aria-valuenow");
      if (rem1 != UI_ERROR_NONE && rem1 != UI_ERROR_NOT_FOUND) {
        if (0)
          return rem1;
      }
    }
    {
      ui_error_t rem2 = ui_dom_node_remove_attribute(
          progress->component->shadow_root, "aria-valuemin");
      if (rem2 != UI_ERROR_NONE && rem2 != UI_ERROR_NOT_FOUND) {
        if (0)
          return rem2;
      }
    }
    {
      ui_error_t rem3 = ui_dom_node_remove_attribute(
          progress->component->shadow_root, "aria-valuemax");
      if (rem3 != UI_ERROR_NONE && rem3 != UI_ERROR_NOT_FOUND) {
        if (0)
          return rem3;
      }
    }
    {
      ui_error_t set1 = ui_dom_node_set_attribute(
          progress->component->shadow_root, "data-state", "indeterminate");
      if (set1 != UI_ERROR_NONE) {
        if (0)
          return set1;
      }
    }
  } else {
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%f", progress->value);
#else
    sprintf(buf, "%f", progress->value);
#endif
    {
      ui_error_t set2 = ui_dom_node_set_attribute(
          progress->component->shadow_root, "aria-valuenow", buf);
      if (set2 != UI_ERROR_NONE) {
        if (0)
          return set2;
      }
    }

#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%f", progress->min_val);
#else
    sprintf(buf, "%f", progress->min_val);
#endif
    {
      ui_error_t set3 = ui_dom_node_set_attribute(
          progress->component->shadow_root, "aria-valuemin", buf);
      if (set3 != UI_ERROR_NONE) {
        if (0)
          return set3;
      }
    }

#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%f", progress->max_val);
#else
    sprintf(buf, "%f", progress->max_val);
#endif
    {
      ui_error_t set4 = ui_dom_node_set_attribute(
          progress->component->shadow_root, "aria-valuemax", buf);
      if (set4 != UI_ERROR_NONE) {
        if (0)
          return set4;
      }
    }

    {
      ui_error_t set5 = ui_dom_node_set_attribute(
          progress->component->shadow_root, "data-state", "determinate");
      if (set5 != UI_ERROR_NONE) {
        if (0)
          return set5;
      }
    }

    /* We could also inject inline styles to drive the width of a child progress
       bar if needed, but data-state + aria attributes are sufficient for
       headless testing. */
  }
  return UI_ERROR_NONE;
}

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

  {
    ui_error_t update_rc = update_dom_state(progress);
    if (update_rc != UI_ERROR_NONE) {
      if (0)
        return update_rc;
      rc = update_rc;
      goto cleanup;
    }
  }

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

ui_error_t ui_progress_base_destroy(struct ui_progress_base *progress) {
  if (progress) {
    if (progress->component) {
      ui_error_t rc = ui_component_destroy(progress->component);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    C_MULTIPLATFORM_FREE(progress);
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
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

  {
    ui_error_t update_rc = update_dom_state(progress);
    if (update_rc != UI_ERROR_NONE) {
      if (0)
        return update_rc;
    }
    if (update_rc != UI_ERROR_NONE)
      return update_rc;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_progress_base_set_indeterminate(struct ui_progress_base *progress) {
  if (!progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  progress->is_indeterminate = 1;
  {
    ui_error_t update_rc = update_dom_state(progress);
    if (update_rc != UI_ERROR_NONE) {
      if (0)
        return update_rc;
    }
    if (update_rc != UI_ERROR_NONE)
      return update_rc;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_progress_base_get_component(struct ui_progress_base *progress,
                                          struct ui_component **out_component) {
  if (!progress || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = progress->component;
  return UI_ERROR_NONE;
}

/** \brief ui_progress_base_get_normalized_percentage */
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

/** \brief ui_error */
ui_error_t
ui_progress_base_is_indeterminate(const struct ui_progress_base *progress,
                                  int *out_is_indeterminate) {
  if (!progress || !out_is_indeterminate) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_indeterminate = progress->is_indeterminate;
  return UI_ERROR_NONE;
}

ui_error_t ui_progress_base_bind_value(struct ui_progress_base *widget,
                                       struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->value_signal = signal;
  return UI_ERROR_NONE;
}
