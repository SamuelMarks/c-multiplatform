/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

typedef struct cmp_focus_node {
  int node_id;
  int has_focus;
  cmp_rect_t rect; /* Used for spatial navigation */
} cmp_focus_node_t;

struct cmp_focus_manager {
  cmp_a11y_tree_t *tree;
  cmp_focus_node_t *nodes;
  size_t count;
  size_t capacity;
  int currently_focused_id;
};

int cmp_focus_manager_create(cmp_a11y_tree_t *tree,
                             cmp_focus_manager_t **out_focus_manager) {
  struct cmp_focus_manager *mgr;

  if (!tree || !out_focus_manager)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_focus_manager), (void **)&mgr) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  mgr->tree = tree;
  mgr->nodes = NULL;
  mgr->count = 0;
  mgr->capacity = 0;
  mgr->currently_focused_id = -1;

  *out_focus_manager = (cmp_focus_manager_t *)mgr;
  return CMP_SUCCESS;
}

int cmp_focus_manager_destroy(cmp_focus_manager_t *focus_manager) {
  struct cmp_focus_manager *mgr = (struct cmp_focus_manager *)focus_manager;

  if (!mgr)
    return CMP_ERROR_INVALID_ARG;

  if (mgr->nodes) {
    CMP_FREE(mgr->nodes);
  }

  CMP_FREE(mgr);
  return CMP_SUCCESS;
}

int cmp_focus_manager_set_focus(cmp_focus_manager_t *focus_manager, int node_id,
                                int has_focus) {
  struct cmp_focus_manager *mgr = (struct cmp_focus_manager *)focus_manager;
  cmp_focus_node_t *new_nodes;
  size_t new_capacity;
  size_t i;

  if (!mgr)
    return CMP_ERROR_INVALID_ARG;

  /* Check if it already exists to update it */
  for (i = 0; i < mgr->count; ++i) {
    if (mgr->nodes[i].node_id == node_id) {
      mgr->nodes[i].has_focus = has_focus;
      if (has_focus) {
        mgr->currently_focused_id = node_id;
      } else if (mgr->currently_focused_id == node_id) {
        mgr->currently_focused_id = -1;
      }
      return CMP_SUCCESS;
    }
  }

  /* Add new node */
  if (mgr->count >= mgr->capacity) {
    new_capacity = mgr->capacity == 0 ? 16 : mgr->capacity * 2;
    if (CMP_MALLOC(new_capacity * sizeof(cmp_focus_node_t),
                   (void **)&new_nodes) != CMP_SUCCESS)
      return CMP_ERROR_OOM;

    if (mgr->nodes) {
      memcpy(new_nodes, mgr->nodes, mgr->count * sizeof(cmp_focus_node_t));
      CMP_FREE(mgr->nodes);
    }
    mgr->nodes = new_nodes;
    mgr->capacity = new_capacity;
  }

  mgr->nodes[mgr->count].node_id = node_id;
  mgr->nodes[mgr->count].has_focus = has_focus;
  /* Just zero the rect for now, in a real implementation we'd grab it from
   * Layout Node */
  mgr->nodes[mgr->count].rect.x = 0;
  mgr->nodes[mgr->count].rect.y = 0;
  mgr->nodes[mgr->count].rect.width = 0;
  mgr->nodes[mgr->count].rect.height = 0;

  if (has_focus) {
    mgr->currently_focused_id = node_id;
  }

  mgr->count++;

  return CMP_SUCCESS;
}

int cmp_focus_manager_navigate(cmp_focus_manager_t *focus_manager,
                               int current_node_id, int direction,
                               int *out_next_node_id) {
  struct cmp_focus_manager *mgr = (struct cmp_focus_manager *)focus_manager;
  size_t i;
  int target_id = -1;
  float min_distance = 9999999.0f;
  cmp_focus_node_t *current_node = NULL;

  if (!mgr || !out_next_node_id)
    return CMP_ERROR_INVALID_ARG;

  if (mgr->count == 0) {
    *out_next_node_id = -1;
    return CMP_SUCCESS;
  }

  /* Find current node */
  for (i = 0; i < mgr->count; ++i) {
    if (mgr->nodes[i].node_id == current_node_id) {
      current_node = &mgr->nodes[i];
      break;
    }
  }

  if (!current_node) {
    /* Current not found, just pick the first one */
    *out_next_node_id = mgr->nodes[0].node_id;
    return CMP_SUCCESS;
  }

  /* Extremely simplified 2D spatial intersection heuristics for TV Remotes */
  for (i = 0; i < mgr->count; ++i) {
    float dx;
    float dy;
    int is_valid_dir = 0;
    float distance;

    if (mgr->nodes[i].node_id == current_node_id)
      continue;

    dx = mgr->nodes[i].rect.x - current_node->rect.x;
    dy = mgr->nodes[i].rect.y - current_node->rect.y;

    if (direction == 0 && dy < 0)
      is_valid_dir = 1; /* Up */
    else if (direction == 1 && dy > 0)
      is_valid_dir = 1; /* Down */
    else if (direction == 2 && dx < 0)
      is_valid_dir = 1; /* Left */
    else if (direction == 3 && dx > 0)
      is_valid_dir = 1; /* Right */

    if (is_valid_dir) {
      distance = (dx * dx) + (dy * dy);
      if (distance < min_distance) {
        min_distance = distance;
        target_id = mgr->nodes[i].node_id;
      }
    }
  }

  /* Fallback: if we didn't find anything in that direction, loop back or return
   * current */
  if (target_id == -1) {
    *out_next_node_id = current_node_id;
  } else {
    *out_next_node_id = target_id;
  }

  return CMP_SUCCESS;
}
