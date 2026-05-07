/* clang-format off */
#include "cmp_ui_switch.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_switch {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_thumb;
  int is_on;
};

/**
 * @brief cmp_ui_switch_create
 *
 * @param out_switch Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_switch_create(cmp_ui_switch_t **out_switch) {
  int rc = CMP_SUCCESS;
  cmp_ui_switch_t *sw;
  int err;

  if (!out_switch) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_switch_t), (void **)&sw);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(sw, 0, sizeof(cmp_ui_switch_t));

  /* Create the track background */
  err = cmp_ui_box_create(&sw->node_root);
  if (err != CMP_SUCCESS) {
    if (CMP_FREE(sw) != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_switch_create: CMP_FREE sw failed\n");
    }
    return err;
  }

  err = CMP_MALLOC(sizeof(cmp_layout_node_t), (void **)&sw->node_root->layout);
  if (err == CMP_SUCCESS) {
    memset(sw->node_root->layout, 0, sizeof(cmp_layout_node_t));
    sw->node_root->layout->id = 1;
    sw->node_root->layout->direction = CMP_FLEX_ROW;
  }

  /* Create the thumb */
  err = cmp_ui_box_create(&sw->node_thumb);
  if (err != CMP_SUCCESS) {
    cmp_ui_node_destroy(sw->node_root);
    if (CMP_FREE(sw) != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_switch_create: CMP_FREE sw failed\n");
    }
    return err;
  }

  sw->is_on = 0; /* Default OFF */

  sw->node_root->bg_color = 0xFFCCCCCC;  /* Track off color */
  sw->node_thumb->bg_color = 0xFFFFFFFF; /* Thumb color */

  err = cmp_ui_node_add_child(sw->node_root, sw->node_thumb);
  if (err != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_switch_create: cmp_ui_node_add_child failed\n");
  }

  *out_switch = sw;

  return rc;
}

/**
 * @brief cmp_ui_switch_destroy
 *
 * @param sw Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_switch_destroy(cmp_ui_switch_t *sw) {
  int rc = CMP_SUCCESS;
  if (!sw) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (CMP_FREE(sw) != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_switch_destroy: CMP_FREE sw failed\n");
  }

  return rc;
}

/**
 * @brief cmp_ui_switch_get_node
 *
 * @param sw Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_switch_get_node(cmp_ui_switch_t *sw, cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!sw || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = sw->node_root;

  return rc;
}

/**
 * @brief cmp_ui_switch_set_on
 *
 * @param sw Parameter description.
 * @param is_on Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_switch_set_on(cmp_ui_switch_t *sw, int is_on) {
  int rc = CMP_SUCCESS;
  if (!sw) {
    return CMP_ERROR_INVALID_ARG;
  }

  sw->is_on = is_on;
  /* Visual changes would be handled by updating layout padding /
   * justify_content to push the thumb, and altering track background colors */
  if (is_on) {
    sw->node_root->bg_color = 0xFF2196F3; /* Active track color */
  } else {
    sw->node_root->bg_color = 0xFFCCCCCC; /* Inactive track color */
  }

  return rc;
}
/**
 * @brief cmp_ui_switch_bind_a11y
 *
 * @param widget Parameter description.
 * @param tree Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_switch_bind_a11y(cmp_ui_switch_t *widget, cmp_a11y_tree_t *tree) {
  int rc = CMP_SUCCESS;
  int err;
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  err = cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "switch",
                               "Switch");
  if (err != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_switch_bind_a11y: cmp_a11y_tree_add_node failed\n");
  }

  rc = err;
  return rc;
}
