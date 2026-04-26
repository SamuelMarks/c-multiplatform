/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_focus_nav {
  cmp_a11y_tree_t *tree;
  cmp_focus_ring_t *ring;
  int current_focus_id;
};

/**
 * @brief cmp_focus_nav_create
 *
 * @param tree Parameter description.
 * @param out_nav Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_focus_nav_create(cmp_a11y_tree_t *tree, cmp_focus_nav_t **out_nav) {
  int rc = CMP_SUCCESS;
  cmp_focus_nav_t *nav = NULL;

  if (!tree || !out_nav) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_focus_nav_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_focus_nav_t), (void **)&nav);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_focus_nav_create: Out of memory\n");
    return rc;
  }

  nav->tree = tree;
  nav->current_focus_id = -1;

  rc = cmp_focus_ring_create(tree, &nav->ring);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(nav);
    LOG_DEBUG("Error in cmp_focus_nav_create: Failed to create focus ring\n");
    return rc;
  }

  *out_nav = nav;
  return rc;
}

/**
 * @brief cmp_focus_nav_destroy
 *
 * @param nav Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_focus_nav_destroy(cmp_focus_nav_t *nav) {
  int rc = CMP_SUCCESS;
  int ret_rc = CMP_SUCCESS;

  if (!nav) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_focus_nav_destroy: Invalid argument\n");
    return rc;
  }
  if (nav->ring) {
    rc = cmp_focus_ring_destroy(nav->ring);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "Error in cmp_focus_nav_destroy: Failed to destroy focus ring\n");
      ret_rc = rc;
    }
  }
  rc = CMP_FREE(nav);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_focus_nav_destroy: Failed to free nav\n");
    ret_rc = rc;
  }
  return ret_rc;
}

/**
 * @brief cmp_focus_nav_handle_tab
 *
 * @param nav Parameter description.
 * @param is_shift_pressed Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_focus_nav_handle_tab(cmp_focus_nav_t *nav, int is_shift_pressed) {
  int rc = CMP_SUCCESS;

  if (!nav) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_focus_nav_handle_tab: Invalid argument\n");
    return rc;
  }

  /*
   * In a complete implementation, this would query the accessibility tree
   * for the next/previous focusable node and set its state.
   */
  (void)is_shift_pressed;

  /* Tell the focus ring to render keyboard navigation visual cues */
  rc = cmp_focus_ring_set_keyboard_mode(nav->ring, 1);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "Error in cmp_focus_nav_handle_tab: Failed to set keyboard mode\n");
    return rc;
  }

  /* Dummy advancement, in reality this queries the DOM/Tree */
  nav->current_focus_id++;

  /* Mirror the focus into the native focus ring */
  rc = cmp_focus_ring_node_focused(nav->ring, nav->current_focus_id);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "Error in cmp_focus_nav_handle_tab: Failed to focus node on ring\n");
    return rc;
  }

  return rc;
}