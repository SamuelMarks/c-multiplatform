/* clang-format off */
#include "ui_tree_grid_base.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

#define MAX_EXPANDED_NODES 256

struct ui_tree_grid_base {
  struct ui_component *component;
  struct ui_tree_grid_model model;

  void *expanded_nodes[MAX_EXPANDED_NODES];
  size_t expanded_count;

  void *active_node;
  size_t active_col;
};

enum ui_error ui_tree_grid_base_create(struct ui_tree_grid_base **out_tree_grid,
                                       const struct ui_tree_grid_model *model) {
  struct ui_tree_grid_base *tree_grid;
  enum ui_error rc;

  if (!out_tree_grid || !model) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tree_grid =
      (struct ui_tree_grid_base *)UI_MALLOC(sizeof(struct ui_tree_grid_base));
  if (!tree_grid) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_component_create(&tree_grid->component);
  if (rc != UI_ERROR_NONE) {
    UI_FREE(tree_grid);
    return rc;
  }

  tree_grid->model = *model;
  tree_grid->expanded_count = 0;
  tree_grid->active_node = NULL;
  tree_grid->active_col = 0;

  *out_tree_grid = tree_grid;
  return UI_ERROR_NONE;
}

void ui_tree_grid_base_destroy(struct ui_tree_grid_base *tree_grid) {
  if (!tree_grid) {
    return;
  }
  ui_component_destroy(tree_grid->component);
  UI_FREE(tree_grid);
}

/** \brief ui_error */
enum ui_error
ui_tree_grid_base_get_component(struct ui_tree_grid_base *tree_grid,
                                struct ui_component **out_component) {
  if (!tree_grid || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = tree_grid->component;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
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

/** \brief ui_error */
enum ui_error
ui_tree_grid_base_set_expanded(struct ui_tree_grid_base *tree_grid,
                               void *node_id, int expanded) {
  int currently_expanded;
  size_t i;
  enum ui_error rc;

  if (!tree_grid || !node_id) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_tree_grid_base_is_expanded(tree_grid, node_id, &currently_expanded);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (expanded && !currently_expanded) {
    if (tree_grid->expanded_count < MAX_EXPANDED_NODES) {
      tree_grid->expanded_nodes[tree_grid->expanded_count++] = node_id;
    } else {
      return UI_ERROR_OUT_OF_BOUNDS;
    }
  } else if (!expanded && currently_expanded) {
    for (i = 0; i < tree_grid->expanded_count; i++) {
      if (tree_grid->expanded_nodes[i] == node_id) {
        tree_grid->expanded_nodes[i] =
            tree_grid->expanded_nodes[tree_grid->expanded_count - 1];
        tree_grid->expanded_count--;
        break;
      }
    }
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_tree_grid_base_toggle_node(struct ui_tree_grid_base *tree_grid,
                                            void *node_id) {
  int expanded;
  enum ui_error rc;

  rc = ui_tree_grid_base_is_expanded(tree_grid, node_id, &expanded);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_tree_grid_base_set_expanded(tree_grid, node_id, !expanded);
}

/** \brief ui_error */
enum ui_error
ui_tree_grid_base_handle_key_event(struct ui_tree_grid_base *tree_grid,
                                   const struct ui_keyboard_event *event) {
  if (!tree_grid || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Real navigation logic would locate the node in the flattened visible list
   * and move 2D focus based on active_col and active_node. */

  if (event->key_code == UI_KEY_RIGHT) {
    if (tree_grid->active_node) {
      int expanded;
      ui_tree_grid_base_is_expanded(tree_grid, tree_grid->active_node,
                                    &expanded);
      if (!expanded) {
        ui_tree_grid_base_set_expanded(tree_grid, tree_grid->active_node, 1);
      } else {
        /* If already expanded, might move focus to first child or next column
         */
        tree_grid->active_col++;
      }
    }
  } else if (event->key_code == UI_KEY_LEFT) {
    if (tree_grid->active_node) {
      int expanded;
      ui_tree_grid_base_is_expanded(tree_grid, tree_grid->active_node,
                                    &expanded);
      if (expanded) {
        ui_tree_grid_base_set_expanded(tree_grid, tree_grid->active_node, 0);
      } else {
        /* Move focus to parent or prev col */
        if (tree_grid->active_col > 0)
          tree_grid->active_col--;
      }
    }
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_tree_grid_base_render(struct ui_tree_grid_base *tree_grid,
                                       struct ui_dom_node *container) {
  if (!tree_grid || !container) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_dom_node_set_attribute(container, "role", "treegrid");

  /* Rendering recursion would go here. */

  return UI_ERROR_NONE;
}
