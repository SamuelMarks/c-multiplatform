/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include "themes/cmp_f2_button.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief cmp_f2_button_init_internal
 *
 * @param out_node Parameter description.
 * @param type Parameter description.
 * @param label Parameter description.
 * @param icon Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static int cmp_f2_button_init_internal(cmp_ui_node_t **out_node,
                                       cmp_f2_button_type_t type,
                                       const char *label, cmp_ui_node_t *icon) {
  int rc = CMP_SUCCESS;
  cmp_f2_button_t *btn_data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_button_init_internal: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_button_create(out_node, label, -1);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_button_init_internal: cmp_ui_button_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_button_t), (void **)&btn_data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_button_init_internal: OOM\n");
    (void)cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  btn_data->type = type;
  btn_data->variant = CMP_F2_BUTTON_VARIANT_SECONDARY;
  btn_data->size = CMP_F2_BUTTON_SIZE_MEDIUM;
  btn_data->shape = CMP_F2_BUTTON_SHAPE_ROUNDED;
  btn_data->state = CMP_F2_BUTTON_STATE_REST;
  btn_data->is_toggled = 0;
  btn_data->is_menu_open = 0;

  btn_data->icon_node = icon;
  btn_data->text_node =
      NULL; /* Usually initialized inside cmp_ui_button_create */
  btn_data->secondary_text_node = NULL;
  btn_data->chevron_node = NULL;
  btn_data->divider_node = NULL;

  if (icon) {
    if (label) {
      btn_data->layout_mode = CMP_F2_BUTTON_LAYOUT_ICON_TEXT;
    } else {
      btn_data->layout_mode = CMP_F2_BUTTON_LAYOUT_ICON_ONLY;
    }
  } else {
    btn_data->layout_mode = CMP_F2_BUTTON_LAYOUT_TEXT_ONLY;
  }

  (*out_node)->properties = (void *)btn_data;

  /* Apply defaults */
  res = cmp_f2_button_set_size(*out_node, CMP_F2_BUTTON_SIZE_MEDIUM);
  if (res != CMP_SUCCESS)
    LOG_DEBUG("cmp_f2_button_init_internal: cmp_f2_button_set_size failed\n");
  res = cmp_f2_button_set_shape(*out_node, CMP_F2_BUTTON_SHAPE_ROUNDED);
  if (res != CMP_SUCCESS)
    LOG_DEBUG("cmp_f2_button_init_internal: cmp_f2_button_set_shape failed\n");

  return rc;
}

/**
 * @brief cmp_f2_button_create
 *
 * @param out_node Parameter description.
 * @param label Parameter description.
 * @param icon Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_button_create(cmp_ui_node_t **out_node, const char *label,
                                 cmp_ui_node_t *icon) {
  int rc;
  rc = 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return cmp_f2_button_init_internal(out_node, CMP_F2_BUTTON_TYPE_STANDARD,
                                     label, icon);
}

/**
 * @brief cmp_f2_compound_button_create
 *
 * @param out_node Parameter description.
 * @param primary_text Parameter description.
 * @param secondary_text Parameter description.
 * @param icon Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_compound_button_create(cmp_ui_node_t **out_node,
                                          const char *primary_text,
                                          const char *secondary_text,
                                          cmp_ui_node_t *icon) {
  int rc = 0;
  int res;
  cmp_f2_button_t *btn_data;

  res = cmp_f2_button_init_internal(out_node, CMP_F2_BUTTON_TYPE_COMPOUND,
                                    primary_text, icon);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_compound_button_create: init_internal failed\n");
    return res;
  }

  btn_data = (cmp_f2_button_t *)(*out_node)->properties;

  if (secondary_text) {
    /* MOCK: create a secondary text node representing descriptive text */
    res =
        cmp_ui_text_create(&btn_data->secondary_text_node, secondary_text, -1);
    if (res == CMP_SUCCESS) {
      res = cmp_ui_node_add_child(*out_node, btn_data->secondary_text_node);
      if (res != CMP_SUCCESS)
        LOG_DEBUG(
            "cmp_f2_compound_button_create: cmp_ui_node_add_child failed\n");
    } else {
      LOG_DEBUG("cmp_f2_compound_button_create: cmp_ui_text_create failed\n");
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

/**
 * @brief cmp_f2_menu_button_create
 *
 * @param out_node Parameter description.
 * @param label Parameter description.
 * @param icon Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_menu_button_create(cmp_ui_node_t **out_node,
                                      const char *label, cmp_ui_node_t *icon) {
  int rc = 0;
  int res;
  cmp_f2_button_t *btn_data;

  res = cmp_f2_button_init_internal(out_node, CMP_F2_BUTTON_TYPE_MENU, label,
                                    icon);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_menu_button_create: init_internal failed\n");
    return res;
  }

  btn_data = (cmp_f2_button_t *)(*out_node)->properties;

  /* MOCK: create trailing chevron icon */
  res = cmp_ui_text_create(&btn_data->chevron_node, "V",
                           -1); /* Fake Chevron down */
  if (res == CMP_SUCCESS) {
    res = cmp_ui_node_add_child(*out_node, btn_data->chevron_node);
    if (res != CMP_SUCCESS)
      LOG_DEBUG("cmp_f2_menu_button_create: cmp_ui_node_add_child failed\n");
  } else {
    LOG_DEBUG("cmp_f2_menu_button_create: cmp_ui_text_create failed\n");
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

/**
 * @brief cmp_f2_split_button_create
 *
 * @param out_node Parameter description.
 * @param label Parameter description.
 * @param icon Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_split_button_create(cmp_ui_node_t **out_node,
                                       const char *label, cmp_ui_node_t *icon) {
  int rc = 0;
  int res;
  cmp_f2_button_t *btn_data;

  res = cmp_f2_button_init_internal(out_node, CMP_F2_BUTTON_TYPE_SPLIT, label,
                                    icon);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_split_button_create: init_internal failed\n");
    return res;
  }

  btn_data = (cmp_f2_button_t *)(*out_node)->properties;

  /* MOCK: create divider and chevron */
  res = cmp_ui_box_create(&btn_data->divider_node);
  if (res == CMP_SUCCESS) {
    btn_data->divider_node->layout->width = 1.0f; /* 1px split line */
    res = cmp_ui_node_add_child(*out_node, btn_data->divider_node);
    if (res != CMP_SUCCESS)
      LOG_DEBUG("cmp_f2_split_button_create: cmp_ui_node_add_child (divider) "
                "failed\n");
  } else {
    LOG_DEBUG("cmp_f2_split_button_create: cmp_ui_box_create failed\n");
  }

  res = cmp_ui_text_create(&btn_data->chevron_node, "V", -1); /* Fake Chevron */
  if (res == CMP_SUCCESS) {
    res = cmp_ui_node_add_child(*out_node, btn_data->chevron_node);
    if (res != CMP_SUCCESS)
      LOG_DEBUG("cmp_f2_split_button_create: cmp_ui_node_add_child (chevron) "
                "failed\n");
  } else {
    LOG_DEBUG("cmp_f2_split_button_create: cmp_ui_text_create failed\n");
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

/**
 * @brief cmp_f2_toggle_button_create
 *
 * @param out_node Parameter description.
 * @param label Parameter description.
 * @param icon Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_toggle_button_create(cmp_ui_node_t **out_node,
                                        const char *label,
                                        cmp_ui_node_t *icon) {
  int rc;
  rc = 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return cmp_f2_button_init_internal(out_node, CMP_F2_BUTTON_TYPE_TOGGLE, label,
                                     icon);
}

/**
 * @brief cmp_f2_button_set_variant
 *
 * @param node Parameter description.
 * @param variant Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_button_set_variant(cmp_ui_node_t *node,
                                      cmp_f2_button_variant_t variant) {
  int rc = 0;
  cmp_f2_button_t *btn_data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_button_set_variant: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  btn_data = (cmp_f2_button_t *)node->properties;

  btn_data->variant = variant;

  {
    cmp_f2_theme_t theme;
    rc = cmp_f2_theme_generate(CMP_F2_COLOR_BLUE, 0, &theme);
    if (rc == CMP_SUCCESS) {
      if (node->child_count > 0 && node->children[0]) {
        node->children[0]->font_size = theme.font_size_body_1;
        node->children[0]->layout->box_sizing = CMP_BOX_SIZING_BORDER_BOX;
        node->children[0]->layout->height = 20.0f;
        node->children[0]->layout->flex_shrink = 0.0f;
        node->children[0]->layout->flex_grow = 1.0f;
      }

      node->hover_opacity = 0.04f;
      node->press_opacity = 0.08f;

      switch (variant) {

      case CMP_F2_BUTTON_VARIANT_SECONDARY:
        node->bg_color = theme.color_neutral_background_1
                             ? (0xFF000000 | theme.color_neutral_background_1)
                             : 0;
        node->border_radius = theme.radius_medium;
        node->border_color = theme.color_neutral_stroke_1
                                 ? (0xFF000000 | theme.color_neutral_stroke_1)
                                 : 0;
        node->border_width = theme.stroke_width_thin;
        if (node->child_count > 0 && node->children[0]) {
          node->children[0]->text_color =
              0xFF000000 | theme.color_neutral_foreground_1;
        }
        break;
      case CMP_F2_BUTTON_VARIANT_PRIMARY:
        node->bg_color = theme.color_brand_background
                             ? (0xFF000000 | theme.color_brand_background)
                             : 0;
        node->border_radius = theme.radius_medium;
        node->border_color = theme.color_transparent_background;
        node->border_width = 0.0f;
        if (node->child_count > 0 && node->children[0]) {
          node->children[0]->text_color =
              0xFF000000 | theme.color_neutral_foreground_static_inverted;
        }
        break;
      case CMP_F2_BUTTON_VARIANT_SUBTLE:
        node->bg_color = theme.color_transparent_background;
        node->border_radius = theme.radius_medium;
        node->border_width = 0.0f;
        if (node->child_count > 0 && node->children[0]) {
          node->children[0]->text_color =
              0xFF000000 | theme.color_neutral_foreground_1;
        }
        break;
      case CMP_F2_BUTTON_VARIANT_OUTLINE:
        node->bg_color = theme.color_transparent_background;
        node->border_width = theme.stroke_width_thin;
        node->border_color = theme.color_neutral_stroke_1
                                 ? (0xFF000000 | theme.color_neutral_stroke_1)
                                 : 0;
        node->border_radius = theme.radius_medium;
        if (node->child_count > 0 && node->children[0]) {
          node->children[0]->text_color =
              0xFF000000 | theme.color_neutral_foreground_1;
        }
        break;
      case CMP_F2_BUTTON_VARIANT_TRANSPARENT:
        node->bg_color = theme.color_transparent_background;
        node->border_width = 0.0f;
        node->border_radius = theme.radius_medium;
        if (node->child_count > 0 && node->children[0]) {
          node->children[0]->text_color =
              0xFF000000 | theme.color_neutral_foreground_1;
        }
        break;
      }
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

/**
 * @brief cmp_f2_button_set_size
 *
 * @param node Parameter description.
 * @param size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_button_set_size(cmp_ui_node_t *node,
                                   cmp_f2_button_size_t size) {
  int rc = 0;
  cmp_f2_button_t *btn_data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  btn_data = (cmp_f2_button_t *)node->properties;
  btn_data->size = size;

  if (size == CMP_F2_BUTTON_SIZE_SMALL) {
    node->layout->box_sizing = CMP_BOX_SIZING_BORDER_BOX;
    node->layout->box_sizing = CMP_BOX_SIZING_BORDER_BOX;
    node->layout->height = 24.0f;
    node->layout->padding[0] = 8.0f;
    node->layout->padding[1] = 2.0f;
    node->layout->padding[2] = 8.0f;
    node->layout->padding[3] = 2.0f;
  } else if (size == CMP_F2_BUTTON_SIZE_LARGE) {
    node->layout->box_sizing = CMP_BOX_SIZING_BORDER_BOX;
    node->layout->box_sizing = CMP_BOX_SIZING_BORDER_BOX;
    node->layout->height = 40.0f;
    node->layout->padding[0] = 20.0f;
    node->layout->padding[1] = 10.0f;
    node->layout->padding[2] = 20.0f;
    node->layout->padding[3] = 10.0f;
  } else {
    node->layout->box_sizing = CMP_BOX_SIZING_BORDER_BOX;
    node->layout->box_sizing = CMP_BOX_SIZING_BORDER_BOX;
    node->layout->height = 32.0f;
    node->layout->padding[0] = 16.0f;
    node->layout->padding[1] = 6.0f;
    node->layout->padding[2] = 16.0f;
    node->layout->padding[3] = 6.0f;
  }
  {
    float content_width = 64.0f;
    if (node->child_count > 0 && node->children[0] &&
        node->children[0]->properties) {
      size_t len = strlen((const char *)node->children[0]->properties);
      float font_size = node->children[0]->font_size > 0
                            ? node->children[0]->font_size
                            : 14.0f;
      content_width = (float)len * (font_size * 0.5f);
    }

    if (size == CMP_F2_BUTTON_SIZE_SMALL) {
      node->layout->width = content_width + 16.0f;
    } else if (size == CMP_F2_BUTTON_SIZE_LARGE) {
      node->layout->width = content_width + 40.0f;
    } else {
      node->layout->width = content_width + 32.0f;
    }
  }

  return rc;
}

/**
 * @brief cmp_f2_button_set_shape
 *
 * @param node Parameter description.
 * @param shape Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_button_set_shape(cmp_ui_node_t *node,
                                    cmp_f2_button_shape_t shape) {
  int rc = 0;
  cmp_f2_button_t *btn_data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_button_set_shape: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  btn_data = (cmp_f2_button_t *)node->properties;
  btn_data->shape = shape;
  /* We will interpret this dynamically during draw */
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
 * @brief cmp_f2_button_set_state
 *
 * @param node Parameter description.
 * @param state Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_button_set_state(cmp_ui_node_t *node,
                                    cmp_f2_button_state_t state) {
  int rc = 0;
  cmp_f2_button_t *btn_data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_button_set_state: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  btn_data = (cmp_f2_button_t *)node->properties;
  btn_data->state = state;
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
 * @brief cmp_f2_button_set_toggled
 *
 * @param node Parameter description.
 * @param is_toggled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_button_set_toggled(cmp_ui_node_t *node, int is_toggled) {
  int rc = 0;
  cmp_f2_button_t *btn_data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_button_set_toggled: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  btn_data = (cmp_f2_button_t *)node->properties;
  if (btn_data->type != CMP_F2_BUTTON_TYPE_TOGGLE) {
    LOG_DEBUG("cmp_f2_button_set_toggled: Not a toggle button\n");
    return CMP_ERROR_INVALID_STATE;
  }
  btn_data->is_toggled = is_toggled ? 1 : 0;
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
 * @brief cmp_f2_button_set_menu_open
 *
 * @param node Parameter description.
 * @param is_open Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_button_set_menu_open(cmp_ui_node_t *node, int is_open) {
  int rc = 0;
  cmp_f2_button_t *btn_data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_button_set_menu_open: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  btn_data = (cmp_f2_button_t *)node->properties;

  if (btn_data->type != CMP_F2_BUTTON_TYPE_MENU &&
      btn_data->type != CMP_F2_BUTTON_TYPE_SPLIT) {
    LOG_DEBUG("cmp_f2_button_set_menu_open: Not a menu/split button\n");
    return CMP_ERROR_INVALID_STATE;
  }

  btn_data->is_menu_open = is_open ? 1 : 0;

  /* In an actual implementation, this rotates the chevron icon node 180 degrees
   * via `style.transform` */

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
