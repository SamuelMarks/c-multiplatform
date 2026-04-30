/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief cmp_ui_node_set_touch_action
 *
 * @param node Parameter description.
 * @param action Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_node_set_touch_action(cmp_ui_node_t *node, uint32_t action) {
  int rc = CMP_SUCCESS;
  if (!node)
    return CMP_ERROR_INVALID_ARG;

  /* Mock storing the bitfield value in the node's properties ptr */
  node->properties = (void *)(size_t)(action);

  return rc;
}

/**
 * @brief cmp_ui_node_get_touch_action
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_node_get_touch_action(const cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  if (!node)
    return (int)CMP_TOUCH_ACTION_AUTO;

  if (node->properties) {
    return (int)(size_t)node->properties;
  }

  rc = (int)CMP_TOUCH_ACTION_AUTO;
  return rc;
}
