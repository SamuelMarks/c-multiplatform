/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_overlays.h"
#include <stddef.h>
#include <stdlib.h>
/* clang-format on */

CMP_API int cmp_f2_dialog_create(cmp_ui_node_t **out_node,
                                 cmp_f2_dialog_variant_t variant) {
  cmp_f2_dialog_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_dialog_t *)malloc(sizeof(cmp_f2_dialog_t));
  if (!data) {
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
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_dialog_set_open(cmp_ui_node_t *node, int is_open) {
  cmp_f2_dialog_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_dialog_t *)node->properties;
  data->is_open = is_open ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_flyout_create(cmp_ui_node_t **out_node,
                                 cmp_ui_node_t *anchor) {
  cmp_f2_flyout_t *data;
  int res;

  if (!out_node || !anchor)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_flyout_t *)malloc(sizeof(cmp_f2_flyout_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->placement = CMP_F2_FLYOUT_PLACEMENT_BOTTOM;
  data->is_open = 0;
  data->anchor_node = anchor;
  data->container_node = NULL;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_flyout_set_placement(cmp_ui_node_t *node,
                                        cmp_f2_flyout_placement_t placement) {
  cmp_f2_flyout_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_flyout_t *)node->properties;
  data->placement = placement;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_flyout_set_open(cmp_ui_node_t *node, int is_open) {
  cmp_f2_flyout_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_flyout_t *)node->properties;
  data->is_open = is_open ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_tooltip_create(cmp_ui_node_t **out_node,
                                  cmp_ui_node_t *anchor, const char *label) {
  cmp_f2_tooltip_t *data;
  int res;

  if (!out_node || !anchor)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_tooltip_t *)malloc(sizeof(cmp_f2_tooltip_t));
  if (!data) {
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
      cmp_ui_node_add_child(*out_node, data->label_node);
    }
  }

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_tooltip_set_visible(cmp_ui_node_t *node, int is_visible) {
  cmp_f2_tooltip_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_tooltip_t *)node->properties;
  data->is_visible = is_visible ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_toast_create(cmp_ui_node_t **out_node,
                                cmp_f2_toast_intent_t intent) {
  cmp_f2_toast_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_toast_t *)malloc(sizeof(cmp_f2_toast_t));
  if (!data) {
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
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_toast_set_visible(cmp_ui_node_t *node, int is_visible) {
  cmp_f2_toast_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_toast_t *)node->properties;
  data->is_visible = is_visible ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_messagebar_create(cmp_ui_node_t **out_node,
                                     cmp_f2_toast_intent_t intent) {
  cmp_f2_messagebar_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_messagebar_t *)malloc(sizeof(cmp_f2_messagebar_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->variant = CMP_F2_MESSAGEBAR_VARIANT_SINGLELINE;
  data->intent = intent;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_messagebar_set_variant(cmp_ui_node_t *node,
                                          cmp_f2_messagebar_variant_t variant) {
  cmp_f2_messagebar_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_messagebar_t *)node->properties;
  data->variant = variant;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_progress_bar_create(cmp_ui_node_t **out_node) {
  cmp_f2_progress_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_progress_t *)malloc(sizeof(cmp_f2_progress_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->variant = CMP_F2_PROGRESS_VARIANT_INDETERMINATE;
  data->state = CMP_F2_PROGRESS_STATE_ACTIVE;
  data->thickness = 2.0f;
  data->value = 0.0f;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_progress_ring_create(cmp_ui_node_t **out_node) {
  /* Shares data structure with progress bar internally for this mock */
  int res = cmp_f2_progress_bar_create(out_node);
  if (res == CMP_SUCCESS) {
    /* Adjust implicit geometry representation later in rendering */
  }
  return res;
}

CMP_API int cmp_f2_progress_set_variant(cmp_ui_node_t *node,
                                        cmp_f2_progress_variant_t variant) {
  cmp_f2_progress_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_progress_t *)node->properties;
  data->variant = variant;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_progress_set_state(cmp_ui_node_t *node,
                                      cmp_f2_progress_state_t state) {
  cmp_f2_progress_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_progress_t *)node->properties;
  data->state = state;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_progress_set_value(cmp_ui_node_t *node, float value) {
  cmp_f2_progress_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_progress_t *)node->properties;
  data->value = value;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_skeleton_create(cmp_ui_node_t **out_node,
                                   cmp_f2_skeleton_shape_t shape) {
  cmp_f2_skeleton_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_skeleton_t *)malloc(sizeof(cmp_f2_skeleton_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->shape = shape;
  data->is_animated = 1;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_skeleton_set_animated(cmp_ui_node_t *node, int is_animated) {
  cmp_f2_skeleton_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_skeleton_t *)node->properties;
  data->is_animated = is_animated ? 1 : 0;
  return CMP_SUCCESS;
}
