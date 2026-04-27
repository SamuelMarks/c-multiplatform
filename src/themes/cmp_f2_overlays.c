/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include "themes/cmp_f2_overlays.h"
#include <stddef.h>
#include <stdlib.h>
/* clang-format on */

/**
 * @brief cmp_f2_dialog_create
 *
 * @param out_node Parameter description.
 * @param variant Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_dialog_create(cmp_ui_node_t **out_node,
                                 cmp_f2_dialog_variant_t variant) {
  int rc;
  rc = 0;
  cmp_f2_dialog_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_dialog_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_dialog_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_dialog_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_dialog_create: OOM\n");
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->variant = variant;
  data->is_open = 0;

  data->backdrop_node = NULL;
  data->container_node = NULL;
  data->title_node = NULL;
  data->body_node = NULL;
  data->footer_node = NULL;

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
 * @brief cmp_f2_dialog_set_open
 *
 * @param node Parameter description.
 * @param is_open Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_dialog_set_open(cmp_ui_node_t *node, int is_open) {
  int rc;
  rc = 0;
  cmp_f2_dialog_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_dialog_set_open: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_dialog_t *)node->properties;
  data->is_open = is_open ? 1 : 0;
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
 * @brief cmp_f2_flyout_create
 *
 * @param out_node Parameter description.
 * @param anchor Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_flyout_create(cmp_ui_node_t **out_node,
                                 cmp_ui_node_t *anchor) {
  int rc;
  rc = 0;
  cmp_f2_flyout_t *data;
  int res;

  if (!out_node || !anchor) {
    LOG_DEBUG("cmp_f2_flyout_create: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_flyout_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_flyout_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_flyout_create: OOM\n");
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->placement = CMP_F2_FLYOUT_PLACEMENT_BOTTOM;
  data->is_open = 0;
  data->anchor_node = anchor;
  data->container_node = NULL;

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
 * @brief cmp_f2_flyout_set_placement
 *
 * @param node Parameter description.
 * @param placement Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_flyout_set_placement(cmp_ui_node_t *node,
                                        cmp_f2_flyout_placement_t placement) {
  int rc;
  rc = 0;
  cmp_f2_flyout_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_flyout_set_placement: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_flyout_t *)node->properties;
  data->placement = placement;
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
 * @brief cmp_f2_flyout_set_open
 *
 * @param node Parameter description.
 * @param is_open Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_flyout_set_open(cmp_ui_node_t *node, int is_open) {
  int rc;
  rc = 0;
  cmp_f2_flyout_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_flyout_set_open: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_flyout_t *)node->properties;
  data->is_open = is_open ? 1 : 0;
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
 * @brief cmp_f2_tooltip_create
 *
 * @param out_node Parameter description.
 * @param anchor Parameter description.
 * @param label Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_tooltip_create(cmp_ui_node_t **out_node,
                                  cmp_ui_node_t *anchor, const char *label) {
  int rc;
  rc = 0;
  cmp_f2_tooltip_t *data;
  int res;

  if (!out_node || !anchor) {
    LOG_DEBUG("cmp_f2_tooltip_create: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_tooltip_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_tooltip_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_tooltip_create: OOM\n");
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->is_visible = 0;
  data->anchor_node = anchor;
  data->label_node = NULL;

  if (label) {
    res = cmp_ui_text_create(&data->label_node, label, -1);
    if (res == CMP_SUCCESS) {
      res = cmp_ui_node_add_child(*out_node, data->label_node);
      if (res != CMP_SUCCESS)
        LOG_DEBUG("cmp_f2_tooltip_create: cmp_ui_node_add_child failed\n");
    } else {
      LOG_DEBUG("cmp_f2_tooltip_create: cmp_ui_text_create failed\n");
    }
  }

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
 * @brief cmp_f2_tooltip_set_visible
 *
 * @param node Parameter description.
 * @param is_visible Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_tooltip_set_visible(cmp_ui_node_t *node, int is_visible) {
  int rc;
  rc = 0;
  cmp_f2_tooltip_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_tooltip_set_visible: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_tooltip_t *)node->properties;
  data->is_visible = is_visible ? 1 : 0;
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
 * @brief cmp_f2_toast_create
 *
 * @param out_node Parameter description.
 * @param intent Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_toast_create(cmp_ui_node_t **out_node,
                                cmp_f2_toast_intent_t intent) {
  int rc;
  rc = 0;
  cmp_f2_toast_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_toast_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_toast_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_toast_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_toast_create: OOM\n");
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->intent = intent;
  data->is_visible = 1; /* Toasts typically start visible when instantiated */

  data->icon_node = NULL;
  data->title_node = NULL;
  data->body_node = NULL;
  data->actions_node = NULL;
  data->close_button_node = NULL;

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
 * @brief cmp_f2_toast_set_visible
 *
 * @param node Parameter description.
 * @param is_visible Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_toast_set_visible(cmp_ui_node_t *node, int is_visible) {
  int rc;
  rc = 0;
  cmp_f2_toast_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_toast_set_visible: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_toast_t *)node->properties;
  data->is_visible = is_visible ? 1 : 0;
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
 * @brief cmp_f2_messagebar_create
 *
 * @param out_node Parameter description.
 * @param intent Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_messagebar_create(cmp_ui_node_t **out_node,
                                     cmp_f2_toast_intent_t intent) {
  int rc;
  rc = 0;
  cmp_f2_messagebar_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_messagebar_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_messagebar_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_messagebar_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_messagebar_create: OOM\n");
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->variant = CMP_F2_MESSAGEBAR_VARIANT_SINGLELINE;
  data->intent = intent;

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
 * @brief cmp_f2_messagebar_set_variant
 *
 * @param node Parameter description.
 * @param variant Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_messagebar_set_variant(cmp_ui_node_t *node,
                                          cmp_f2_messagebar_variant_t variant) {
  int rc;
  rc = 0;
  cmp_f2_messagebar_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_messagebar_set_variant: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_messagebar_t *)node->properties;
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
 * @brief cmp_f2_progress_bar_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_progress_bar_create(cmp_ui_node_t **out_node) {
  int rc;
  rc = 0;
  cmp_f2_progress_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_progress_bar_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_progress_bar_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_progress_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_progress_bar_create: OOM\n");
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->variant = CMP_F2_PROGRESS_VARIANT_INDETERMINATE;
  data->state = CMP_F2_PROGRESS_STATE_ACTIVE;
  data->thickness = 2.0f;
  data->value = 0.0f;

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
 * @brief cmp_f2_progress_ring_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_progress_ring_create(cmp_ui_node_t **out_node) {
  int rc;
  rc = 0; /* Shares data structure with progress bar internally for this mock */
  int res = cmp_f2_progress_bar_create(out_node);
  if (res == CMP_SUCCESS) {
    /* Adjust implicit geometry representation later in rendering */
  } else {
    LOG_DEBUG(
        "cmp_f2_progress_ring_create: cmp_f2_progress_bar_create failed\n");
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
 * @brief cmp_f2_progress_set_variant
 *
 * @param node Parameter description.
 * @param variant Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_progress_set_variant(cmp_ui_node_t *node,
                                        cmp_f2_progress_variant_t variant) {
  int rc;
  rc = 0;
  cmp_f2_progress_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_progress_set_variant: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_progress_t *)node->properties;
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
 * @brief cmp_f2_progress_set_state
 *
 * @param node Parameter description.
 * @param state Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_progress_set_state(cmp_ui_node_t *node,
                                      cmp_f2_progress_state_t state) {
  int rc;
  rc = 0;
  cmp_f2_progress_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_progress_set_state: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_progress_t *)node->properties;
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
 * @brief cmp_f2_progress_set_value
 *
 * @param node Parameter description.
 * @param value Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_progress_set_value(cmp_ui_node_t *node, float value) {
  int rc;
  rc = 0;
  cmp_f2_progress_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_progress_set_value: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_progress_t *)node->properties;
  data->value = value;
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
 * @brief cmp_f2_skeleton_create
 *
 * @param out_node Parameter description.
 * @param shape Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_skeleton_create(cmp_ui_node_t **out_node,
                                   cmp_f2_skeleton_shape_t shape) {
  int rc;
  rc = 0;
  cmp_f2_skeleton_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_skeleton_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_skeleton_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_skeleton_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_skeleton_create: OOM\n");
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->shape = shape;
  data->is_animated = 1;

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
 * @brief cmp_f2_skeleton_set_animated
 *
 * @param node Parameter description.
 * @param is_animated Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_skeleton_set_animated(cmp_ui_node_t *node, int is_animated) {
  int rc;
  rc = 0;
  cmp_f2_skeleton_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_skeleton_set_animated: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_skeleton_t *)node->properties;
  data->is_animated = is_animated ? 1 : 0;
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