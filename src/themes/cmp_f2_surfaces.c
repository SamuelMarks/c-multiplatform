/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include "themes/cmp_f2_surfaces.h"
#include <stddef.h>
#include <stdlib.h>
/* clang-format on */

/**
 * @brief cmp_f2_card_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_API int cmp_f2_card_create(cmp_ui_node_t **out_node) {
  cmp_f2_card_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_card_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_card_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_card_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_card_create: OOM\n");
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->variant = CMP_F2_CARD_VARIANT_FILLED;
  data->orientation = CMP_F2_CARD_ORIENTATION_VERTICAL;
  data->is_clickable = 0;

  data->header_node = NULL;
  data->preview_node = NULL;
  data->body_node = NULL;
  data->footer_node = NULL;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_f2_card_set_variant
 *
 * @param node Parameter description.
 * @param variant Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_API int cmp_f2_card_set_variant(cmp_ui_node_t *node,
                                    cmp_f2_card_variant_t variant) {
  cmp_f2_card_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_card_set_variant: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_card_t *)node->properties;
  data->variant = variant;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_f2_card_set_orientation
 *
 * @param node Parameter description.
 * @param orientation Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_API int cmp_f2_card_set_orientation(cmp_ui_node_t *node,
                                        cmp_f2_card_orientation_t orientation) {
  cmp_f2_card_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_card_set_orientation: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_card_t *)node->properties;
  data->orientation = orientation;

  if (orientation == CMP_F2_CARD_ORIENTATION_HORIZONTAL) {
    node->layout->direction = CMP_FLEX_ROW;
  } else {
    node->layout->direction = CMP_FLEX_COLUMN;
  }

  return CMP_SUCCESS;
}

/**
 * @brief cmp_f2_card_set_clickable
 *
 * @param node Parameter description.
 * @param is_clickable Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_API int cmp_f2_card_set_clickable(cmp_ui_node_t *node, int is_clickable) {
  cmp_f2_card_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_card_set_clickable: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_card_t *)node->properties;
  data->is_clickable = is_clickable ? 1 : 0;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_f2_accordion_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_API int cmp_f2_accordion_create(cmp_ui_node_t **out_node) {
  cmp_f2_accordion_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_accordion_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_accordion_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_accordion_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_accordion_create: OOM\n");
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->variant = CMP_F2_ACCORDION_VARIANT_FLUSH;
  data->multi_expand = 0;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_f2_accordion_set_variant
 *
 * @param node Parameter description.
 * @param variant Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_API int cmp_f2_accordion_set_variant(cmp_ui_node_t *node,
                                         cmp_f2_accordion_variant_t variant) {
  cmp_f2_accordion_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_accordion_set_variant: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_accordion_t *)node->properties;
  data->variant = variant;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_f2_accordion_set_multi_expand
 *
 * @param node Parameter description.
 * @param multi_expand Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_API int cmp_f2_accordion_set_multi_expand(cmp_ui_node_t *node,
                                              int multi_expand) {
  cmp_f2_accordion_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_accordion_set_multi_expand: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_accordion_t *)node->properties;
  data->multi_expand = multi_expand ? 1 : 0;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_f2_accordion_panel_create
 *
 * @param out_node Parameter description.
 * @param header_label Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_API int cmp_f2_accordion_panel_create(cmp_ui_node_t **out_node,
                                          const char *header_label) {
  cmp_f2_accordion_panel_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_accordion_panel_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_accordion_panel_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_accordion_panel_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_accordion_panel_create: OOM\n");
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->is_expanded = 0;
  data->header_node = NULL;
  data->chevron_node = NULL;
  data->content_body_node = NULL;

  if (header_label) {
    res = cmp_ui_text_create(&data->header_node, header_label, -1);
    if (res == CMP_SUCCESS) {
      res = cmp_ui_node_add_child(*out_node, data->header_node);
      if (res != CMP_SUCCESS)
        LOG_DEBUG(
            "cmp_f2_accordion_panel_create: cmp_ui_node_add_child failed\n");
    } else {
      LOG_DEBUG("cmp_f2_accordion_panel_create: cmp_ui_text_create failed\n");
    }
  }

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_f2_accordion_panel_set_expanded
 *
 * @param node Parameter description.
 * @param is_expanded Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_API int cmp_f2_accordion_panel_set_expanded(cmp_ui_node_t *node,
                                                int is_expanded) {
  cmp_f2_accordion_panel_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_accordion_panel_set_expanded: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_accordion_panel_t *)node->properties;
  data->is_expanded = is_expanded ? 1 : 0;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_f2_divider_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_API int cmp_f2_divider_create(cmp_ui_node_t **out_node) {
  cmp_f2_divider_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_divider_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_divider_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_divider_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_divider_create: OOM\n");
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->orientation = CMP_F2_DIVIDER_HORIZONTAL;
  data->appearance = CMP_F2_DIVIDER_APPEARANCE_DEFAULT;
  data->is_inset = 0;

  (*out_node)->layout->height =
      1.0f; /* Base width mapped to horizontal initially */

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int
cmp_f2_divider_set_orientation(cmp_ui_node_t *node,
                               cmp_f2_divider_orientation_t orientation) {
  cmp_f2_divider_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_divider_set_orientation: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_divider_t *)node->properties;
  data->orientation = orientation;

  if (orientation == CMP_F2_DIVIDER_HORIZONTAL) {
    node->layout->height = 1.0f;
    /* Reset width if needed, usually mapped dynamically via flex */
  } else {
    node->layout->width = 1.0f;
  }

  return CMP_SUCCESS;
}

CMP_API int
cmp_f2_divider_set_appearance(cmp_ui_node_t *node,
                              cmp_f2_divider_appearance_t appearance) {
  cmp_f2_divider_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_divider_set_appearance: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_divider_t *)node->properties;
  data->appearance = appearance;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_f2_divider_set_inset
 *
 * @param node Parameter description.
 * @param is_inset Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_API int cmp_f2_divider_set_inset(cmp_ui_node_t *node, int is_inset) {
  cmp_f2_divider_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_divider_set_inset: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_divider_t *)node->properties;
  data->is_inset = is_inset ? 1 : 0;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_f2_split_view_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_API int cmp_f2_split_view_create(cmp_ui_node_t **out_node) {
  cmp_f2_split_view_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_split_view_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_split_view_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_split_view_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_split_view_create: OOM\n");
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->pane_1_node = NULL;
  data->resizer_node = NULL;
  data->pane_2_node = NULL;

  /* Configure horizontal split flow base */
  (*out_node)->layout->direction = CMP_FLEX_ROW;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_f2_scroll_view_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_API int cmp_f2_scroll_view_create(cmp_ui_node_t **out_node) {
  cmp_f2_scroll_view_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_scroll_view_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Map the generic scrolling bounds implementation via box flow rules */
  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_scroll_view_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_scroll_view_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_scroll_view_create: OOM\n");
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->behavior = CMP_F2_SCROLLBAR_BEHAVIOR_AUTO_HIDE;
  data->is_overlay = 1;

  data->content_wrapper_node = NULL;
  data->vertical_scrollbar_node = NULL;
  data->horizontal_scrollbar_node = NULL;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int
cmp_f2_scroll_view_set_behavior(cmp_ui_node_t *node,
                                cmp_f2_scrollbar_behavior_t behavior) {
  cmp_f2_scroll_view_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_scroll_view_set_behavior: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_scroll_view_t *)node->properties;
  data->behavior = behavior;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_f2_scroll_view_set_overlay_mode
 *
 * @param node Parameter description.
 * @param is_overlay Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_API int cmp_f2_scroll_view_set_overlay_mode(cmp_ui_node_t *node,
                                                int is_overlay) {
  cmp_f2_scroll_view_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_scroll_view_set_overlay_mode: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_scroll_view_t *)node->properties;
  data->is_overlay = is_overlay ? 1 : 0;
  return CMP_SUCCESS;
}