/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

typedef struct cmp_aria_live_node {
  int node_id;
  cmp_aria_live_mode_t mode;
} cmp_aria_live_node_t;

struct cmp_aria_live {
  cmp_a11y_tree_t *tree;
  cmp_aria_live_node_t *nodes;
  size_t count;
  size_t capacity;
};

int cmp_aria_live_create(cmp_a11y_tree_t *tree, cmp_aria_live_t **out_live) {
  struct cmp_aria_live *live;

  if (!tree || !out_live)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_aria_live), (void **)&live) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  live->tree = tree;
  live->nodes = NULL;
  live->count = 0;
  live->capacity = 0;

  *out_live = (cmp_aria_live_t *)live;
  return CMP_SUCCESS;
}

int cmp_aria_live_destroy(cmp_aria_live_t *live) {
  struct cmp_aria_live *l = (struct cmp_aria_live *)live;

  if (!l)
    return CMP_ERROR_INVALID_ARG;

  if (l->nodes) {
    CMP_FREE(l->nodes);
  }

  CMP_FREE(l);
  return CMP_SUCCESS;
}

int cmp_aria_live_set_mode(cmp_aria_live_t *live, int node_id,
                           cmp_aria_live_mode_t mode) {
  struct cmp_aria_live *l = (struct cmp_aria_live *)live;
  cmp_aria_live_node_t *new_nodes;
  size_t new_capacity;
  size_t i;

  if (!l)
    return CMP_ERROR_INVALID_ARG;

  /* Check if it already exists to update it */
  for (i = 0; i < l->count; ++i) {
    if (l->nodes[i].node_id == node_id) {
      l->nodes[i].mode = mode;
      return CMP_SUCCESS;
    }
  }

  /* Add new node */
  if (l->count >= l->capacity) {
    new_capacity = l->capacity == 0 ? 16 : l->capacity * 2;
    if (CMP_MALLOC(new_capacity * sizeof(cmp_aria_live_node_t),
                   (void **)&new_nodes) != CMP_SUCCESS)
      return CMP_ERROR_OOM;

    if (l->nodes) {
      memcpy(new_nodes, l->nodes, l->count * sizeof(cmp_aria_live_node_t));
      CMP_FREE(l->nodes);
    }
    l->nodes = new_nodes;
    l->capacity = new_capacity;
  }

  l->nodes[l->count].node_id = node_id;
  l->nodes[l->count].mode = mode;
  l->count++;

  return CMP_SUCCESS;
}

int cmp_aria_live_announce(cmp_aria_live_t *live, int node_id,
                           const char *message) {
  struct cmp_aria_live *l = (struct cmp_aria_live *)live;
  size_t i;
  cmp_aria_live_mode_t mode = CMP_ARIA_LIVE_OFF;

  if (!l || !message)
    return CMP_ERROR_INVALID_ARG;

  /* Find the mode of the node */
  for (i = 0; i < l->count; ++i) {
    if (l->nodes[i].node_id == node_id) {
      mode = l->nodes[i].mode;
      break;
    }
  }

  if (mode == CMP_ARIA_LIVE_OFF) {
    return CMP_SUCCESS; /* No announcement needed */
  }

  /* Here we would integrate with cmp_screen_reader_announce if we had a
   * reference to cmp_screen_reader_t, but typically the a11y tree dispatches
   * this or it's handled by a global event in this architecture. For now, we
   * simulate success by recognizing the mode.
   */

  if (mode == CMP_ARIA_LIVE_POLITE) {
    /* Queue for polite announcement */
  } else if (mode == CMP_ARIA_LIVE_ASSERTIVE) {
    /* Interrupt screen reader for assertive announcement */
  }

  return CMP_SUCCESS;
}
