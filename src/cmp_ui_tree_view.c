/* clang-format off */
#include "cmp_ui_tree_view.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

struct cmp_ui_tree_view {
  cmp_ui_node_t *node_root;
  int item_count;
};

/**
 * @brief Creates a new tree view component.
 *
 * @param out_tree_view Pointer to store the created tree view handle.
 * @param bg_color Background color of the tree view (ARGB).
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_tree_view_create(cmp_ui_tree_view_t **out_tree_view,
                            uint32_t bg_color) {
  int rc = CMP_SUCCESS;
  cmp_ui_tree_view_t *tree_view;
  int err;

  if (!out_tree_view) {
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_tree_view_t), (void **)&(tree_view));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  tree_view->item_count = 0;

  err = cmp_ui_box_create(&tree_view->node_root);
  if (err != 0) {
    rc = CMP_FREE(tree_view);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    return err;
  }

  tree_view->node_root->bg_color = bg_color;

  *out_tree_view = tree_view;
  return rc;
}

/**
 * @brief cmp_ui_tree_view_destroy
 *
 * @param tree_view Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_tree_view_destroy(cmp_ui_tree_view_t *tree_view) {
  int rc = CMP_SUCCESS;
  if (!tree_view) {
    return CMP_ERROR_INVALID_ARG;
  }
  rc = CMP_FREE(tree_view);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return rc;
}

/**
 * @brief cmp_ui_tree_view_get_node
 *
 * @param tree_view Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_tree_view_get_node(cmp_ui_tree_view_t *tree_view,
                              cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!tree_view || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = tree_view->node_root;

  return rc;
}

/**
 * @brief cmp_ui_tree_view_add_item
 *
 * @param tree_view Parameter description.
 * @param label Parameter description.
 * @param depth Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_tree_view_add_item(cmp_ui_tree_view_t *tree_view, const char *label,
                              int depth) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node_text;
  int err;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_label = label;

  if (!tree_view || !label || depth < 0) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (cmp_i18n_translate(label, &translated) == 0 && translated.data) {
    final_label = translated.data;
  }

  err = cmp_ui_text_create(&node_text, final_label, -1);

  if (translated.data) {
    cmp_string_destroy(&translated);
  }

  if (err != 0) {
    return err;
  }

  cmp_ui_node_add_child(tree_view->node_root, node_text);
  tree_view->item_count++;

  return rc;
}
