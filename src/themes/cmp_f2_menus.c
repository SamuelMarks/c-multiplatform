/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_menus.h"
#include <stddef.h>
#include <stdlib.h>
/* clang-format on */

CMP_API int cmp_f2_menu_create(cmp_ui_node_t **out_node) {
  int res;
  if (!out_node)
    return CMP_ERROR_INVALID_ARG;
  res = cmp_ui_box_create(out_node);
  if (res == CMP_SUCCESS) {
    (*out_node)->layout->direction = CMP_FLEX_COLUMN;
  }
  return res;
}

CMP_API int cmp_f2_menu_item_create(cmp_ui_node_t **out_node, const char *label) {
  cmp_f2_menu_item_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_menu_item_t *)malloc(sizeof(cmp_f2_menu_item_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->is_disabled = 0;
  data->leading_icon_node = NULL;
  data->text_node = NULL;
  data->shortcut_text_node = NULL;
  data->trailing_chevron_node = NULL;

  if (label) {
    res = cmp_ui_text_create(&data->text_node, label, -1);
    if (res == CMP_SUCCESS) {
      cmp_ui_node_add_child(*out_node, data->text_node);
    }
  }

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_menu_item_set_shortcut(cmp_ui_node_t *node, const char *shortcut) {
  cmp_f2_menu_item_t *data;
  int res;

  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_menu_item_t *)node->properties;

  if (!data->shortcut_text_node && shortcut) {
    res = cmp_ui_text_create(&data->shortcut_text_node, shortcut, -1);
    if (res == CMP_SUCCESS) {
      cmp_ui_node_add_child(node, data->shortcut_text_node);
    }
  }

  return CMP_SUCCESS;
}

CMP_API int cmp_f2_menu_item_set_disabled(cmp_ui_node_t *node, int is_disabled) {
  cmp_f2_menu_item_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_menu_item_t *)node->properties;
  data->is_disabled = is_disabled ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_nav_view_create(cmp_ui_node_t **out_node) {
  cmp_f2_nav_view_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_nav_view_t *)malloc(sizeof(cmp_f2_nav_view_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->state = CMP_F2_NAV_STATE_EXPANDED;
  data->header_node = NULL;
  data->content_node = NULL;
  data->footer_node = NULL;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_nav_view_set_state(cmp_ui_node_t *node, cmp_f2_nav_state_t state) {
  cmp_f2_nav_view_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_nav_view_t *)node->properties;
  data->state = state;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_nav_item_create(cmp_ui_node_t **out_node, const char *label) {
  cmp_f2_nav_item_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_nav_item_t *)malloc(sizeof(cmp_f2_nav_item_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->is_active = 0;
  data->icon_node = NULL;
  data->label_node = NULL;
  data->active_indicator_node = NULL;

  if (label) {
    res = cmp_ui_text_create(&data->label_node, label, -1);
    if (res == CMP_SUCCESS) {
      cmp_ui_node_add_child(*out_node, data->label_node);
    }
  }

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_nav_item_set_active(cmp_ui_node_t *node, int is_active) {
  cmp_f2_nav_item_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_nav_item_t *)node->properties;
  data->is_active = is_active ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_tab_container_create(cmp_ui_node_t **out_node, int is_vertical) {
  cmp_f2_tab_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_tab_t *)malloc(sizeof(cmp_f2_tab_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->variant = CMP_F2_TAB_VARIANT_TRANSPARENT;
  data->size = CMP_F2_TAB_SIZE_MEDIUM;
  data->is_vertical = is_vertical ? 1 : 0;

  data->header_area_node = NULL;
  data->active_indicator_node = NULL;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_tab_container_set_variant(cmp_ui_node_t *node,
                                     cmp_f2_tab_variant_t variant) {
  cmp_f2_tab_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_tab_t *)node->properties;
  data->variant = variant;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_tab_container_set_size(cmp_ui_node_t *node, cmp_f2_tab_size_t size) {
  cmp_f2_tab_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_tab_t *)node->properties;
  data->size = size;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_tab_item_create(cmp_ui_node_t **out_node, const char *label) {
  /* Essentially acts as a styled UI box or button within the tab container
   * context */
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  if (label) {
    cmp_ui_node_t *label_node = NULL;
    res = cmp_ui_text_create(&label_node, label, -1);
    if (res == CMP_SUCCESS) {
      cmp_ui_node_add_child(*out_node, label_node);
    }
  }

  return CMP_SUCCESS;
}

CMP_API int cmp_f2_breadcrumb_create(cmp_ui_node_t **out_node) {
  int res;
  if (!out_node)
    return CMP_ERROR_INVALID_ARG;
  res = cmp_ui_box_create(out_node);
  if (res == CMP_SUCCESS) {
    (*out_node)->layout->direction = CMP_FLEX_ROW;
  }
  return res;
}

CMP_API int cmp_f2_breadcrumb_item_create(cmp_ui_node_t **out_node, const char *label,
                                  int is_last) {
  int res;
  cmp_ui_node_t *text_node = NULL;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  if (label) {
    res = cmp_ui_text_create(&text_node, label, -1);
    if (res == CMP_SUCCESS) {
      cmp_ui_node_add_child(*out_node, text_node);
    }
  }

  if (!is_last) {
    cmp_ui_node_t *chevron_node = NULL;
    res = cmp_ui_text_create(&chevron_node, ">", -1);
    if (res == CMP_SUCCESS) {
      cmp_ui_node_add_child(*out_node, chevron_node);
    }
  }

  return CMP_SUCCESS;
}

CMP_API int cmp_f2_toolbar_create(cmp_ui_node_t **out_node) {
  cmp_f2_toolbar_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_toolbar_t *)malloc(sizeof(cmp_f2_toolbar_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->primary_commands_node = NULL;
  data->secondary_commands_node = NULL;
  data->overflow_menu_node = NULL;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_link_create(cmp_ui_node_t **out_node, const char *label,
                       cmp_f2_link_variant_t variant) {
  cmp_f2_link_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_link_t *)malloc(sizeof(cmp_f2_link_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->variant = variant;
  data->is_disabled = 0;
  data->is_visited = 0;
  data->text_node = NULL;
  data->external_icon_node = NULL;

  if (label) {
    res = cmp_ui_text_create(&data->text_node, label, -1);
    if (res == CMP_SUCCESS) {
      cmp_ui_node_add_child(*out_node, data->text_node);
    }
  }

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_link_set_disabled(cmp_ui_node_t *node, int is_disabled) {
  cmp_f2_link_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_link_t *)node->properties;
  data->is_disabled = is_disabled ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_stepper_create(cmp_ui_node_t **out_node) {
  int res;
  if (!out_node)
    return CMP_ERROR_INVALID_ARG;
  res = cmp_ui_box_create(out_node);
  if (res == CMP_SUCCESS) {
    (*out_node)->layout->direction = CMP_FLEX_ROW;
  }
  return res;
}

CMP_API int cmp_f2_stepper_item_create(cmp_ui_node_t **out_node, const char *label) {
  cmp_f2_stepper_item_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_stepper_item_t *)malloc(sizeof(cmp_f2_stepper_item_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->state = CMP_F2_STEPPER_STATE_UPCOMING;
  data->indicator_node = NULL;
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

CMP_API int cmp_f2_stepper_item_set_state(cmp_ui_node_t *node,
                                  cmp_f2_stepper_state_t state) {
  cmp_f2_stepper_item_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_stepper_item_t *)node->properties;
  data->state = state;
  return CMP_SUCCESS;
}
