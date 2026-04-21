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

int cmp_focus_ring_create(cmp_a11y_tree_t *tree, cmp_focus_ring_t **out_ring) {
  int rc = CMP_SUCCESS;
  struct cmp_focus_ring *ring = NULL;

  if (!tree || !out_ring) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_focus_ring_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_focus_ring), (void **)&ring);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_focus_ring_create: Out of memory\n");
    return rc;
  }

  ring->tree = tree;
  ring->is_keyboard_mode = 0; /* Default to pointer/hidden */
  ring->currently_focused_id = -1;

  *out_ring = (cmp_focus_ring_t *)ring;
  return rc;
}

int cmp_focus_ring_destroy(cmp_focus_ring_t *ring) {
  int rc = CMP_SUCCESS;
  struct cmp_focus_ring *r = (struct cmp_focus_ring *)ring;

  if (!r) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_focus_ring_destroy: Invalid argument\n");
    return rc;
  }

  CMP_FREE(r);
  return rc;
}

int cmp_focus_ring_set_keyboard_mode(cmp_focus_ring_t *ring, int is_keyboard) {
  int rc = CMP_SUCCESS;
  struct cmp_focus_ring *r = (struct cmp_focus_ring *)ring;

  if (!r) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_focus_ring_set_keyboard_mode: Invalid argument\n");
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

  return rc;
}

int cmp_focus_ring_node_focused(cmp_focus_ring_t *ring, int node_id) {
  int rc = CMP_SUCCESS;
  struct cmp_focus_ring *r = (struct cmp_focus_ring *)ring;

  if (!r) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_focus_ring_node_focused: Invalid argument\n");
    return rc;
  }

  r->currently_focused_id = node_id;

  if (r->is_keyboard_mode) {
    /* The rendering system will draw a high-contrast ring around this node_id
     */
  }

  return rc;
}
