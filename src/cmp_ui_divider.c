/* clang-format off */
#include "cmp_ui_divider.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_divider {
  cmp_ui_node_t *node_root;
};

/**
 * @brief cmp_ui_divider_create
 *
 * @param out_divider Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_divider_create(cmp_ui_divider_t **out_divider) {
  cmp_ui_divider_t *divider;
  int rc;

  if (!out_divider) {
    LOG_DEBUG("cmp_ui_divider_create: out_divider is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_divider_t), (void **)&divider);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_divider_create: OOM\n");
    return rc;
  }
  memset(divider, 0, sizeof(cmp_ui_divider_t));

  rc = cmp_ui_box_create(&divider->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_divider_create: cmp_ui_box_create failed\n");
    CMP_FREE(divider);
    return rc;
  }

  /* Usually a divider is 1px tall and takes 100% width or similar */
  /* Here we simply provide a background color; exact dimensions would
     be provided via styles. */
  divider->node_root->bg_color = 0xFFCCCCCC;

  *out_divider = divider;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_divider_destroy
 *
 * @param divider Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_divider_destroy(cmp_ui_divider_t *divider) {
  int rc;

  if (!divider) {
    LOG_DEBUG("cmp_ui_divider_destroy: divider is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (divider->node_root) {
    rc = cmp_ui_node_destroy(divider->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_divider_destroy: cmp_ui_node_destroy failed\n");
    }
  }

  rc = CMP_FREE(divider);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_divider_destroy: CMP_FREE failed\n");
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_divider_get_node
 *
 * @param divider Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_divider_get_node(cmp_ui_divider_t *divider,
                            cmp_ui_node_t **out_node) {
  if (!divider || !out_node) {
    LOG_DEBUG("cmp_ui_divider_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = divider->node_root;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_divider_bind_a11y
 *
 * @param widget Parameter description.
 * @param tree Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_divider_bind_a11y(cmp_ui_divider_t *widget, cmp_a11y_tree_t *tree) {
  int rc;

  if (!widget || !tree) {
    LOG_DEBUG("cmp_ui_divider_bind_a11y: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (!widget->node_root || !widget->node_root->layout) {
    LOG_DEBUG("cmp_ui_divider_bind_a11y: widget missing layout\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "separator",
                              "Divider");
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_divider_bind_a11y: cmp_a11y_tree_add_node failed\n");
    return rc;
  }

  return CMP_SUCCESS;
}