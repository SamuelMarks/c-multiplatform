/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include "themes/cmp_f2_inputs.h"
#include <stddef.h>
#include <stdlib.h>
/* clang-format on */

/**
 * @brief cmp_f2_checkbox_create
 *
 * @param out_node Parameter description.
 * @param label Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_checkbox_create(cmp_ui_node_t **out_node,
                                   const char *label) {
  int rc = 0;
  cmp_f2_checkbox_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_checkbox_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_checkbox_create(out_node, label);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_checkbox_create: cmp_ui_checkbox_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_checkbox_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_checkbox_create: OOM\n");
    (void)cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->size = CMP_F2_CHECKBOX_SIZE_MEDIUM;
  data->state = CMP_F2_CHECKBOX_STATE_UNCHECKED;
  data->label_position = CMP_F2_LABEL_POSITION_AFTER;
  data->is_disabled = 0;

  (*out_node)->properties = (void *)data;

  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_checkbox_set_size
 *
 * @param node Parameter description.
 * @param size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_checkbox_set_size(cmp_ui_node_t *node,
                                     cmp_f2_checkbox_size_t size) {
  int rc = 0;
  cmp_f2_checkbox_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_checkbox_set_size: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_checkbox_t *)node->properties;
  data->size = size;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_checkbox_set_state
 *
 * @param node Parameter description.
 * @param state Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_checkbox_set_state(cmp_ui_node_t *node,
                                      cmp_f2_checkbox_state_t state) {
  int rc = 0;
  cmp_f2_checkbox_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_checkbox_set_state: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_checkbox_t *)node->properties;
  data->state = state;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_checkbox_set_label_position
 *
 * @param node Parameter description.
 * @param pos Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_checkbox_set_label_position(cmp_ui_node_t *node,
                                               cmp_f2_label_position_t pos) {
  int rc = 0;
  cmp_f2_checkbox_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_checkbox_set_label_position: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_checkbox_t *)node->properties;
  data->label_position = pos;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_checkbox_set_disabled
 *
 * @param node Parameter description.
 * @param disabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_checkbox_set_disabled(cmp_ui_node_t *node, int disabled) {
  int rc = 0;
  cmp_f2_checkbox_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_checkbox_set_disabled: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_checkbox_t *)node->properties;
  data->is_disabled = disabled ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_radio_create
 *
 * @param out_node Parameter description.
 * @param label Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_radio_create(cmp_ui_node_t **out_node, const char *label) {
  int rc = 0;
  cmp_f2_radio_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_radio_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_radio_create(out_node, 0); /* Default group 0 */
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_radio_create: cmp_ui_radio_create failed\n");
    return res;
  }

  /* Since radio in generic doesn't take label directly in create, we'd mock it
   * for now */
  (void)label;

  res = CMP_MALLOC(sizeof(cmp_f2_radio_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_radio_create: OOM\n");
    (void)cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->size = CMP_F2_RADIO_SIZE_MEDIUM;
  data->label_position = CMP_F2_LABEL_POSITION_AFTER;
  data->is_selected = 0;
  data->is_disabled = 0;

  (*out_node)->properties = (void *)data;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_radio_set_size
 *
 * @param node Parameter description.
 * @param size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_radio_set_size(cmp_ui_node_t *node,
                                  cmp_f2_radio_size_t size) {
  int rc = 0;
  cmp_f2_radio_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_radio_set_size: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_radio_t *)node->properties;
  data->size = size;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_radio_set_selected
 *
 * @param node Parameter description.
 * @param selected Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_radio_set_selected(cmp_ui_node_t *node, int selected) {
  int rc = 0;
  cmp_f2_radio_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_radio_set_selected: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_radio_t *)node->properties;
  data->is_selected = selected ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_radio_set_disabled
 *
 * @param node Parameter description.
 * @param disabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_radio_set_disabled(cmp_ui_node_t *node, int disabled) {
  int rc = 0;
  cmp_f2_radio_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_radio_set_disabled: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_radio_t *)node->properties;
  data->is_disabled = disabled ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_radio_group_create
 *
 * @param out_node Parameter description.
 * @param horizontal Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_radio_group_create(cmp_ui_node_t **out_node,
                                      int horizontal) {
  int rc = 0;
  int res;
  if (!out_node) {
    LOG_DEBUG("cmp_f2_radio_group_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  res = cmp_ui_box_create(out_node);
  if (res == CMP_SUCCESS) {
    (*out_node)->layout->direction =
        horizontal ? CMP_FLEX_ROW : CMP_FLEX_COLUMN;
  } else {
    LOG_DEBUG("cmp_f2_radio_group_create: cmp_ui_box_create failed\n");
  }
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return res;
}

/**
 * @brief cmp_f2_toggle_create
 *
 * @param out_node Parameter description.
 * @param label Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_toggle_create(cmp_ui_node_t **out_node, const char *label) {
  int rc = 0;
  cmp_f2_toggle_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_toggle_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node); /* There is no generic toggle component.
                                        Treat it as a styled composite box */
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_toggle_create: cmp_ui_box_create failed\n");
    return res;
  }

  (void)label;

  res = CMP_MALLOC(sizeof(cmp_f2_toggle_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_toggle_create: OOM\n");
    (void)cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->is_on = 0;
  data->is_disabled = 0;
  data->label_position = CMP_F2_LABEL_POSITION_AFTER;

  (*out_node)->properties = (void *)data;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_toggle_set_on
 *
 * @param node Parameter description.
 * @param is_on Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_toggle_set_on(cmp_ui_node_t *node, int is_on) {
  int rc = 0;
  cmp_f2_toggle_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_toggle_set_on: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_toggle_t *)node->properties;
  data->is_on = is_on ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_toggle_set_disabled
 *
 * @param node Parameter description.
 * @param disabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_toggle_set_disabled(cmp_ui_node_t *node, int disabled) {
  int rc = 0;
  cmp_f2_toggle_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_toggle_set_disabled: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_toggle_t *)node->properties;
  data->is_disabled = disabled ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_slider_create
 *
 * @param out_node Parameter description.
 * @param orientation Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_slider_create(cmp_ui_node_t **out_node,
                                 cmp_f2_slider_orientation_t orientation) {
  int rc = 0;
  cmp_f2_slider_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_slider_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_slider_create(out_node, 0.0f, 100.0f);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_slider_create: cmp_ui_slider_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_slider_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_slider_create: OOM\n");
    (void)cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->orientation = orientation;
  data->is_range = 0;
  data->is_stepped = 0;
  data->min_val = 0.0f;
  data->max_val = 100.0f;
  data->step_val = 1.0f;
  data->value1 = 0.0f;
  data->value2 = 100.0f;
  data->is_disabled = 0;

  (*out_node)->properties = (void *)data;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_slider_set_range_mode
 *
 * @param node Parameter description.
 * @param is_range Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_slider_set_range_mode(cmp_ui_node_t *node, int is_range) {
  int rc = 0;
  cmp_f2_slider_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_slider_set_range_mode: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_slider_t *)node->properties;
  data->is_range = is_range ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_slider_set_bounds
 *
 * @param node Parameter description.
 * @param min_val Parameter description.
 * @param max_val Parameter description.
 * @param step_val Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_slider_set_bounds(cmp_ui_node_t *node, float min_val,
                                     float max_val, float step_val) {
  int rc = 0;
  cmp_f2_slider_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_slider_set_bounds: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_slider_t *)node->properties;
  data->min_val = min_val;
  data->max_val = max_val;
  data->step_val = step_val;
  data->is_stepped = (step_val > 0.0f) ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_slider_set_value
 *
 * @param node Parameter description.
 * @param val1 Parameter description.
 * @param val2 Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_slider_set_value(cmp_ui_node_t *node, float val1,
                                    float val2) {
  int rc = 0;
  cmp_f2_slider_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_slider_set_value: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_slider_t *)node->properties;
  data->value1 = val1;
  data->value2 = val2;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_slider_set_disabled
 *
 * @param node Parameter description.
 * @param disabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_slider_set_disabled(cmp_ui_node_t *node, int disabled) {
  int rc = 0;
  cmp_f2_slider_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_slider_set_disabled: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_slider_t *)node->properties;
  data->is_disabled = disabled ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_spin_button_create
 *
 * @param out_node Parameter description.
 * @param initial_val Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_spin_button_create(cmp_ui_node_t **out_node,
                                      float initial_val) {
  int rc = 0;
  cmp_f2_spin_button_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_spin_button_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node); /* SpinButton is a composite */
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_spin_button_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_spin_button_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_spin_button_create: OOM\n");
    (void)cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->value = initial_val;
  data->min_val = 0.0f;
  data->max_val = 100.0f;
  data->step_val = 1.0f;
  data->is_disabled = 0;

  (*out_node)->properties = (void *)data;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_spin_button_set_bounds
 *
 * @param node Parameter description.
 * @param min_val Parameter description.
 * @param max_val Parameter description.
 * @param step_val Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_spin_button_set_bounds(cmp_ui_node_t *node, float min_val,
                                          float max_val, float step_val) {
  int rc = 0;
  cmp_f2_spin_button_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_spin_button_set_bounds: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_spin_button_t *)node->properties;
  data->min_val = min_val;
  data->max_val = max_val;
  data->step_val = step_val;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_spin_button_set_value
 *
 * @param node Parameter description.
 * @param val Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_spin_button_set_value(cmp_ui_node_t *node, float val) {
  int rc = 0;
  cmp_f2_spin_button_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_spin_button_set_value: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_spin_button_t *)node->properties;
  data->value = val;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_spin_button_set_disabled
 *
 * @param node Parameter description.
 * @param disabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_spin_button_set_disabled(cmp_ui_node_t *node, int disabled) {
  int rc = 0;
  cmp_f2_spin_button_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_spin_button_set_disabled: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_spin_button_t *)node->properties;
  data->is_disabled = disabled ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
