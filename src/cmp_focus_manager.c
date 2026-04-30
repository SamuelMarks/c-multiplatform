/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
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

/**
 * @brief cmp_focus_manager_create
 *
 * @param tree Parameter description.
 * @param out_focus_manager Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_focus_manager_create(cmp_a11y_tree_t *tree,
                             cmp_focus_manager_t **out_focus_manager) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_focus_manager *mgr = NULL;

  if (tree == NULL || out_focus_manager == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_manager_create: Invalid argument: %s\n", err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_focus_manager), (void **)&mgr);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_manager_create: Out of memory: %s\n", err_str);

    return rc;
  }

  mgr->tree = tree;
  mgr->nodes = NULL;
  mgr->count = 0;
  mgr->capacity = 0;
  mgr->currently_focused_id = -1;

  *out_focus_manager = (cmp_focus_manager_t *)mgr;
  cmp_log_debug(
      "cmp_focus_manager_create: Successfully created focus manager context\n");
  return rc;
}

/**
 * @brief cmp_focus_manager_destroy
 *
 * @param focus_manager Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_focus_manager_destroy(cmp_focus_manager_t *focus_manager) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_focus_manager *mgr = (struct cmp_focus_manager *)focus_manager;

  if (mgr == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_manager_destroy: Invalid argument: %s\n", err_str);

    return rc;
  }

  if (mgr->nodes != NULL) {
    rc = CMP_FREE(mgr->nodes);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_focus_manager_destroy: CMP_FREE nodes failed\n");
    }
  }

  rc = CMP_FREE(mgr);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_focus_manager_destroy: CMP_FREE context failed\n");
  }

  cmp_log_debug("cmp_focus_manager_destroy: Successfully destroyed focus "
                "manager context\n");
  return rc;
}

/**
 * @brief cmp_focus_manager_set_focus
 *
 * @param focus_manager Parameter description.
 * @param node_id Parameter description.
 * @param has_focus Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_focus_manager_set_focus(cmp_focus_manager_t *focus_manager, int node_id,
                                int has_focus) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_focus_manager *mgr = (struct cmp_focus_manager *)focus_manager;
  cmp_focus_node_t *new_nodes = NULL;
  size_t new_capacity;
  size_t i;

  if (mgr == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_manager_set_focus: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  /* Check if it already exists to update it */
  for (i = 0; i < mgr->count; ++i) {
    if (mgr->nodes[i].node_id == node_id) {
      mgr->nodes[i].has_focus = has_focus;
      if (has_focus) {
        mgr->currently_focused_id = node_id;
      } else if (mgr->currently_focused_id == node_id) {
        mgr->currently_focused_id = -1;
      }
      cmp_log_debug(
          "cmp_focus_manager_set_focus: Updated focus state for node %d\n",
          node_id);
      return rc;
    }
  }

  /* Add new node */
  if (mgr->count >= mgr->capacity) {
    new_capacity = mgr->capacity == 0 ? 16 : mgr->capacity * 2;
    rc = CMP_MALLOC(new_capacity * sizeof(cmp_focus_node_t),
                    (void **)&new_nodes);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_focus_manager_set_focus: Out of memory: %s\n",
                    err_str);

      return rc;
    }

    if (mgr->nodes != NULL) {
      memcpy(new_nodes, mgr->nodes, mgr->count * sizeof(cmp_focus_node_t));
      rc = CMP_FREE(mgr->nodes);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug(
            "cmp_focus_manager_set_focus: CMP_FREE old nodes failed\n");
      }
    }
    mgr->nodes = new_nodes;
    mgr->capacity = new_capacity;
  }

  mgr->nodes[mgr->count].node_id = node_id;
  mgr->nodes[mgr->count].has_focus = has_focus;
  /* Just zero the rect for now, in a real implementation we'd grab it from
   * Layout Node */
  mgr->nodes[mgr->count].rect.x = 0.0f;
  mgr->nodes[mgr->count].rect.y = 0.0f;
  mgr->nodes[mgr->count].rect.width = 0.0f;
  mgr->nodes[mgr->count].rect.height = 0.0f;

  if (has_focus) {
    mgr->currently_focused_id = node_id;
  }

  mgr->count++;

  cmp_log_debug(
      "cmp_focus_manager_set_focus: Registered new node into focus graph\n");
  return rc;
}

/**
 * @brief cmp_focus_manager_navigate
 *
 * @param focus_manager Parameter description.
 * @param current_node_id Parameter description.
 * @param direction Parameter description.
 * @param out_next_node_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_focus_manager_navigate(cmp_focus_manager_t *focus_manager,
                               int current_node_id, int direction,
                               int *out_next_node_id) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_focus_manager *mgr = (struct cmp_focus_manager *)focus_manager;
  size_t i;
  int target_id = -1;
  float min_distance = 9999999.0f;
  cmp_focus_node_t *current_node = NULL;
  float dx;
  float dy;
  int is_valid_dir;
  float distance;

  if (mgr == NULL || out_next_node_id == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_focus_manager_navigate: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  if (mgr->count == 0) {
    *out_next_node_id = -1;
    return rc;
  }

  /* Find current node */
  for (i = 0; i < mgr->count; ++i) {
    if (mgr->nodes[i].node_id == current_node_id) {
      current_node = &mgr->nodes[i];
      break;
    }
  }

  if (current_node == NULL) {
    /* Current not found, just pick the first one */
    *out_next_node_id = mgr->nodes[0].node_id;
    return rc;
  }

  /* Extremely simplified 2D spatial intersection heuristics for TV Remotes */
  for (i = 0; i < mgr->count; ++i) {
    is_valid_dir = 0;

    if (mgr->nodes[i].node_id == current_node_id)
      continue;

    dx = mgr->nodes[i].rect.x - current_node->rect.x;
    dy = mgr->nodes[i].rect.y - current_node->rect.y;

    if (direction == 0 && dy < 0.0f)
      is_valid_dir = 1; /* Up */
    else if (direction == 1 && dy > 0.0f)
      is_valid_dir = 1; /* Down */
    else if (direction == 2 && dx < 0.0f)
      is_valid_dir = 1; /* Left */
    else if (direction == 3 && dx > 0.0f)
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

  cmp_log_debug("cmp_focus_manager_navigate: Computed 2D spatial focus "
                "navigation target\n");

  return rc;
}
