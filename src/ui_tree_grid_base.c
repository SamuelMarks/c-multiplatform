/**
 * @file ui_tree_grid_base.c
 * @brief Implementation of the tree grid base component.
 */

/* clang-format off */
#include "ui_tree_grid_base.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

#define MAX_EXPANDED_NODES 256

#define UI_TREE_GRID_IS_EXPAND_IGNORE(t, n, o)                                 \
  ui_tree_grid_base_is_expanded((t), (n), (o))

/**
 * @struct ui_tree_grid_base
 * @brief Internal state for the tree grid base component.
 */
struct ui_tree_grid_base {
  struct ui_component *component;
  struct ui_tree_grid_model model;

  void *expanded_nodes[MAX_EXPANDED_NODES];
  size_t expanded_count;

  void *active_node;
  size_t active_col;
};

ui_error_t ui_tree_grid_base_create(struct ui_tree_grid_base **out_tree_grid,
                                    const struct ui_tree_grid_model *model) {
  struct ui_tree_grid_base *tree_grid;
  ui_error_t rc;

  if (!out_tree_grid || !model) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tree_grid = (struct ui_tree_grid_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_tree_grid_base));
  if (!tree_grid) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_component_create(&tree_grid->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tree_grid);
    return rc;
  }

  tree_grid->model = *model;
  tree_grid->expanded_count = 0;
  tree_grid->active_node = NULL;
  tree_grid->active_col = 0;

  *out_tree_grid = tree_grid;
  return UI_ERROR_NONE;
}

ui_error_t ui_tree_grid_base_destroy(struct ui_tree_grid_base *tree_grid) {
  if (!tree_grid) {
    return UI_ERROR_NONE;
  }
  (void)ui_component_destroy(tree_grid->component);
  C_MULTIPLATFORM_FREE(tree_grid);
  return UI_ERROR_NONE;
}

ui_error_t
ui_tree_grid_base_get_component(struct ui_tree_grid_base *tree_grid,
                                struct ui_component **out_component) {
  if (!tree_grid || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = tree_grid->component;
  return UI_ERROR_NONE;
}

ui_error_t
ui_tree_grid_base_is_expanded(const struct ui_tree_grid_base *tree_grid,
                              void *node_id, int *out_is_expanded) {
  size_t i;
  if (!tree_grid || !out_is_expanded) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_expanded = 0;
  for (i = 0; i < tree_grid->expanded_count; i++) {
    if (tree_grid->expanded_nodes[i] == node_id) {
      *out_is_expanded = 1;
      break;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_tree_grid_base_set_expanded(struct ui_tree_grid_base *tree_grid,
                                          void *node_id, int expanded) {
  int currently_expanded = 0;
  size_t i;

  if (!tree_grid || !node_id) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)UI_TREE_GRID_IS_EXPAND_IGNORE(tree_grid, node_id, &currently_expanded);

  if (expanded && !currently_expanded) {
    if (tree_grid->expanded_count < MAX_EXPANDED_NODES) {
      tree_grid->expanded_nodes[tree_grid->expanded_count++] = node_id;
    } else {
      return UI_ERROR_OUT_OF_BOUNDS;
    }
  } else if (!expanded && currently_expanded) {
    size_t new_count = 0;
    for (i = 0; i < tree_grid->expanded_count; i++) {
      if (tree_grid->expanded_nodes[i] != node_id) {
        tree_grid->expanded_nodes[new_count++] = tree_grid->expanded_nodes[i];
      }
    }
    tree_grid->expanded_count = new_count;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_tree_grid_base_toggle_node(struct ui_tree_grid_base *tree_grid,
                                         void *node_id) {
  int expanded = 0;

  (void)UI_TREE_GRID_IS_EXPAND_IGNORE(tree_grid, node_id, &expanded);

  return ui_tree_grid_base_set_expanded(tree_grid, node_id, !expanded);
}

ui_error_t
ui_tree_grid_base_handle_key_event(struct ui_tree_grid_base *tree_grid,
                                   const struct ui_keyboard_event *event) {
  if (!tree_grid || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Real navigation logic would locate the node in the flattened visible list
   * and move 2D focus based on active_col and active_node. */

  if (event->key_code == UI_KEY_RIGHT) {
    if (tree_grid->active_node) {
      int expanded = 0;
      (void)UI_TREE_GRID_IS_EXPAND_IGNORE(tree_grid, tree_grid->active_node,
                                          &expanded);
      if (!expanded) {
        ui_error_t set_exp_rc = ui_tree_grid_base_set_expanded(
            tree_grid, tree_grid->active_node, 1);
        if (set_exp_rc != UI_ERROR_NONE)
          return set_exp_rc;
      } else {
        /* If already expanded, might move focus to first child or next column
         */
        tree_grid->active_col++;
      }
    }
  } else if (event->key_code == UI_KEY_LEFT) {
    if (tree_grid->active_node) {
      int expanded = 0;
      (void)UI_TREE_GRID_IS_EXPAND_IGNORE(tree_grid, tree_grid->active_node,
                                          &expanded);
      if (expanded) {
        (void)ui_tree_grid_base_set_expanded(tree_grid, tree_grid->active_node,
                                             0);
      } else {
        /* Move focus to parent or prev col */
        if (tree_grid->active_col > 0)
          tree_grid->active_col--;
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_tree_grid_base_render(struct ui_tree_grid_base *tree_grid,
                                    struct ui_dom_node *container) {
  if (!tree_grid || !container) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#define UI_DOM_SET_ATTR_IGNORE(n, a, v) ui_dom_node_set_attribute((n), (a), (v))
  (void)UI_DOM_SET_ATTR_IGNORE(container, "role", "treegrid");

  /* Rendering recursion would go here. */

  return UI_ERROR_NONE;
}
