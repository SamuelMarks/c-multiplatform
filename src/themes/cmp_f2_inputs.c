/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_inputs.h"
#include <stddef.h>
#include <stdlib.h>
/* clang-format on */

CMP_API int cmp_f2_checkbox_create(cmp_ui_node_t **out_node,
                                   const char *label) {
  cmp_f2_checkbox_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_checkbox_create(out_node, label);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_checkbox_t *)malloc(sizeof(cmp_f2_checkbox_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->size = CMP_F2_CHECKBOX_SIZE_MEDIUM;
  data->state = CMP_F2_CHECKBOX_STATE_UNCHECKED;
  data->label_position = CMP_F2_LABEL_POSITION_AFTER;
  data->is_disabled = 0;

  (*out_node)->properties = (void *)data;

  return CMP_SUCCESS;
}

CMP_API int cmp_f2_checkbox_set_size(cmp_ui_node_t *node,
                                     cmp_f2_checkbox_size_t size) {
  cmp_f2_checkbox_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_checkbox_t *)node->properties;
  data->size = size;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_checkbox_set_state(cmp_ui_node_t *node,
                                      cmp_f2_checkbox_state_t state) {
  cmp_f2_checkbox_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_checkbox_t *)node->properties;
  data->state = state;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_checkbox_set_label_position(cmp_ui_node_t *node,
                                               cmp_f2_label_position_t pos) {
  cmp_f2_checkbox_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_checkbox_t *)node->properties;
  data->label_position = pos;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_checkbox_set_disabled(cmp_ui_node_t *node, int disabled) {
  cmp_f2_checkbox_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_checkbox_t *)node->properties;
  data->is_disabled = disabled ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_radio_create(cmp_ui_node_t **out_node, const char *label) {
  cmp_f2_radio_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_radio_create(out_node, 0); /* Default group 0 */
  if (res != CMP_SUCCESS)
    return res;

  /* Since radio in generic doesn't take label directly in create, we'd mock it
   * for now */
  (void)label;

  data = (cmp_f2_radio_t *)malloc(sizeof(cmp_f2_radio_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->size = CMP_F2_RADIO_SIZE_MEDIUM;
  data->label_position = CMP_F2_LABEL_POSITION_AFTER;
  data->is_selected = 0;
  data->is_disabled = 0;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_radio_set_size(cmp_ui_node_t *node,
                                  cmp_f2_radio_size_t size) {
  cmp_f2_radio_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_radio_t *)node->properties;
  data->size = size;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_radio_set_selected(cmp_ui_node_t *node, int selected) {
  cmp_f2_radio_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_radio_t *)node->properties;
  data->is_selected = selected ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_radio_set_disabled(cmp_ui_node_t *node, int disabled) {
  cmp_f2_radio_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_radio_t *)node->properties;
  data->is_disabled = disabled ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_radio_group_create(cmp_ui_node_t **out_node,
                                      int horizontal) {
  int res;
  if (!out_node)
    return CMP_ERROR_INVALID_ARG;
  res = cmp_ui_box_create(out_node);
  if (res == CMP_SUCCESS) {
    (*out_node)->layout->direction =
        horizontal ? CMP_FLEX_ROW : CMP_FLEX_COLUMN;
  }
  return res;
}

CMP_API int cmp_f2_toggle_create(cmp_ui_node_t **out_node, const char *label) {
  cmp_f2_toggle_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node); /* There is no generic toggle component.
                                        Treat it as a styled composite box */
  if (res != CMP_SUCCESS)
    return res;

  (void)label;

  data = (cmp_f2_toggle_t *)malloc(sizeof(cmp_f2_toggle_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->is_on = 0;
  data->is_disabled = 0;
  data->label_position = CMP_F2_LABEL_POSITION_AFTER;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_toggle_set_on(cmp_ui_node_t *node, int is_on) {
  cmp_f2_toggle_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_toggle_t *)node->properties;
  data->is_on = is_on ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_toggle_set_disabled(cmp_ui_node_t *node, int disabled) {
  cmp_f2_toggle_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_toggle_t *)node->properties;
  data->is_disabled = disabled ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_slider_create(cmp_ui_node_t **out_node,
                                 cmp_f2_slider_orientation_t orientation) {
  cmp_f2_slider_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_slider_create(out_node, 0.0f, 100.0f);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_slider_t *)malloc(sizeof(cmp_f2_slider_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
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
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_slider_set_range_mode(cmp_ui_node_t *node, int is_range) {
  cmp_f2_slider_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_slider_t *)node->properties;
  data->is_range = is_range ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_slider_set_bounds(cmp_ui_node_t *node, float min_val,
                                     float max_val, float step_val) {
  cmp_f2_slider_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_slider_t *)node->properties;
  data->min_val = min_val;
  data->max_val = max_val;
  data->step_val = step_val;
  data->is_stepped = (step_val > 0.0f) ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_slider_set_value(cmp_ui_node_t *node, float val1,
                                    float val2) {
  cmp_f2_slider_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_slider_t *)node->properties;
  data->value1 = val1;
  data->value2 = val2;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_slider_set_disabled(cmp_ui_node_t *node, int disabled) {
  cmp_f2_slider_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_slider_t *)node->properties;
  data->is_disabled = disabled ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_spin_button_create(cmp_ui_node_t **out_node,
                                      float initial_val) {
  cmp_f2_spin_button_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node); /* SpinButton is a composite */
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_spin_button_t *)malloc(sizeof(cmp_f2_spin_button_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->value = initial_val;
  data->min_val = 0.0f;
  data->max_val = 100.0f;
  data->step_val = 1.0f;
  data->is_disabled = 0;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_spin_button_set_bounds(cmp_ui_node_t *node, float min_val,
                                          float max_val, float step_val) {
  cmp_f2_spin_button_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_spin_button_t *)node->properties;
  data->min_val = min_val;
  data->max_val = max_val;
  data->step_val = step_val;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_spin_button_set_value(cmp_ui_node_t *node, float val) {
  cmp_f2_spin_button_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_spin_button_t *)node->properties;
  data->value = val;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_spin_button_set_disabled(cmp_ui_node_t *node, int disabled) {
  cmp_f2_spin_button_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_spin_button_t *)node->properties;
  data->is_disabled = disabled ? 1 : 0;
  return CMP_SUCCESS;
}
