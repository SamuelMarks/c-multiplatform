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
 * @brief Creates a focus navigation context, binding it to an accessibility
 * tree.
 *
 * @param tree Pointer to the accessibility tree.
 * @param out_nav Pointer to store the newly created focus navigation context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_focus_nav_create(cmp_a11y_tree_t *tree, cmp_focus_nav_t **out_nav) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_focus_nav_t *nav = NULL;

  if (tree == NULL || out_nav == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_nav_create: Invalid argument: %s\n", err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_focus_nav_t), (void **)&nav);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_nav_create: Out of memory: %s\n", err_str);

    return rc;
  }

  nav->tree = tree;
  nav->current_focus_id = -1;

  rc = cmp_focus_ring_create(tree, &nav->ring);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_nav_create: Failed to create focus ring: %s\n",
                  err_str);
    CMP_FREE(nav);

    return rc;
  }

  *out_nav = nav;
  cmp_log_debug(
      "cmp_focus_nav_create: Successfully created focus nav context\n");
  return rc;
}

/**
 * @brief Destroys a focus navigation context and frees its associated
 * resources.
 *
 * @param nav Pointer to the focus navigation context to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_focus_nav_destroy(cmp_focus_nav_t *nav) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (nav == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_nav_destroy: Invalid argument: %s\n", err_str);

    return rc;
  }

  if (nav->ring != NULL) {
    rc = cmp_focus_ring_destroy(nav->ring);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_focus_nav_destroy: Failed to destroy focus ring\n");
    }
  }

  rc = CMP_FREE(nav);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_focus_nav_destroy: Failed to free nav\n");

    return rc;
  }

  cmp_log_debug(
      "cmp_focus_nav_destroy: Successfully destroyed focus nav context\n");
  return rc;
}

/**
 * @brief Handles spatial advancement triggered by the TAB key.
 *
 * @param nav Pointer to the focus navigation context.
 * @param is_shift_pressed 1 if the Shift key was pressed (for reverse tab), 0
 * otherwise.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_focus_nav_handle_tab(cmp_focus_nav_t *nav, int is_shift_pressed) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (nav == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_nav_handle_tab: Invalid argument: %s\n", err_str);

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
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_nav_handle_tab: Failed to set keyboard mode: %s\n",
                  err_str);

    return rc;
  }

  /* Dummy advancement, in reality this queries the DOM/Tree */
  nav->current_focus_id++;

  /* Mirror the focus into the native focus ring */
  rc = cmp_focus_ring_node_focused(nav->ring, nav->current_focus_id);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_focus_nav_handle_tab: Failed to focus node on ring: %s\n",
        err_str);

    return rc;
  }

  cmp_log_debug("cmp_focus_nav_handle_tab: Handled TAB spatial advance\n");
  return rc;
}
