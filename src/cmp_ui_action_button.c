/* clang-format off */
#include "cmp_ui_action_button.h"
#include "themes/cmp_material3_sys.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/* Action Button Metrics */
#define CMP_UI_ACTION_BUTTON_HEIGHT 40.0f
#define CMP_UI_ACTION_BUTTON_PADDING_H 24.0f
#define CMP_UI_ACTION_BUTTON_PADDING_V 10.0f
#define CMP_UI_ACTION_BUTTON_PADDING_TEXT_BTN_H 12.0f
#define CMP_UI_ACTION_BUTTON_BORDER_RADIUS 20.0f
#define CMP_UI_ACTION_BUTTON_FONT_SIZE 14.0f
#define CMP_UI_ACTION_BUTTON_TEXT_HEIGHT 20.0f
#define CMP_UI_ACTION_BUTTON_MIN_WIDTH 64.0f
#define CMP_UI_ACTION_BUTTON_HOVER_OPACITY 0.08f
#define CMP_UI_ACTION_BUTTON_PRESS_OPACITY 0.10f
#define CMP_UI_ACTION_BUTTON_ELEVATION CMP_MATH_ONE
#define CMP_UI_ACTION_BUTTON_BORDER_WIDTH CMP_MATH_ONE

/**
 * @brief Opaque internal structure for UI Action Button widget.
 */
struct cmp_ui_action_button {
  /** @brief The root button node */
  cmp_ui_node_t *node_root;
  /** @brief The text node containing the button label */
  cmp_ui_node_t *node_text;
  /** @brief The raw string of the label */
  char *label;
  /** @brief The visual style of the button */
  cmp_ui_action_button_style_t style;
};

/**
 * @brief cmp_ui_action_button_create
 *
 * @param out_btn Pointer to output the newly created button.
 * @param label The text label of the button.
 * @param style The visual style.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_action_button_create(cmp_ui_action_button_t **out_btn,
                                const char *label,
                                cmp_ui_action_button_style_t style) {
  cmp_ui_action_button_t *btn;
  int rc = CMP_SUCCESS;
  size_t len;

  if (!out_btn) {
    LOG_DEBUG("cmp_ui_action_button_create: out_btn is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_action_button_t), (void **)&btn);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_action_button_create: OOM\n");

    return rc;
  }
  memset(btn, 0, sizeof(cmp_ui_action_button_t));

  btn->style = style;

  if (label) {
    len = strlen(label);
    rc = CMP_MALLOC(len + 1, (void **)&btn->label);
    if (rc == CMP_SUCCESS) {
      memcpy(btn->label, label, len + 1);
    } else {
      LOG_DEBUG("cmp_ui_action_button_create: OOM label\n");
      {

        int free_rc_1 = CMP_FREE(btn);

        if (free_rc_1 != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_action_button_create: CMP_FREE failed\n");
        }
      }
      return CMP_ERROR_OOM;
    }
  }

  rc = cmp_ui_button_create(&btn->node_root, "", 0);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_action_button_create: cmp_ui_button_create failed\n");
    if (btn->label) {
      {

        int free_rc_2 = CMP_FREE(btn->label);

        if (free_rc_2 != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_action_button_create: CMP_FREE label failed\n");
        }
      }
    }
    {

      int free_rc_3 = CMP_FREE(btn);

      if (free_rc_3 != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_action_button_create: CMP_FREE failed\n");
      }
    }

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_layout_node_t), (void **)&btn->node_root->layout);
  if (rc != CMP_SUCCESS) {
    cmp_ui_action_button_destroy(btn);
    return rc;
  }
    memset(btn->node_root->layout, 0, sizeof(cmp_layout_node_t));
    btn->node_root->layout->id = 1;

  btn->node_root->type = CMP_UI_NODE_TYPE_BUTTON;

  rc = cmp_ui_text_create(&btn->node_text, btn->label ? btn->label : "", -1);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_action_button_create: cmp_ui_text_create failed\n");
    rc = cmp_ui_node_destroy(btn->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_action_button_create: cmp_ui_node_destroy failed\n");
    }
    if (btn->label) {
      {

        int free_rc_4 = CMP_FREE(btn->label);

        if (free_rc_4 != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_action_button_create: CMP_FREE label failed\n");
        }
      }
    }
    {

      int free_rc_5 = CMP_FREE(btn);

      if (free_rc_5 != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_action_button_create: CMP_FREE failed\n");
      }
    }

    return rc;
  }

  rc = cmp_ui_node_add_child(btn->node_root, btn->node_text);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_action_button_create: cmp_ui_node_add_child failed\n");
    cmp_ui_node_destroy(btn->node_text);
    cmp_ui_node_destroy(btn->node_root);
    CMP_FREE(btn->label);
    CMP_FREE(btn);
    return rc;
  }

  {
    cmp_m3_sys_colors_t colors;
    cmp_color_t seed;
    seed.r = 0x67 / CMP_COLOR_MAX_F;
    seed.g = 0x50 / CMP_COLOR_MAX_F;
    seed.b = 0xA4 / CMP_COLOR_MAX_F;
    seed.a = CMP_MATH_ONE;
    seed.space = CMP_COLOR_SPACE_SRGB;

    if (cmp_m3_sys_colors_generate(seed, 0, CMP_M3_CONTRAST_STANDARD,
                                   &colors) == CMP_SUCCESS) {
      btn->node_root->layout->box_sizing = CMP_BOX_SIZING_BORDER_BOX;
btn->node_root->layout->box_sizing = CMP_BOX_SIZING_BORDER_BOX;
btn->node_root->layout->height = CMP_UI_ACTION_BUTTON_HEIGHT;
      btn->node_root->layout->padding[0] = CMP_UI_ACTION_BUTTON_PADDING_H;
      btn->node_root->layout->padding[1] = CMP_UI_ACTION_BUTTON_PADDING_V;
      btn->node_root->layout->padding[2] = CMP_UI_ACTION_BUTTON_PADDING_H;
      btn->node_root->layout->padding[3] = CMP_UI_ACTION_BUTTON_PADDING_V;
      btn->node_root->border_radius = CMP_UI_ACTION_BUTTON_BORDER_RADIUS;
      {
        len = label ? strlen(label) : 0;
        btn->node_root->layout->width = (float)len * (CMP_UI_ACTION_BUTTON_FONT_SIZE * CMP_MATH_HALF) + (CMP_UI_ACTION_BUTTON_PADDING_H * CMP_MATH_DOUBLE);
      }

      btn->node_root->hover_opacity = CMP_UI_ACTION_BUTTON_HOVER_OPACITY;
      btn->node_root->press_opacity = CMP_UI_ACTION_BUTTON_PRESS_OPACITY;

      switch (style) {
      case CMP_UI_ACTION_BUTTON_STYLE_ELEVATED:
        btn->node_root->elevation = CMP_UI_ACTION_BUTTON_ELEVATION;
        btn->node_root->bg_color =
            CMP_COLOR_OPAQUE_MASK |
            ((uint32_t)(colors.surface_container_low.r * CMP_COLOR_MAX) << 16) |
            ((uint32_t)(colors.surface_container_low.g * CMP_COLOR_MAX) << 8) |
            (uint32_t)(colors.surface_container_low.b * CMP_COLOR_MAX);
        btn->node_text->text_color =
            CMP_COLOR_OPAQUE_MASK | ((uint32_t)(colors.primary.r * CMP_COLOR_MAX) << 16) |
            ((uint32_t)(colors.primary.g * CMP_COLOR_MAX) << 8) |
            (uint32_t)(colors.primary.b * CMP_COLOR_MAX);
        break;
      case CMP_UI_ACTION_BUTTON_STYLE_FILLED:
        btn->node_root->bg_color = CMP_COLOR_OPAQUE_MASK |
                                   ((uint32_t)(colors.primary.r * CMP_COLOR_MAX) << 16) |
                                   ((uint32_t)(colors.primary.g * CMP_COLOR_MAX) << 8) |
                                   (uint32_t)(colors.primary.b * CMP_COLOR_MAX);
        btn->node_text->text_color =
            CMP_COLOR_OPAQUE_MASK | ((uint32_t)(colors.on_primary.r * CMP_COLOR_MAX) << 16) |
            ((uint32_t)(colors.on_primary.g * CMP_COLOR_MAX) << 8) |
            (uint32_t)(colors.on_primary.b * CMP_COLOR_MAX);
        break;
      case CMP_UI_ACTION_BUTTON_STYLE_TONAL:
        btn->node_root->bg_color =
            CMP_COLOR_OPAQUE_MASK |
            ((uint32_t)(colors.secondary_container.r * CMP_COLOR_MAX) << 16) |
            ((uint32_t)(colors.secondary_container.g * CMP_COLOR_MAX) << 8) |
            (uint32_t)(colors.secondary_container.b * CMP_COLOR_MAX);
        btn->node_text->text_color =
            CMP_COLOR_OPAQUE_MASK |
            ((uint32_t)(colors.on_secondary_container.r * CMP_COLOR_MAX) << 16) |
            ((uint32_t)(colors.on_secondary_container.g * CMP_COLOR_MAX) << 8) |
            (uint32_t)(colors.on_secondary_container.b * CMP_COLOR_MAX);
        break;
      case CMP_UI_ACTION_BUTTON_STYLE_OUTLINED:
        btn->node_root->bg_color = CMP_COLOR_TRANSPARENT;
        btn->node_root->border_color =
            CMP_COLOR_OPAQUE_MASK | ((uint32_t)(colors.outline.r * CMP_COLOR_MAX) << 16) |
            ((uint32_t)(colors.outline.g * CMP_COLOR_MAX) << 8) |
            (uint32_t)(colors.outline.b * CMP_COLOR_MAX);
        btn->node_root->border_width = CMP_UI_ACTION_BUTTON_BORDER_WIDTH;
        btn->node_text->text_color =
            CMP_COLOR_OPAQUE_MASK | ((uint32_t)(colors.primary.r * CMP_COLOR_MAX) << 16) |
            ((uint32_t)(colors.primary.g * CMP_COLOR_MAX) << 8) |
            (uint32_t)(colors.primary.b * CMP_COLOR_MAX);
        break;
      case CMP_UI_ACTION_BUTTON_STYLE_TEXT:
        btn->node_root->bg_color = CMP_COLOR_TRANSPARENT;
        btn->node_root->layout->padding[0] = CMP_UI_ACTION_BUTTON_PADDING_TEXT_BTN_H;
        btn->node_root->layout->padding[2] = CMP_UI_ACTION_BUTTON_PADDING_TEXT_BTN_H;
        btn->node_text->text_color =
            CMP_COLOR_OPAQUE_MASK | ((uint32_t)(colors.primary.r * CMP_COLOR_MAX) << 16) |
            ((uint32_t)(colors.primary.g * CMP_COLOR_MAX) << 8) |
            (uint32_t)(colors.primary.b * CMP_COLOR_MAX);
        break;
      }
      btn->node_text->font_size = CMP_UI_ACTION_BUTTON_FONT_SIZE;
      btn->node_text->layout->box_sizing = CMP_BOX_SIZING_BORDER_BOX;
btn->node_text->layout->height = CMP_UI_ACTION_BUTTON_TEXT_HEIGHT;
      btn->node_text->layout->flex_grow = CMP_MATH_ONE;
      btn->node_text->layout->flex_shrink = CMP_MATH_ONE;
      btn->node_text->layout->text_overflow = CMP_TEXT_OVERFLOW_ELLIPSIS;
      btn->node_text->layout->white_space = CMP_WHITE_SPACE_NOWRAP;
      btn->node_root->layout->min_width = CMP_UI_ACTION_BUTTON_MIN_WIDTH;
    }
  }

  *out_btn = btn;
  return rc;
}

/**
 * @brief cmp_ui_action_button_destroy
 *
 * @param btn The button.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_action_button_destroy(cmp_ui_action_button_t *btn) {
  int rc = CMP_SUCCESS;
  if (!btn) {
    LOG_DEBUG("cmp_ui_action_button_destroy: btn is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (btn->label) {
    {

      int free_rc_6 = CMP_FREE(btn->label);

      if (free_rc_6 != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_action_button_destroy: CMP_FREE label failed\n");
      }
    }
  }
  if (btn->node_root) {
    rc = cmp_ui_node_destroy(btn->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_action_button_destroy: cmp_ui_node_destroy failed\n");
    }
  }
  {

    int free_rc_7 = CMP_FREE(btn);

    if (free_rc_7 != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_action_button_destroy: CMP_FREE failed\n");

      return rc;
    }
  }
  return rc;
}

/**
 * @brief cmp_ui_action_button_get_node
 *
 * @param btn The button.
 * @param out_node Pointer to receive the node.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_action_button_get_node(cmp_ui_action_button_t *btn,
                                  cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!btn || !out_node) {
    LOG_DEBUG("cmp_ui_action_button_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = btn->node_root;

  return rc;
}

/**
 * @brief cmp_ui_action_button_set_label
 *
 * @param btn The button.
 * @param label The new label.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_action_button_set_label(cmp_ui_action_button_t *btn,
                                   const char *label) {
  size_t len;
  int rc = CMP_SUCCESS;

  if (!btn) {
    LOG_DEBUG("cmp_ui_action_button_set_label: btn is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (btn->label) {
    {

      int free_rc_8 = CMP_FREE(btn->label);

      if (free_rc_8 != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_action_button_set_label: CMP_FREE label failed\n");
      }
    }
    btn->label = NULL;
  }

  if (label) {
    len = strlen(label);
    rc = CMP_MALLOC(len + 1, (void **)&btn->label);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_action_button_set_label: OOM\n");

      return rc;
    }
    memcpy(btn->label, label, len + 1);
    if (btn->node_text->properties) {
      {

        int free_rc_9 = CMP_FREE(btn->node_text->properties);

        if (free_rc_9 != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_action_button_set_label: CMP_FREE text properties "
                    "failed\n");
        }
      }
    }
    rc = CMP_MALLOC(len + 1, (void **)&btn->node_text->properties);
    if (rc == CMP_SUCCESS) {
      memcpy(btn->node_text->properties, label, len + 1);
    } else {
      LOG_DEBUG("cmp_ui_action_button_set_label: OOM text properties\n");
      return CMP_ERROR_OOM;
    }
  } else {
    if (btn->node_text->properties) {
      {

        int free_rc_10 = CMP_FREE(btn->node_text->properties);

        if (free_rc_10 != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_action_button_set_label: CMP_FREE text properties "
                    "failed\n");
        }
      }
    }
    rc = CMP_MALLOC(1, (void **)&btn->node_text->properties);
    if (rc == CMP_SUCCESS) {
      ((char *)btn->node_text->properties)[0] = '\0';
    } else {
      LOG_DEBUG("cmp_ui_action_button_set_label: OOM text properties\n");
      return CMP_ERROR_OOM;
    }
  }

  return rc;
}

/**
 * @brief cmp_ui_action_button_bind_a11y
 *
 * @param widget The component.
 * @param tree The accessibility tree.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_action_button_bind_a11y(cmp_ui_action_button_t *widget,
                                   cmp_a11y_tree_t *tree) {
  int rc = CMP_SUCCESS;
  if (!widget || !tree || !widget->node_root || !widget->node_root->layout) {
    LOG_DEBUG("cmp_ui_action_button_bind_a11y: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  rc = cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "button",
                              "Action Button");
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "cmp_ui_action_button_bind_a11y: cmp_a11y_tree_add_node failed\n");
  }
  rc = cmp_a11y_tree_set_node_traits(tree, widget->node_root->layout->id,
                                     CMP_A11Y_TRAIT_BUTTON);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_action_button_bind_a11y: cmp_a11y_tree_set_node_traits "
              "failed\n");
  }
  return rc;
}
