/* clang-format off */
#include "cmp_ui_dialog.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Opaque internal structure for UI Dialog widget.
 */
struct cmp_ui_dialog {
  /** @brief The root node of the dialog */
  cmp_ui_node_t *node_root;
  /** @brief The title text node */
  cmp_ui_node_t *node_title;
  /** @brief The content text node */
  cmp_ui_node_t *node_content;
  /** @brief Visible state of the dialog */
  int is_visible;
};

/**
 * @brief Creates a new UI dialog component.
 *
 * @param out_dialog Pointer to store the created dialog.
 * @param title The title string.
 * @param content The content string.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_dialog_create(cmp_ui_dialog_t **out_dialog, const char *title,
                         const char *content) {
  int rc = CMP_SUCCESS;
  cmp_ui_dialog_t *dialog = NULL;

  if (!out_dialog) {
    LOG_DEBUG("cmp_ui_dialog_create: out_dialog is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_dialog_t), (void **)&dialog);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_dialog_create: OOM\n");
    return CMP_ERROR_OOM;
  }
  memset(dialog, 0, sizeof(cmp_ui_dialog_t));

  rc = cmp_ui_box_create(&dialog->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_dialog_create: cmp_ui_box_create failed\n");
    rc = CMP_FREE(dialog);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_dialog_create: CMP_FREE failed\n");
    }
    return CMP_ERROR_GENERAL;
  }

  if (dialog->node_root && dialog->node_root->layout) {
    dialog->node_root->layout->direction = CMP_FLEX_COLUMN;
  }

  dialog->is_visible = 0; /* Initially hidden */

  if (title) {
    rc = cmp_ui_text_create(&dialog->node_title, title, -1);
    if (rc == CMP_SUCCESS) {
      rc = cmp_ui_node_add_child(dialog->node_root, dialog->node_title);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_dialog_create: cmp_ui_node_add_child title failed\n");
      }
    } else {
      LOG_DEBUG("cmp_ui_dialog_create: cmp_ui_text_create title failed\n");
    }
  }

  if (content) {
    rc = cmp_ui_text_create(&dialog->node_content, content, -1);
    if (rc == CMP_SUCCESS) {
      rc = cmp_ui_node_add_child(dialog->node_root, dialog->node_content);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG(
            "cmp_ui_dialog_create: cmp_ui_node_add_child content failed\n");
      }
    } else {
      LOG_DEBUG("cmp_ui_dialog_create: cmp_ui_text_create content failed\n");
    }
  }

  *out_dialog = dialog;
  return rc;
}

/**
 * @brief Destroys a UI dialog component and frees its resources.
 *
 * @param dialog The dialog component.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_dialog_destroy(cmp_ui_dialog_t *dialog) {
  int rc = CMP_SUCCESS;
  if (!dialog) {
    LOG_DEBUG("cmp_ui_dialog_destroy: dialog is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (dialog->node_root) {
    rc = cmp_ui_node_destroy(dialog->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_dialog_destroy: cmp_ui_node_destroy failed\n");
    }
  }
  rc = CMP_FREE(dialog);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_dialog_destroy: CMP_FREE failed\n");

    return rc;
  }
  return rc;
}

/**
 * @brief Retrieves the root UI node of the dialog component.
 *
 * @param dialog The dialog component.
 * @param out_node Pointer to store the underlying UI node.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_dialog_get_node(cmp_ui_dialog_t *dialog, cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!dialog || !out_node) {
    LOG_DEBUG("cmp_ui_dialog_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = dialog->node_root;

  return rc;
}

/**
 * @brief Sets the visibility state of the UI dialog.
 *
 * @param dialog The dialog component.
 * @param visible 1 to show, 0 to hide.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_dialog_set_visible(cmp_ui_dialog_t *dialog, int visible) {
  int rc = CMP_SUCCESS;
  if (!dialog) {
    LOG_DEBUG("cmp_ui_dialog_set_visible: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  dialog->is_visible = visible;

  return rc;
}

/**
 * @brief Binds the dialog to an accessibility tree.
 *
 * @param widget The component.
 * @param tree The accessibility tree.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_dialog_bind_a11y(cmp_ui_dialog_t *widget, cmp_a11y_tree_t *tree) {
  int rc = CMP_SUCCESS;
  if (!widget || !tree) {
    LOG_DEBUG("cmp_ui_dialog_bind_a11y: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (!widget->node_root || !widget->node_root->layout) {
    LOG_DEBUG("cmp_ui_dialog_bind_a11y: widget missing layout\n");
    return CMP_ERROR_INVALID_ARG;
  }
  rc = cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "dialog",
                              "Dialog");
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_dialog_bind_a11y: cmp_a11y_tree_add_node failed\n");

    return rc;
  }
  return rc;
}
