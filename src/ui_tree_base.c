/**
 * @file ui_tree_base.c
 * @brief Implementation of the tree base component.
 */

/* clang-format off */
#include "ui_tree_base.h"
#include "ui_internal_mem.h"
#include "ui_dom_node.h"
#include <stdio.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

#define UI_TREE_IS_EXPAND_IGNORE(t, n, o)                                      \
  ui_tree_base_is_expanded((t), (n), (o))

/**
 * @struct ui_tree_base
 * @brief Internal state for the tree base component.
 */
struct ui_tree_base {
  struct ui_tree_model model;
  void **expanded_nodes;
  size_t num_expanded;
  size_t expanded_cap;
  void *active_node;
  struct ui_selection_model *selection_model;
  struct ui_computed *data_signal;
};

ui_error_t ui_tree_base_create(struct ui_tree_base **out_tree,
                               const struct ui_tree_model *model) {
  struct ui_tree_base *tree;
  ui_error_t rc;

  if (!out_tree || !model || !model->get_root_count || !model->get_root_node ||
      !model->get_parent || !model->get_child_count || !model->get_child ||
      !model->render_node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tree = (struct ui_tree_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_tree_base));
  if (!tree)
    return UI_ERROR_OUT_OF_MEMORY;

  tree->model = *model;
  tree->expanded_nodes = NULL;
  tree->num_expanded = 0;
  tree->expanded_cap = 0;
  tree->active_node = NULL;

  rc = ui_selection_model_create(&tree->selection_model);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tree);
    return rc;
  }

  *out_tree = tree;
  return UI_ERROR_NONE;
}

ui_error_t ui_tree_base_destroy(struct ui_tree_base *tree) {
  if (!tree)
    return UI_ERROR_NONE;
  if (tree->selection_model) {
    ui_selection_model_destroy(tree->selection_model);
  }
  if (tree->expanded_nodes) {
    C_MULTIPLATFORM_FREE(tree->expanded_nodes);
  }
  C_MULTIPLATFORM_FREE(tree);
  return UI_ERROR_NONE;
}

ui_error_t
ui_tree_base_get_selection_model(struct ui_tree_base *tree,
                                 struct ui_selection_model **out_model) {
  if (!tree || !out_model) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_model = tree->selection_model;
  return UI_ERROR_NONE;
}

ui_error_t ui_tree_base_is_expanded(const struct ui_tree_base *tree,
                                    void *node_id, int *out_is_expanded) {
  size_t i;
  if (!tree || !node_id || !out_is_expanded)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_is_expanded = 0;
  for (i = 0; i < tree->num_expanded; i++) {
    if (tree->expanded_nodes[i] == node_id) {
      *out_is_expanded = 1;
      return UI_ERROR_NONE;
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_tree_base_set_expanded(struct ui_tree_base *tree, void *node_id,
                                     int expanded) {
  size_t i;
  int currently_expanded;

  if (!tree || !node_id)
    return UI_ERROR_INVALID_ARGUMENT;

  (void)UI_TREE_IS_EXPAND_IGNORE(tree, node_id, &currently_expanded);

  if (expanded) {
    if (!currently_expanded) {
      if (tree->num_expanded >= tree->expanded_cap) {
        size_t new_cap = tree->expanded_cap == 0 ? 8 : tree->expanded_cap * 2;
        void **new_arr = (void **)C_MULTIPLATFORM_REALLOC(
            tree->expanded_nodes, new_cap * sizeof(void *));
        if (!new_arr)
          return UI_ERROR_OUT_OF_MEMORY;
        tree->expanded_nodes = new_arr;
        tree->expanded_cap = new_cap;
      }
      tree->expanded_nodes[tree->num_expanded++] = node_id;
    }
  } else {
    if (currently_expanded) {
      for (i = 0;; i++) {
        if (tree->expanded_nodes[i] == node_id) {
          tree->expanded_nodes[i] =
              tree->expanded_nodes[tree->num_expanded - 1];
          tree->num_expanded--;
          break;
        }
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_tree_base_toggle_node(struct ui_tree_base *tree, void *node_id) {
  if (!tree || !node_id)
    return UI_ERROR_INVALID_ARGUMENT;
  {
    int is_expanded = 0;
    (void)UI_TREE_IS_EXPAND_IGNORE(tree, node_id, &is_expanded);
    return ui_tree_base_set_expanded(tree, node_id, !is_expanded);
  }
}

ui_error_t ui_tree_base_set_active_node(struct ui_tree_base *tree,
                                        void *node_id) {
  if (!tree)
    return UI_ERROR_INVALID_ARGUMENT;
  tree->active_node = node_id;
  return UI_ERROR_NONE;
}

ui_error_t ui_tree_base_get_active_node(const struct ui_tree_base *tree,
                                        void **out_node) {
  if (!tree || !out_node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_node = tree->active_node;
  return UI_ERROR_NONE;
}

/**
 * @brief Helper to find a node's index among its siblings.
 * @param tree The tree instance.
 * @param parent The parent node identifier.
 * @param node The node identifier to find.
 * @return The 0-based index of the node, or 0 if not found.
 */
static size_t get_node_index(struct ui_tree_base *tree, void *parent,
                             void *node) {
  size_t count =
      parent ? tree->model.get_child_count(parent, tree->model.user_data)
             : tree->model.get_root_count(tree->model.user_data);
  size_t i;
  for (i = 0; i < count; i++) {
    void *sib = parent ? tree->model.get_child(parent, i, tree->model.user_data)
                       : tree->model.get_root_node(i, tree->model.user_data);
    if (sib == node)
      return i;
  }
  return 0;
}

/**
 * @brief Gets the next visible node in a pre-order traversal.
 * @param tree The tree instance.
 * @param node The current node identifier.
 * @return The next visible node, or NULL if none.
 */
static void *get_next_visible_node(struct ui_tree_base *tree, void *node) {
  void *parent;
  size_t idx, count;

  {
    int is_expanded = 0;
    (void)UI_TREE_IS_EXPAND_IGNORE(tree, node, &is_expanded);
    if (is_expanded &&
        tree->model.get_child_count(node, tree->model.user_data) > 0) {
      return tree->model.get_child(node, 0, tree->model.user_data);
    }
  }

  while (node) {
    parent = tree->model.get_parent(node, tree->model.user_data);
    idx = get_node_index(tree, parent, node);
    count = parent ? tree->model.get_child_count(parent, tree->model.user_data)
                   : tree->model.get_root_count(tree->model.user_data);

    if (idx + 1 < count) {
      return parent
                 ? tree->model.get_child(parent, idx + 1, tree->model.user_data)
                 : tree->model.get_root_node(idx + 1, tree->model.user_data);
    }
    node = parent;
  }
  return NULL;
}

/**
 * @brief Gets the previous visible node in a pre-order traversal.
 * @param tree The tree instance.
 * @param node The current node identifier.
 * @return The previous visible node, or NULL if none.
 */
static void *get_prev_visible_node(struct ui_tree_base *tree, void *node) {
  void *parent = tree->model.get_parent(node, tree->model.user_data);
  size_t idx = get_node_index(tree, parent, node);

  if (idx > 0) {
    void *prev_sib =
        parent ? tree->model.get_child(parent, idx - 1, tree->model.user_data)
               : tree->model.get_root_node(idx - 1, tree->model.user_data);

    while (1) {
      int is_expanded = 0;
      (void)UI_TREE_IS_EXPAND_IGNORE(tree, prev_sib, &is_expanded);
      if (is_expanded &&
          tree->model.get_child_count(prev_sib, tree->model.user_data) > 0) {
        size_t count =
            tree->model.get_child_count(prev_sib, tree->model.user_data);
        prev_sib =
            tree->model.get_child(prev_sib, count - 1, tree->model.user_data);
      } else {
        break;
      }
    }
    return prev_sib;
  }
  return parent;
}

ui_error_t
ui_tree_base_handle_key_event(struct ui_tree_base *tree,
                              const struct ui_keyboard_event *event) {
  void *next_node = NULL;

  if (!tree || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!tree->active_node) {
    if (tree->model.get_root_count(tree->model.user_data) > 0) {
      tree->active_node = tree->model.get_root_node(0, tree->model.user_data);
    }
    return UI_ERROR_NONE;
  }

  switch (event->key_code) {
  case UI_KEY_DOWN:
    next_node = get_next_visible_node(tree, tree->active_node);
    if (next_node)
      tree->active_node = next_node;
    break;

  case UI_KEY_UP:
    next_node = get_prev_visible_node(tree, tree->active_node);
    if (next_node)
      tree->active_node = next_node;
    break;

  case UI_KEY_RIGHT:
    if (tree->model.get_child_count(tree->active_node, tree->model.user_data) >
        0) {
      {
        int is_expanded = 0;
        (void)UI_TREE_IS_EXPAND_IGNORE(tree, tree->active_node, &is_expanded);
        if (!is_expanded) {
          ui_error_t set_rc =
              ui_tree_base_set_expanded(tree, tree->active_node, 1);
          if (set_rc != UI_ERROR_NONE)
            return set_rc;
        } else {
          tree->active_node = tree->model.get_child(tree->active_node, 0,
                                                    tree->model.user_data);
        }
      }
    }
    break;

  case UI_KEY_LEFT: {
    int is_expanded = 0;
    (void)UI_TREE_IS_EXPAND_IGNORE(tree, tree->active_node, &is_expanded);
    if (is_expanded) {
      (void)ui_tree_base_set_expanded(tree, tree->active_node, 0);
    } else {
      void *parent =
          tree->model.get_parent(tree->active_node, tree->model.user_data);
      if (parent)
        tree->active_node = parent;
    }
  } break;

  case UI_KEY_ENTER:
  case UI_KEY_SPACE: {
    ui_error_t tog_rc;
    ui_error_t set_rc =
        ui_selection_model_toggle(tree->selection_model, tree->active_node);
    if (set_rc != UI_ERROR_NONE)
      return set_rc;
    tog_rc = ui_tree_base_toggle_node(tree, tree->active_node);
    if (tog_rc != UI_ERROR_NONE)
      return tog_rc;
  } break;

  default:
    break;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Recursively renders tree nodes to the DOM.
 * @param tree The tree instance.
 * @param node The current node identifier.
 * @param parent_container The parent DOM container to append to.
 * @param level The ARIA hierarchy level.
 * @param posinset The ARIA position in set.
 * @param setsize The ARIA set size.
 * @return UI_ERROR_NONE on success, or an error code.
 */
static ui_error_t render_recursive(struct ui_tree_base *tree, void *node,
                                   struct ui_dom_node *parent_container,
                                   size_t level, size_t posinset,
                                   size_t setsize) {
  struct ui_dom_node *item = NULL;
  struct ui_dom_node *group = NULL;
  char buf[32];
  ui_error_t rc;
  size_t child_count, i;
  int is_selected = 0;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &item);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Appending eagerly limits dangling resources if deeper generation fails */
  {
    ui_error_t _ign_rc = ui_dom_node_append_child(parent_container, item);
    (void)_ign_rc;
  }

#define UI_DOM_SET_ATTR_IGNORE(n, a, v) ui_dom_node_set_attribute((n), (a), (v))

  /* Accessibility */
  (void)UI_DOM_SET_ATTR_IGNORE(item, "role", "treeitem");
#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%lu", (unsigned long)level);
#else
  sprintf(buf, "%lu", (unsigned long)level);
#endif
  (void)UI_DOM_SET_ATTR_IGNORE(item, "aria-level", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%lu", (unsigned long)posinset);
#else
  sprintf(buf, "%lu", (unsigned long)posinset);
#endif
  (void)UI_DOM_SET_ATTR_IGNORE(item, "aria-posinset", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%lu", (unsigned long)setsize);
#else
  sprintf(buf, "%lu", (unsigned long)setsize);
#endif
  (void)UI_DOM_SET_ATTR_IGNORE(item, "aria-setsize", buf);

#define UI_SEL_IS_SEL_IGNORE(m, n, o)                                          \
  ui_selection_model_is_selected((m), (n), (o))
  (void)UI_SEL_IS_SEL_IGNORE(tree->selection_model, node, &is_selected);
  if (is_selected) {
    (void)UI_DOM_SET_ATTR_IGNORE(item, "aria-selected", "true");
  }

  rc = tree->model.render_node(node, item, tree->model.user_data);
  if (rc != UI_ERROR_NONE)
    return rc;

  child_count = tree->model.get_child_count(node, tree->model.user_data);
  if (child_count > 0) {
    int expanded = 0;
    (void)UI_TREE_IS_EXPAND_IGNORE(tree, node, &expanded);
    (void)UI_DOM_SET_ATTR_IGNORE(item, "aria-expanded",
                                 expanded ? "true" : "false");

    if (expanded) {
      rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &group);
      if (rc != UI_ERROR_NONE)
        return rc;

      {

        ui_error_t _ign_rc = ui_dom_node_append_child(item, group);

        (void)_ign_rc;
      }
      (void)UI_DOM_SET_ATTR_IGNORE(group, "role", "group");

      for (i = 0; i < child_count; i++) {
        void *child = tree->model.get_child(node, i, tree->model.user_data);
        rc =
            render_recursive(tree, child, group, level + 1, i + 1, child_count);
        if (rc != UI_ERROR_NONE)
          return rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_tree_base_render(struct ui_tree_base *tree,
                               struct ui_dom_node *container) {
  struct ui_dom_node *tree_root = NULL;
  ui_error_t rc;
  size_t root_count, i;

  if (!tree || !container)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &tree_root);
  if (rc != UI_ERROR_NONE)
    return rc;

  (void)UI_DOM_SET_ATTR_IGNORE(tree_root, "role", "tree");

  root_count = tree->model.get_root_count(tree->model.user_data);
  for (i = 0; i < root_count; i++) {
    void *root_node = tree->model.get_root_node(i, tree->model.user_data);
    rc = render_recursive(tree, root_node, tree_root, 1, i + 1, root_count);
    if (rc != UI_ERROR_NONE) {
      (void)ui_dom_node_destroy(tree_root);
      return rc;
    }
  }

  {

    ui_error_t _ign_rc = ui_dom_node_append_child(container, tree_root);

    (void)_ign_rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_tree_base_bind_data(struct ui_tree_base *widget,
                                  struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
