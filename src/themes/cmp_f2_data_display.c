/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_data_display.h"
#include <stddef.h>
#include <stdlib.h>
/* clang-format on */

CMP_API int cmp_f2_avatar_create(cmp_ui_node_t **out_node, cmp_f2_avatar_size_t size,
                         cmp_f2_avatar_shape_t shape) {
  cmp_f2_avatar_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_avatar_t *)malloc(sizeof(cmp_f2_avatar_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->size = size;
  data->shape = shape;
  data->image_node = NULL;
  data->initials_node = NULL;
  data->icon_node = NULL;

  /* Map dimension layout */
  (*out_node)->layout->width = (float)size;
  (*out_node)->layout->height = (float)size;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_avatar_set_initials(cmp_ui_node_t *node, const char *initials) {
  cmp_f2_avatar_t *data;
  int res;

  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_avatar_t *)node->properties;

  if (!data->initials_node && initials) {
    res = cmp_ui_text_create(&data->initials_node, initials, -1);
    if (res == CMP_SUCCESS) {
      cmp_ui_node_add_child(node, data->initials_node);
    }
  }

  return CMP_SUCCESS;
}

CMP_API int cmp_f2_avatar_group_create(cmp_ui_node_t **out_node,
                               cmp_f2_avatar_group_layout_t layout,
                               int max_count) {
  cmp_f2_avatar_group_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_avatar_group_t *)malloc(sizeof(cmp_f2_avatar_group_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->layout = layout;
  data->max_count = max_count;
  data->current_count = 0;
  data->overflow_node = NULL;

  /* If stacked, map negative margins structurally */
  if (layout == CMP_F2_AVATAR_GROUP_LAYOUT_STACKED) {
    (*out_node)->layout->direction = CMP_FLEX_ROW;
  }

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_presence_badge_create(cmp_ui_node_t **out_node,
                                 cmp_f2_presence_status_t status) {
  cmp_f2_presence_badge_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_presence_badge_t *)malloc(sizeof(cmp_f2_presence_badge_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->status = status;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_badge_create(cmp_ui_node_t **out_node, const char *label) {
  cmp_f2_badge_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_badge_t *)malloc(sizeof(cmp_f2_badge_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->variant = CMP_F2_BADGE_VARIANT_FILLED;
  data->shape = CMP_F2_BADGE_SHAPE_ROUNDED;
  data->text_node = NULL;

  if (label) {
    res = cmp_ui_text_create(&data->text_node, label, -1);
    if (res == CMP_SUCCESS) {
      cmp_ui_node_add_child(*out_node, data->text_node);
    }
  }

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_badge_set_variant(cmp_ui_node_t *node,
                             cmp_f2_badge_variant_t variant) {
  cmp_f2_badge_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_badge_t *)node->properties;
  data->variant = variant;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_badge_set_shape(cmp_ui_node_t *node, cmp_f2_badge_shape_t shape) {
  cmp_f2_badge_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_badge_t *)node->properties;
  data->shape = shape;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_tag_create(cmp_ui_node_t **out_node, const char *label,
                      int is_dismissible) {
  cmp_f2_tag_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_tag_t *)malloc(sizeof(cmp_f2_tag_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->is_dismissible = is_dismissible ? 1 : 0;
  data->is_selected = 0;
  data->text_node = NULL;
  data->dismiss_button_node = NULL;

  if (label) {
    res = cmp_ui_text_create(&data->text_node, label, -1);
    if (res == CMP_SUCCESS) {
      cmp_ui_node_add_child(*out_node, data->text_node);
    }
  }

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_tag_set_selected(cmp_ui_node_t *node, int is_selected) {
  cmp_f2_tag_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_tag_t *)node->properties;
  data->is_selected = is_selected ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_datagrid_create(cmp_ui_node_t **out_node) {
  cmp_f2_datagrid_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_datagrid_t *)malloc(sizeof(cmp_f2_datagrid_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->header_row_node = NULL;
  data->rows_container_node = NULL;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_datagrid_row_create(cmp_ui_node_t **out_node, int is_header) {
  int res;
  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res == CMP_SUCCESS) {
    (*out_node)->layout->direction = CMP_FLEX_ROW;
    /* Map styling distinction for header vs regular row internally */
    (void)is_header;
  }
  return res;
}

CMP_API int cmp_f2_tree_create(cmp_ui_node_t **out_node) {
  int res;
  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res == CMP_SUCCESS) {
    (*out_node)->layout->direction = CMP_FLEX_COLUMN;
  }
  return res;
}

CMP_API int cmp_f2_tree_item_create(cmp_ui_node_t **out_node, const char *label) {
  cmp_f2_tree_item_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_tree_item_t *)malloc(sizeof(cmp_f2_tree_item_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->is_expanded = 0;
  data->is_selected = 0;
  data->chevron_node = NULL;
  data->content_node = NULL;
  data->children_container_node = NULL;

  if (label) {
    res = cmp_ui_text_create(&data->content_node, label, -1);
    if (res == CMP_SUCCESS) {
      cmp_ui_node_add_child(*out_node, data->content_node);
    }
  }

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_tree_item_set_expanded(cmp_ui_node_t *node, int is_expanded) {
  cmp_f2_tree_item_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_tree_item_t *)node->properties;
  data->is_expanded = is_expanded ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_tree_item_set_selected(cmp_ui_node_t *node, int is_selected) {
  cmp_f2_tree_item_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_tree_item_t *)node->properties;
  data->is_selected = is_selected ? 1 : 0;
  return CMP_SUCCESS;
}
