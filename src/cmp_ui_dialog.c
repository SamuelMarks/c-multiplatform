/* clang-format off */
#include "cmp_ui_dialog.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_dialog {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_title;
  cmp_ui_node_t *node_content;
  int is_visible;
};

int cmp_ui_dialog_create(cmp_ui_dialog_t **out_dialog, const char *title,
                         const char *content) {
  cmp_ui_dialog_t *dialog;
  int err;

  if (!out_dialog) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_dialog_t), (void **)&dialog);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(dialog, 0, sizeof(cmp_ui_dialog_t));

  err = cmp_ui_box_create(&dialog->node_root);
  if (err != CMP_SUCCESS) {
    CMP_FREE(dialog);
    return err;
  }

  dialog->node_root->layout->direction = CMP_FLEX_COLUMN;
  dialog->is_visible = 0; /* Initially hidden */

  if (title) {
    err = cmp_ui_text_create(&dialog->node_title, title, -1);
    if (err == CMP_SUCCESS) {
      cmp_ui_node_add_child(dialog->node_root, dialog->node_title);
    }
  }

  if (content) {
    err = cmp_ui_text_create(&dialog->node_content, content, -1);
    if (err == CMP_SUCCESS) {
      cmp_ui_node_add_child(dialog->node_root, dialog->node_content);
    }
  }

  *out_dialog = dialog;
  return CMP_SUCCESS;
}

int cmp_ui_dialog_destroy(cmp_ui_dialog_t *dialog) {
  if (!dialog) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(dialog);
  return CMP_SUCCESS;
}

int cmp_ui_dialog_get_node(cmp_ui_dialog_t *dialog, cmp_ui_node_t **out_node) {
  if (!dialog || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = dialog->node_root;
  return CMP_SUCCESS;
}

int cmp_ui_dialog_set_visible(cmp_ui_dialog_t *dialog, int visible) {
  if (!dialog) {
    return CMP_ERROR_INVALID_ARG;
  }

  dialog->is_visible = visible;
  /* Visual transition to hide or show via CSS/layout would be triggered here */

  return CMP_SUCCESS;
}
int cmp_ui_dialog_bind_a11y(cmp_ui_dialog_t *widget, cmp_a11y_tree_t *tree) {
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "dialog",
                         "Dialog");
  return CMP_SUCCESS;
}
