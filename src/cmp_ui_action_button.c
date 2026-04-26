/* clang-format off */
#include "cmp_ui_action_button.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_action_button {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_text;
  char *label;
  cmp_ui_action_button_style_t style;
};

/**
 * @brief cmp_ui_action_button_create
 *
 * @param out_btn Parameter description.
 * @param label Parameter description.
 * @param style Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_action_button_create(cmp_ui_action_button_t **out_btn,
                                const char *label,
                                cmp_ui_action_button_style_t style) {
  cmp_ui_action_button_t *btn;
  int err;
  size_t len;

  if (!out_btn) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_action_button_t), (void **)&btn);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(btn, 0, sizeof(cmp_ui_action_button_t));

  btn->style = style;

  if (label) {
    len = strlen(label);
    err = CMP_MALLOC(len + 1, (void **)&btn->label);
    if (err == CMP_SUCCESS) {
      memcpy(btn->label, label, len + 1);
    }
  }

  err = cmp_ui_button_create(&btn->node_root, "", 0);
  if (err != CMP_SUCCESS) {
    CMP_FREE(btn->label);
    CMP_FREE(btn);
    return err;
  }

  btn->node_root->type = 3; /* Button */

  err = cmp_ui_text_create(&btn->node_text, btn->label ? btn->label : "", -1);
  if (err != CMP_SUCCESS) {
    cmp_ui_node_destroy(btn->node_root);
    CMP_FREE(btn->label);
    CMP_FREE(btn);
    return err;
  }

  cmp_ui_node_add_child(btn->node_root, btn->node_text);

  *out_btn = btn;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_action_button_destroy
 *
 * @param btn Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_action_button_destroy(cmp_ui_action_button_t *btn) {
  if (!btn) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(btn->label);
  CMP_FREE(btn);
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_action_button_get_node
 *
 * @param btn Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_action_button_get_node(cmp_ui_action_button_t *btn,
                                  cmp_ui_node_t **out_node) {
  if (!btn || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = btn->node_root;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_action_button_set_label
 *
 * @param btn Parameter description.
 * @param label Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_action_button_set_label(cmp_ui_action_button_t *btn,
                                   const char *label) {
  size_t len;
  int err;

  if (!btn) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (btn->label) {
    CMP_FREE(btn->label);
    btn->label = NULL;
  }

  if (label) {
    len = strlen(label);
    err = CMP_MALLOC(len + 1, (void **)&btn->label);
    if (err != CMP_SUCCESS) {
      return err;
    }
    memcpy(btn->label, label, len + 1);
    if (btn->node_text->properties) {
      CMP_FREE(btn->node_text->properties);
    }
    err = CMP_MALLOC(len + 1, (void **)&btn->node_text->properties);
    if (err == CMP_SUCCESS) {
      memcpy(btn->node_text->properties, label, len + 1);
    }
  } else {
    if (btn->node_text->properties) {
      CMP_FREE(btn->node_text->properties);
    }
    err = CMP_MALLOC(1, (void **)&btn->node_text->properties);
    if (err == CMP_SUCCESS) {
      ((char *)btn->node_text->properties)[0] = '\0';
    }
  }

  return CMP_SUCCESS;
}
/**
 * @brief cmp_ui_action_button_bind_a11y
 *
 * @param widget Parameter description.
 * @param tree Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_action_button_bind_a11y(cmp_ui_action_button_t *widget,
                                   cmp_a11y_tree_t *tree) {
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "button",
                         "Action Button");
  cmp_a11y_tree_set_node_traits(tree, widget->node_root->layout->id,
                                CMP_A11Y_TRAIT_BUTTON);
  return CMP_SUCCESS;
}
