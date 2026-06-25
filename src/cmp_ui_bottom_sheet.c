/* clang-format off */
#include "cmp_ui_bottom_sheet.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Opaque internal structure for UI Bottom Sheet widget.
 */
struct cmp_ui_bottom_sheet {
  /** @brief The root node of the bottom sheet */
  cmp_ui_node_t *node_root;
  /** @brief Visible state of the bottom sheet */
  int is_visible;
};

/**
 * @brief cmp_ui_bottom_sheet_create
 *
 * @param out_sheet Pointer to store the created bottom sheet.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_bottom_sheet_create(cmp_ui_bottom_sheet_t **out_sheet) {
  cmp_ui_bottom_sheet_t *sheet;
  int rc = CMP_SUCCESS;

  if (!out_sheet) {
    LOG_DEBUG("cmp_ui_bottom_sheet_create: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_bottom_sheet_t), (void **)&sheet);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_bottom_sheet_create: OOM\n");
    return CMP_ERROR_OOM;
  }

  rc = cmp_ui_box_create(&sheet->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_bottom_sheet_create: cmp_ui_box_create failed\n");
    CMP_FREE(sheet);
    return rc;
  }

  sheet->node_root->layout->direction = CMP_FLEX_COLUMN;

  sheet->is_visible = 0; /* Initially hidden */

  *out_sheet = sheet;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_bottom_sheet_destroy
 *
 * @param sheet The bottom sheet component.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_bottom_sheet_destroy(cmp_ui_bottom_sheet_t *sheet) {
  int rc = CMP_SUCCESS;
  if (!sheet) {
    LOG_DEBUG("cmp_ui_bottom_sheet_destroy: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (sheet->node_root) {
    rc = cmp_ui_node_destroy(sheet->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_bottom_sheet_destroy: cmp_ui_node_destroy failed\n");
    }
  }
  {

    int free_rc_2 = CMP_FREE(sheet);

    if (free_rc_2 != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_bottom_sheet_destroy: CMP_FREE failed\n");

      return rc;
    }
  }
  return rc;
}

/**
 * @brief cmp_ui_bottom_sheet_get_node
 *
 * @param sheet The bottom sheet component.
 * @param out_node Pointer to store the underlying UI node.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_bottom_sheet_get_node(cmp_ui_bottom_sheet_t *sheet,
                                 cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!sheet || !out_node) {
    LOG_DEBUG("cmp_ui_bottom_sheet_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = sheet->node_root;

  return rc;
}

/**
 * @brief cmp_ui_bottom_sheet_set_visible
 *
 * @param sheet The bottom sheet component.
 * @param visible 1 to show, 0 to hide.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_bottom_sheet_set_visible(cmp_ui_bottom_sheet_t *sheet, int visible) {
  int rc = CMP_SUCCESS;
  if (!sheet) {
    LOG_DEBUG("cmp_ui_bottom_sheet_set_visible: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  sheet->is_visible = visible;

  return rc;
}

/**
 * @brief cmp_ui_bottom_sheet_bind_a11y
 *
 * @param widget The component.
 * @param tree The accessibility tree.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_bottom_sheet_bind_a11y(cmp_ui_bottom_sheet_t *widget,
                                  cmp_a11y_tree_t *tree) {
  int rc = CMP_SUCCESS;
  if (!widget || !tree) {
    LOG_DEBUG("cmp_ui_bottom_sheet_bind_a11y: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (!widget->node_root || !widget->node_root->layout) {
    LOG_DEBUG("cmp_ui_bottom_sheet_bind_a11y: Missing layout\n");
    return CMP_ERROR_INVALID_ARG;
  }
  rc = cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "dialog",
                              "Bottom Sheet");
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_bottom_sheet_bind_a11y: cmp_a11y_tree_add_node failed\n");

    return rc;
  }
  return rc;
}
