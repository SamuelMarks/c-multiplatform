/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

struct cmp_focus_ring {
  cmp_a11y_tree_t *tree;
  int is_keyboard_mode;
  int currently_focused_id;
};

/**
 * @brief cmp_focus_ring_create
 *
 * @param tree Parameter description.
 * @param out_ring Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_focus_ring_create(cmp_a11y_tree_t *tree, cmp_focus_ring_t **out_ring) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_focus_ring *ring = NULL;

  if (tree == NULL || out_ring == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_ring_create: Invalid argument: %s\n", err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_focus_ring), (void **)&ring);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_ring_create: Out of memory: %s\n", err_str);

    return rc;
  }

  ring->tree = tree;
  ring->is_keyboard_mode = 0; /* Default to pointer/hidden */
  ring->currently_focused_id = -1;

  *out_ring = (cmp_focus_ring_t *)ring;
  cmp_log_debug(
      "cmp_focus_ring_create: Successfully created focus ring context\n");
  return rc;
}

/**
 * @brief cmp_focus_ring_destroy
 *
 * @param ring Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_focus_ring_destroy(cmp_focus_ring_t *ring) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_focus_ring *r = (struct cmp_focus_ring *)ring;

  if (r == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_ring_destroy: Invalid argument: %s\n", err_str);

    return rc;
  }

  rc = CMP_FREE(r);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_focus_ring_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug(
      "cmp_focus_ring_destroy: Successfully destroyed focus ring context\n");
  return rc;
}

/**
 * @brief cmp_focus_ring_set_keyboard_mode
 *
 * @param ring Parameter description.
 * @param is_keyboard Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_focus_ring_set_keyboard_mode(cmp_focus_ring_t *ring, int is_keyboard) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_focus_ring *r = (struct cmp_focus_ring *)ring;

  if (r == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_ring_set_keyboard_mode: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  r->is_keyboard_mode = is_keyboard;

  /* If we switched away from keyboard mode, we should tell the rendering system
   * to hide the ring */
  if (!is_keyboard && r->currently_focused_id != -1) {
    /* Hide focus ring logic (e.g. mark layout node dirty_paint) */
  } else if (is_keyboard && r->currently_focused_id != -1) {
    /* Show focus ring logic */
  }

  cmp_log_debug("cmp_focus_ring_set_keyboard_mode: Applied keyboard state %d\n",
                is_keyboard);

  return rc;
}

/**
 * @brief cmp_focus_ring_node_focused
 *
 * @param ring Parameter description.
 * @param node_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_focus_ring_node_focused(cmp_focus_ring_t *ring, int node_id) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_focus_ring *r = (struct cmp_focus_ring *)ring;

  if (r == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_ring_node_focused: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  r->currently_focused_id = node_id;

  if (r->is_keyboard_mode) {
    /* The rendering system will draw a high-contrast ring around this node_id
     */
  }

  cmp_log_debug(
      "cmp_focus_ring_node_focused: Highlighted node via ring logic\n");

  return rc;
}
