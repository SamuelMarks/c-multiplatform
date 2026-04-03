/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_text_inputs.h"
#include <stddef.h>
#include <stdlib.h>
/* clang-format on */

CMP_API int cmp_f2_text_input_create(cmp_ui_node_t **out_node) {
  cmp_f2_text_input_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_text_input_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_text_input_t *)malloc(sizeof(cmp_f2_text_input_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->variant = CMP_F2_TEXT_INPUT_VARIANT_OUTLINE;
  data->size = CMP_F2_TEXT_INPUT_SIZE_MEDIUM;
  data->state = CMP_F2_TEXT_INPUT_STATE_REST;
  data->is_password = 0;
  data->leading_icon_node = NULL;
  data->trailing_icon_node = NULL;
  data->clear_button_node = NULL;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_text_input_set_variant(cmp_ui_node_t *node,
                                          cmp_f2_text_input_variant_t variant) {
  cmp_f2_text_input_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_text_input_t *)node->properties;
  data->variant = variant;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_text_input_set_size(cmp_ui_node_t *node,
                                       cmp_f2_text_input_size_t size) {
  cmp_f2_text_input_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_text_input_t *)node->properties;
  data->size = size;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_text_input_set_state(cmp_ui_node_t *node,
                                        cmp_f2_text_input_state_t state) {
  cmp_f2_text_input_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_text_input_t *)node->properties;
  data->state = state;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_text_input_set_password_mode(cmp_ui_node_t *node,
                                                int is_password) {
  cmp_f2_text_input_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_text_input_t *)node->properties;
  data->is_password = is_password ? 1 : 0;
  /* Further mapping onto actual generic text input attributes happens here */
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_textarea_create(cmp_ui_node_t **out_node) {
  cmp_f2_textarea_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  /* Note: c-multiplatform may not have a dedicated generic textarea right now,
   * use editable */
  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_textarea_t *)malloc(sizeof(cmp_f2_textarea_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->size = CMP_F2_TEXT_INPUT_SIZE_MEDIUM;
  data->state = CMP_F2_TEXT_INPUT_STATE_REST;
  data->resize_mode = CMP_F2_TEXTAREA_RESIZE_NONE;
  data->auto_grow = 0;
  data->max_height = 0.0f;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_textarea_set_size(cmp_ui_node_t *node,
                                     cmp_f2_text_input_size_t size) {
  cmp_f2_textarea_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_textarea_t *)node->properties;
  data->size = size;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_textarea_set_state(cmp_ui_node_t *node,
                                      cmp_f2_text_input_state_t state) {
  cmp_f2_textarea_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_textarea_t *)node->properties;
  data->state = state;
  return CMP_SUCCESS;
}

CMP_API int
cmp_f2_textarea_set_resize_mode(cmp_ui_node_t *node,
                                cmp_f2_textarea_resize_t resize_mode) {
  cmp_f2_textarea_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_textarea_t *)node->properties;
  data->resize_mode = resize_mode;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_textarea_set_auto_grow(cmp_ui_node_t *node, int auto_grow,
                                          float max_height) {
  cmp_f2_textarea_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_textarea_t *)node->properties;
  data->auto_grow = auto_grow ? 1 : 0;
  data->max_height = max_height;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_field_create(cmp_ui_node_t **out_node,
                                cmp_ui_node_t *input_node, const char *label) {
  cmp_f2_field_t *data;
  int res;

  if (!out_node || !input_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_field_t *)malloc(sizeof(cmp_f2_field_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->label_position = CMP_F2_FIELD_LABEL_TOP;
  data->is_required = 0;
  data->validation_state = CMP_F2_TEXT_INPUT_STATE_REST;
  data->input_node = input_node;
  data->label_node = NULL;
  data->help_text_node = NULL;
  data->validation_message_node = NULL;

  /* Mock label addition */
  if (label) {
    res = cmp_ui_text_create(&data->label_node, label, -1);
    if (res == CMP_SUCCESS) {
      cmp_ui_node_add_child(*out_node, data->label_node);
    }
  }

  cmp_ui_node_add_child(*out_node, input_node);

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_field_set_label_position(cmp_ui_node_t *node,
                                            cmp_f2_field_label_position_t pos) {
  cmp_f2_field_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_field_t *)node->properties;
  data->label_position = pos;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_field_set_required(cmp_ui_node_t *node, int is_required) {
  cmp_f2_field_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_field_t *)node->properties;
  data->is_required = is_required ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_field_set_help_text(cmp_ui_node_t *node,
                                       const char *help_text) {
  cmp_f2_field_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_field_t *)node->properties;

  if (data->help_text_node) {
    /* If changing text, map it natively. Here we mock it logicially */
  } else if (help_text) {
    cmp_ui_text_create(&data->help_text_node, help_text, -1);
    if (data->help_text_node) {
      cmp_ui_node_add_child(node, data->help_text_node);
    }
  }
  return CMP_SUCCESS;
}

CMP_API int
cmp_f2_field_set_validation_message(cmp_ui_node_t *node, const char *message,
                                    cmp_f2_text_input_state_t state) {
  cmp_f2_field_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_field_t *)node->properties;
  data->validation_state = state;
  /* Map message to child node similar to help_text */
  (void)message;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_searchbox_create(cmp_ui_node_t **out_node) {
  cmp_f2_searchbox_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_text_input_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_searchbox_t *)malloc(sizeof(cmp_f2_searchbox_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->size = CMP_F2_TEXT_INPUT_SIZE_MEDIUM;
  data->state = CMP_F2_TEXT_INPUT_STATE_REST;
  data->is_collapsed = 0;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_searchbox_set_size(cmp_ui_node_t *node,
                                      cmp_f2_text_input_size_t size) {
  cmp_f2_searchbox_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_searchbox_t *)node->properties;
  data->size = size;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_searchbox_set_state(cmp_ui_node_t *node,
                                       cmp_f2_text_input_state_t state) {
  cmp_f2_searchbox_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_searchbox_t *)node->properties;
  data->state = state;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_searchbox_set_collapsed_mode(cmp_ui_node_t *node,
                                                int is_collapsed) {
  cmp_f2_searchbox_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_searchbox_t *)node->properties;
  data->is_collapsed = is_collapsed ? 1 : 0;
  return CMP_SUCCESS;
}
