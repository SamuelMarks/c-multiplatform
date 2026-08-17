/**
 * \file ui_focus_manager.c
 * \brief Implementation of the UI focus manager.
 */
/* clang-format off */
#include "ui_focus_manager.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * \struct ui_focus_trap
 * \brief Represents a focus trap boundary.
 */
struct ui_focus_trap {
  struct ui_dom_node *root;
  struct ui_dom_node *previously_focused;
};

/**
 * \struct ui_focus_manager
 * \brief Manages focus state and navigation within the UI.
 */
struct ui_focus_manager {
  struct ui_dom_node *focused_node;
  struct ui_focus_trap *traps;
  size_t traps_count;
  size_t traps_capacity;
};

/**
 * \brief Creates a focus manager.
 * \param[out] out_manager Pointer to store the created focus manager.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_focus_manager_create(struct ui_focus_manager **out_manager) {
  struct ui_focus_manager *mgr;

  if (!out_manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  mgr = (struct ui_focus_manager *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_focus_manager));
  if (!mgr) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  mgr->focused_node = NULL;
  mgr->traps = NULL;
  mgr->traps_count = 0;
  mgr->traps_capacity = 0;

  *out_manager = mgr;
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a focus manager.
 * \param[in,out] manager The focus manager to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_focus_manager_destroy(struct ui_focus_manager *manager) {
  if (!manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (manager->traps) {
    C_MULTIPLATFORM_FREE(manager->traps);
  }
  C_MULTIPLATFORM_FREE(manager);
  return UI_ERROR_NONE;
}

/**
 * \brief Requests focus for a specific DOM node.
 * \param[in,out] manager The focus manager.
 * \param[in] node The node to focus.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_focus_manager_request_focus(struct ui_focus_manager *manager,
                                          struct ui_dom_node *node) {
  if (!manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  manager->focused_node = node;
  return UI_ERROR_NONE;
}

/**
 * \brief Gets the currently focused DOM node.
 * \param[in] manager The focus manager.
 * \param[out] out_node Pointer to store the focused node.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_focus_manager_get_focused_node(const struct ui_focus_manager *manager,
                                  struct ui_dom_node **out_node) {
  if (!manager || !out_node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_node = manager->focused_node;
  return UI_ERROR_NONE;
}

/**
 * \brief Checks if a DOM node is focusable based on tabindex.
 * \param[in] node The DOM node to check.
 * \param[out] out_focusable Set to UI_TRUE if focusable, UI_FALSE otherwise.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t is_focusable(struct ui_dom_node *node,
                               ui_bool_t *out_focusable) {
  const char *tabindex_val = NULL;

  *out_focusable = UI_FALSE;

  if (node->type != UI_DOM_NODE_TYPE_ELEMENT) {
    return UI_ERROR_NONE;
  }
  {
    ui_error_t rc = ui_dom_node_get_attribute(node, "tabindex", &tabindex_val);
    if (rc == UI_ERROR_NONE) {
      int index = atoi(tabindex_val);
      if (index >= 0) {
        *out_focusable = UI_TRUE;
      }
    }
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Recursively gathers focusable DOM nodes from a root node.
 * \param[in] root The root DOM node.
 * \param[in,out] out_nodes Pointer to a dynamically allocated array of
 * focusable nodes.
 * \param[in,out] out_count Number of nodes found.
 * \param[in,out] out_capacity Capacity of the nodes array.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t gather_focusable_nodes(struct ui_dom_node *root,
                                         struct ui_dom_node ***out_nodes,
                                         size_t *out_count,
                                         size_t *out_capacity) {
  struct ui_dom_node *child;
  ui_bool_t focusable = UI_FALSE;
  ui_error_t rc;

  { (void)is_focusable(root, &focusable); }

  if (focusable) {
    if (*out_count >= *out_capacity) {
      size_t new_cap = (*out_capacity == 0) ? 16 : (*out_capacity * 2);
      struct ui_dom_node **new_arr =
          (struct ui_dom_node **)C_MULTIPLATFORM_REALLOC(
              *out_nodes, new_cap * sizeof(struct ui_dom_node *));
      if (new_arr) {
        *out_nodes = new_arr;
        *out_capacity = new_cap;
      } else {
        return UI_ERROR_OUT_OF_MEMORY;
      }
    }
    (*out_nodes)[(*out_count)++] = root;
  }

  child = root->first_child;
  while (child) {
    {
      ui_error_t g_rc =
          gather_focusable_nodes(child, out_nodes, out_count, out_capacity);
      if (g_rc != UI_ERROR_NONE) {
        return g_rc;
      }
    }
    child = child->next_sibling;
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Advances focus sequentially (e.g., via Tab key).
 * \param[in,out] manager The focus manager.
 * \param[in] root The root DOM node to search within.
 * \param[in] forward 1 to advance forward, 0 to advance backward.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_focus_manager_advance(struct ui_focus_manager *manager,
                                    struct ui_dom_node *root, int forward) {
  struct ui_dom_node *target_root;
  struct ui_dom_node **nodes = NULL;
  size_t count = 0;
  size_t capacity = 0;
  size_t i;
  int current_index = -1;
  ui_error_t rc;

  if (!manager || !root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  target_root = root;
  if (manager->traps_count > 0) {
    target_root = manager->traps[manager->traps_count - 1].root;
  }

  gather_focusable_nodes(target_root, &nodes, &count, &capacity);

  if (count == 0) {
    return UI_ERROR_NONE;
  }

  for (i = 0; i < count; ++i) {
    if (nodes[i] == manager->focused_node) {
      current_index = (int)i;
      break;
    }
  }

  if (forward) {
    if (current_index == -1 || current_index == (int)(count - 1)) {
      manager->focused_node = nodes[0];
    } else {
      manager->focused_node = nodes[current_index + 1];
    }
  } else {
    if (current_index == -1 || current_index == 0) {
      manager->focused_node = nodes[count - 1];
    } else {
      manager->focused_node = nodes[current_index - 1];
    }
  }

  C_MULTIPLATFORM_FREE(nodes);

  return UI_ERROR_NONE;
}

/**
 * \brief Recursively gathers focusable layout nodes from a root node.
 * \param[in] node The root layout node.
 * \param[in,out] out_nodes Pointer to a dynamically allocated array of
 * focusable nodes.
 * \param[in,out] out_count Number of nodes found.
 * \param[in,out] out_capacity Capacity of the nodes array.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t
gather_focusable_layout_nodes(struct ui_layout_node *node,
                              struct ui_layout_node ***out_nodes,
                              size_t *out_count, size_t *out_capacity) {
  struct ui_layout_node *child;
  ui_bool_t focusable = UI_FALSE;
  ui_error_t rc = UI_ERROR_NONE;

  if (node->dom_node) {
    {
      (void)is_focusable((struct ui_dom_node *)node->dom_node, &focusable);
    }
  }

  if (focusable) {
    if (*out_count >= *out_capacity) {
      size_t new_cap = (*out_capacity == 0) ? 16 : (*out_capacity * 2);
      struct ui_layout_node **new_arr =
          (struct ui_layout_node **)C_MULTIPLATFORM_REALLOC(
              *out_nodes, new_cap * sizeof(struct ui_layout_node *));
      if (new_arr) {
        *out_nodes = new_arr;
        *out_capacity = new_cap;
      } else {
        return UI_ERROR_OUT_OF_MEMORY;
      }
    }
    (*out_nodes)[(*out_count)++] = node;
  }

  child = node->first_child;
  while (child) {
    {
      ui_error_t g_rc = gather_focusable_layout_nodes(child, out_nodes,
                                                      out_count, out_capacity);
      if (g_rc != UI_ERROR_NONE) {
        return g_rc;
      }
    }
    child = child->next_sibling;
  }
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

/**
 * \brief Calculates the spatial distance penalty for navigating between layout
 * nodes.
 * \param[in] a The starting layout node.
 * \param[in] b The target layout node.
 * \param[in] dir The direction of navigation.
 * \param[out] out_distance Computed distance penalty. -1.0f if invalid.
 */
static void get_distance(struct ui_layout_node *a, struct ui_layout_node *b,
                         enum ui_focus_direction dir, float *out_distance) {
  float dx = 0.0f, dy = 0.0f;
  float center_ax, center_ay, center_bx, center_by;

  center_ax = a->x + a->width * 0.5f;
  center_ay = a->y + a->height * 0.5f;
  center_bx = b->x + b->width * 0.5f;
  center_by = b->y + b->height * 0.5f;

  *out_distance = -1.0f;

  switch (dir) {
  case UI_FOCUS_DIRECTION_UP:
    if (center_by >= center_ay)
      return; /* Must be strictly above */
    dx = center_ax - center_bx;
    dy = center_ay - center_by;
    break;
  case UI_FOCUS_DIRECTION_DOWN:
    if (center_by <= center_ay)
      return; /* Must be strictly below */
    dx = center_ax - center_bx;
    dy = center_by - center_ay;
    break;
  case UI_FOCUS_DIRECTION_LEFT:
    if (center_bx >= center_ax)
      return; /* Must be strictly left */
    dx = center_ax - center_bx;
    dy = center_ay - center_by;
    break;
  case UI_FOCUS_DIRECTION_RIGHT:
    if (center_bx <= center_ax)
      return; /* Must be strictly right */
    dx = center_bx - center_ax;
    dy = center_ay - center_by;
    break;
  default:
    break;
  }

  /* Give heavy penalty to perpendicular distance */
  *out_distance = dy * dy + dx * dx * 4.0f;
  return;
}

/**
 * \brief Navigates focus spatially (e.g., via arrow keys).
 * \param[in,out] manager The focus manager.
 * \param[in] layout_root The root layout node to search within.
 * \param[in] direction The direction to navigate.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_focus_manager_navigate(struct ui_focus_manager *manager,
                                     struct ui_layout_node *layout_root,
                                     enum ui_focus_direction direction) {
  struct ui_layout_node **nodes = NULL;
  size_t count = 0;
  size_t capacity = 0;
  size_t i;
  struct ui_layout_node *current_layout_node = NULL;
  struct ui_layout_node *best_node = NULL;
  float best_distance = -1.0f;
  ui_error_t rc;

  if (!manager || !layout_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!manager->focused_node) {
    return UI_ERROR_NONE; /* No starting point for spatial navigation */
  }

  {
    ui_error_t g_rc =
        gather_focusable_layout_nodes(layout_root, &nodes, &count, &capacity);
    if (g_rc != UI_ERROR_NONE) {
      return g_rc;
    }
  }

  for (i = 0; i < count; ++i) {
    if (nodes[i]->dom_node == manager->focused_node) {
      current_layout_node = nodes[i];
      break;
    }
  }

  if (!current_layout_node) {
    if (nodes)
      C_MULTIPLATFORM_FREE(nodes);
    return UI_ERROR_NONE;
  }

  for (i = 0; i < count; ++i) {
    float dist;
    if (nodes[i] == current_layout_node)
      continue;

    /* If there's a trap, only consider nodes inside the trap */
    if (manager->traps_count > 0) {
      struct ui_dom_node *trap_root =
          manager->traps[manager->traps_count - 1].root;
      struct ui_dom_node *ancestor = (struct ui_dom_node *)nodes[i]->dom_node;
      int in_trap = 0;
      while (ancestor) {
        if (ancestor == trap_root) {
          in_trap = 1;
          break;
        }
        ancestor = ancestor->parent;
      }
      if (!in_trap)
        continue;
    }

    get_distance(current_layout_node, nodes[i], direction, &dist);

    if (dist >= 0.0f) {
      if (best_distance < 0.0f || dist < best_distance) {
        best_distance = dist;
        best_node = nodes[i];
      }
    }
  }

  if (best_node) {
    manager->focused_node = (struct ui_dom_node *)best_node->dom_node;
  }

  C_MULTIPLATFORM_FREE(nodes);

  return UI_ERROR_NONE;
}

/**
 * \brief Pushes a new focus trap onto the focus manager.
 * \param[in,out] manager The focus manager.
 * \param[in] trap_root The root node of the trap.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_focus_manager_push_trap(struct ui_focus_manager *manager,
                                      struct ui_dom_node *trap_root) {
  if (!manager || !trap_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (manager->traps_count >= manager->traps_capacity) {
    size_t new_cap =
        (manager->traps_capacity == 0) ? 4 : (manager->traps_capacity * 2);
    struct ui_focus_trap *new_arr =
        (struct ui_focus_trap *)C_MULTIPLATFORM_REALLOC(
            manager->traps, new_cap * sizeof(struct ui_focus_trap));
    if (!new_arr) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    manager->traps = new_arr;
    manager->traps_capacity = new_cap;
  }

  manager->traps[manager->traps_count].root = trap_root;
  manager->traps[manager->traps_count].previously_focused =
      manager->focused_node;
  manager->traps_count++;

  /* Reset focus to the trap root or its first focusable element */
  (void)ui_focus_manager_advance(manager, trap_root, 1);

  return UI_ERROR_NONE;
}

/**
 * \brief Pops the current focus trap from the focus manager.
 * \param[in,out] manager The focus manager.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_focus_manager_pop_trap(struct ui_focus_manager *manager) {
  struct ui_focus_trap *popped;

  if (!manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (manager->traps_count == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  popped = &manager->traps[manager->traps_count - 1];
  manager->focused_node = popped->previously_focused;
  manager->traps_count--;

  return UI_ERROR_NONE;
}
