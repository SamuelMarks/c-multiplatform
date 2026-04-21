/* clang-format off */
#include "cmp_ui_icon_button.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_icon_button {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_icon;
  char *icon_name;
  cmp_ui_icon_button_style_t style;
};

int cmp_ui_icon_button_create(cmp_ui_icon_button_t **out_btn,
                              const char *icon_name,
                              cmp_ui_icon_button_style_t style) {
  cmp_ui_icon_button_t *btn;
  int err;
  size_t len;

  if (!out_btn) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_icon_button_t), (void **)&btn);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(btn, 0, sizeof(cmp_ui_icon_button_t));

  btn->style = style;

  if (icon_name) {
    len = strlen(icon_name);
    err = CMP_MALLOC(len + 1, (void **)&btn->icon_name);
    if (err == CMP_SUCCESS) {
      memcpy(btn->icon_name, icon_name, len + 1);
    }
  }

  err = cmp_ui_button_create(&btn->node_root, "", 0);
  if (err != CMP_SUCCESS) {
    CMP_FREE(btn->icon_name);
    CMP_FREE(btn);
    return err;
  }

  btn->node_root->type = 3; /* Button */

  err = cmp_ui_text_create(&btn->node_icon,
                           btn->icon_name ? btn->icon_name : "", -1);
  if (err != CMP_SUCCESS) {
    cmp_ui_node_destroy(btn->node_root);
    CMP_FREE(btn->icon_name);
    CMP_FREE(btn);
    return err;
  }

  cmp_ui_node_add_child(btn->node_root, btn->node_icon);

  *out_btn = btn;
  return CMP_SUCCESS;
}

int cmp_ui_icon_button_destroy(cmp_ui_icon_button_t *btn) {
  if (!btn) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(btn->icon_name);
  CMP_FREE(btn);
  return CMP_SUCCESS;
}

int cmp_ui_icon_button_get_node(cmp_ui_icon_button_t *btn,
                                cmp_ui_node_t **out_node) {
  if (!btn || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = btn->node_root;
  return CMP_SUCCESS;
}

int cmp_ui_icon_button_set_icon(cmp_ui_icon_button_t *btn,
                                const char *icon_name) {
  size_t len;
  int err;

  if (!btn) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (btn->icon_name) {
    CMP_FREE(btn->icon_name);
    btn->icon_name = NULL;
  }

  if (icon_name) {
    len = strlen(icon_name);
    err = CMP_MALLOC(len + 1, (void **)&btn->icon_name);
    if (err != CMP_SUCCESS) {
      return err;
    }
    memcpy(btn->icon_name, icon_name, len + 1);
  }

  return CMP_SUCCESS;
}
int cmp_ui_icon_button_bind_a11y(cmp_ui_icon_button_t *widget,
                                 cmp_a11y_tree_t *tree) {
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "button",
                         "Icon Button");
  cmp_a11y_tree_set_node_traits(tree, widget->node_root->layout->id,
                                CMP_A11Y_TRAIT_BUTTON);
  return CMP_SUCCESS;
}
