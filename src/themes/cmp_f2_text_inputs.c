/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include "themes/cmp_f2_text_inputs.h"
#include <stddef.h>
#include <stdlib.h>
/* clang-format on */

/**
 * @brief cmp_f2_text_input_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_text_input_create(cmp_ui_node_t **out_node) {
  int rc;
  rc = 0;
  cmp_f2_text_input_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_text_input_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_text_input_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_text_input_create: cmp_ui_text_input_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_text_input_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_text_input_create: OOM\n");
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
 * @brief cmp_f2_text_input_set_variant
 *
 * @param node Parameter description.
 * @param variant Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_text_input_set_variant(cmp_ui_node_t *node,
                                          cmp_f2_text_input_variant_t variant) {
  int rc;
  rc = 0;
  cmp_f2_text_input_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_text_input_set_variant: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_text_input_t *)node->properties;
  data->variant = variant;
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
 * @brief cmp_f2_text_input_set_size
 *
 * @param node Parameter description.
 * @param size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_text_input_set_size(cmp_ui_node_t *node,
                                       cmp_f2_text_input_size_t size) {
  int rc;
  rc = 0;
  cmp_f2_text_input_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_text_input_set_size: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_text_input_t *)node->properties;
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
 * @brief cmp_f2_text_input_set_state
 *
 * @param node Parameter description.
 * @param state Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_text_input_set_state(cmp_ui_node_t *node,
                                        cmp_f2_text_input_state_t state) {
  int rc;
  rc = 0;
  cmp_f2_text_input_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_text_input_set_state: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_text_input_t *)node->properties;
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
 * @brief cmp_f2_text_input_set_password_mode
 *
 * @param node Parameter description.
 * @param is_password Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_text_input_set_password_mode(cmp_ui_node_t *node,
                                                int is_password) {
  int rc;
  rc = 0;
  cmp_f2_text_input_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_text_input_set_password_mode: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_text_input_t *)node->properties;
  data->is_password = is_password ? 1 : 0;
  /* Further mapping onto actual generic text input attributes happens here */
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
 * @brief cmp_f2_textarea_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_textarea_create(cmp_ui_node_t **out_node) {
  int rc;
  rc = 0;
  cmp_f2_textarea_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_textarea_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Note: c-multiplatform may not have a dedicated generic textarea right now,
   * use editable */
  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_textarea_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_textarea_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_textarea_create: OOM\n");
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
 * @brief cmp_f2_textarea_set_size
 *
 * @param node Parameter description.
 * @param size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_textarea_set_size(cmp_ui_node_t *node,
                                     cmp_f2_text_input_size_t size) {
  int rc;
  rc = 0;
  cmp_f2_textarea_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_textarea_set_size: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_textarea_t *)node->properties;
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
 * @brief cmp_f2_textarea_set_state
 *
 * @param node Parameter description.
 * @param state Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_textarea_set_state(cmp_ui_node_t *node,
                                      cmp_f2_text_input_state_t state) {
  int rc;
  rc = 0;
  cmp_f2_textarea_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_textarea_set_state: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_textarea_t *)node->properties;
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

int CMP_API cmp_f2_textarea_set_resize_mode(
    cmp_ui_node_t *node, cmp_f2_textarea_resize_t resize_mode) {
  int rc;
  rc = 0;
  cmp_f2_textarea_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_textarea_set_resize_mode: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_textarea_t *)node->properties;
  data->resize_mode = resize_mode;
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
 * @brief cmp_f2_textarea_set_auto_grow
 *
 * @param node Parameter description.
 * @param auto_grow Parameter description.
 * @param max_height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_textarea_set_auto_grow(cmp_ui_node_t *node, int auto_grow,
                                          float max_height) {
  int rc;
  rc = 0;
  cmp_f2_textarea_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_textarea_set_auto_grow: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_textarea_t *)node->properties;
  data->auto_grow = auto_grow ? 1 : 0;
  data->max_height = max_height;
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
 * @brief cmp_f2_field_create
 *
 * @param out_node Parameter description.
 * @param input_node Parameter description.
 * @param label Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_field_create(cmp_ui_node_t **out_node,
                                cmp_ui_node_t *input_node, const char *label) {
  int rc;
  rc = 0;
  cmp_f2_field_t *data;
  int res;

  if (!out_node || !input_node) {
    LOG_DEBUG("cmp_f2_field_create: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_field_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_field_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_field_create: OOM\n");
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
      res = cmp_ui_node_add_child(*out_node, data->label_node);
      if (res != CMP_SUCCESS)
        LOG_DEBUG("cmp_f2_field_create: cmp_ui_node_add_child failed\n");
    } else {
      LOG_DEBUG("cmp_f2_field_create: cmp_ui_text_create failed\n");
    }
  }

  res = cmp_ui_node_add_child(*out_node, input_node);
  if (res != CMP_SUCCESS)
    LOG_DEBUG("cmp_f2_field_create: cmp_ui_node_add_child (input) failed\n");

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
 * @brief cmp_f2_field_set_label_position
 *
 * @param node Parameter description.
 * @param pos Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_field_set_label_position(cmp_ui_node_t *node,
                                            cmp_f2_field_label_position_t pos) {
  int rc;
  rc = 0;
  cmp_f2_field_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_field_set_label_position: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_field_t *)node->properties;
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
 * @brief cmp_f2_field_set_required
 *
 * @param node Parameter description.
 * @param is_required Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_field_set_required(cmp_ui_node_t *node, int is_required) {
  int rc;
  rc = 0;
  cmp_f2_field_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_field_set_required: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_field_t *)node->properties;
  data->is_required = is_required ? 1 : 0;
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
 * @brief cmp_f2_field_set_help_text
 *
 * @param node Parameter description.
 * @param help_text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_field_set_help_text(cmp_ui_node_t *node,
                                       const char *help_text) {
  int rc;
  rc = 0;
  cmp_f2_field_t *data;
  int res;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_field_set_help_text: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_field_t *)node->properties;

  if (data->help_text_node) {
    /* If changing text, map it natively. Here we mock it logicially */
  } else if (help_text) {
    res = cmp_ui_text_create(&data->help_text_node, help_text, -1);
    if (res == CMP_SUCCESS && data->help_text_node) {
      res = cmp_ui_node_add_child(node, data->help_text_node);
      if (res != CMP_SUCCESS)
        LOG_DEBUG("cmp_f2_field_set_help_text: cmp_ui_node_add_child failed\n");
    } else {
      LOG_DEBUG("cmp_f2_field_set_help_text: cmp_ui_text_create failed\n");
    }
  }
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

int CMP_API cmp_f2_field_set_validation_message(
    cmp_ui_node_t *node, const char *message, cmp_f2_text_input_state_t state) {
  int rc;
  rc = 0;
  cmp_f2_field_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_field_set_validation_message: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_field_t *)node->properties;
  data->validation_state = state;
  /* Map message to child node similar to help_text */
  (void)message;
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
 * @brief cmp_f2_searchbox_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_searchbox_create(cmp_ui_node_t **out_node) {
  int rc;
  rc = 0;
  cmp_f2_searchbox_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_searchbox_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_text_input_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_searchbox_create: cmp_ui_text_input_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_searchbox_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_searchbox_create: OOM\n");
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->size = CMP_F2_TEXT_INPUT_SIZE_MEDIUM;
  data->state = CMP_F2_TEXT_INPUT_STATE_REST;
  data->is_collapsed = 0;

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
 * @brief cmp_f2_searchbox_set_size
 *
 * @param node Parameter description.
 * @param size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_searchbox_set_size(cmp_ui_node_t *node,
                                      cmp_f2_text_input_size_t size) {
  int rc;
  rc = 0;
  cmp_f2_searchbox_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_searchbox_set_size: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_searchbox_t *)node->properties;
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
 * @brief cmp_f2_searchbox_set_state
 *
 * @param node Parameter description.
 * @param state Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_searchbox_set_state(cmp_ui_node_t *node,
                                       cmp_f2_text_input_state_t state) {
  int rc;
  rc = 0;
  cmp_f2_searchbox_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_searchbox_set_state: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_searchbox_t *)node->properties;
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
 * @brief cmp_f2_searchbox_set_collapsed_mode
 *
 * @param node Parameter description.
 * @param is_collapsed Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_searchbox_set_collapsed_mode(cmp_ui_node_t *node,
                                                int is_collapsed) {
  int rc;
  rc = 0;
  cmp_f2_searchbox_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_searchbox_set_collapsed_mode: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_searchbox_t *)node->properties;
  data->is_collapsed = is_collapsed ? 1 : 0;
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