/* clang-format off */
#include "cmp_ui_action_button.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

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
      rc = CMP_FREE(btn);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_action_button_create: CMP_FREE failed\n");
      }
      return CMP_ERROR_OOM;
    }
  }

  rc = cmp_ui_button_create(&btn->node_root, "", 0);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_action_button_create: cmp_ui_button_create failed\n");
    if (btn->label) {
      rc = CMP_FREE(btn->label);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_action_button_create: CMP_FREE label failed\n");
      }
    }
    rc = CMP_FREE(btn);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_action_button_create: CMP_FREE failed\n");
    }

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_layout_node_t), (void **)&btn->node_root->layout);
  if (rc == CMP_SUCCESS) {
    memset(btn->node_root->layout, 0, sizeof(cmp_layout_node_t));
    btn->node_root->layout->id = 1;
  }

  btn->node_root->type = 3; /* Button */

  rc = cmp_ui_text_create(&btn->node_text, btn->label ? btn->label : "", -1);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_action_button_create: cmp_ui_text_create failed\n");
    rc = cmp_ui_node_destroy(btn->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_action_button_create: cmp_ui_node_destroy failed\n");
    }
    if (btn->label) {
      rc = CMP_FREE(btn->label);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_action_button_create: CMP_FREE label failed\n");
      }
    }
    rc = CMP_FREE(btn);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_action_button_create: CMP_FREE failed\n");
    }

    return rc;
  }

  rc = cmp_ui_node_add_child(btn->node_root, btn->node_text);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_action_button_create: cmp_ui_node_add_child failed\n");
    /* Handle but continue */
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
    rc = CMP_FREE(btn->label);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_action_button_destroy: CMP_FREE label failed\n");
    }
  }
  if (btn->node_root) {
    rc = cmp_ui_node_destroy(btn->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_action_button_destroy: cmp_ui_node_destroy failed\n");
    }
  }
  rc = CMP_FREE(btn);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_action_button_destroy: CMP_FREE failed\n");

    return rc;
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
    rc = CMP_FREE(btn->label);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_action_button_set_label: CMP_FREE label failed\n");
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
      rc = CMP_FREE(btn->node_text->properties);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_action_button_set_label: CMP_FREE text properties "
                  "failed\n");
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
      rc = CMP_FREE(btn->node_text->properties);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_action_button_set_label: CMP_FREE text properties "
                  "failed\n");
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
