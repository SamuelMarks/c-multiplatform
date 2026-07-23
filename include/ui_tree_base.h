#ifndef UI_TREE_BASE_H
#define UI_TREE_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
#include "ui_selection_model.h"
#include <stddef.h>
/* clang-format on */

struct ui_tree_base;
struct ui_dom_node;

/**
 * @brief Data model interface for a tree.
 * The implementation MUST guarantee stable void* identifiers for each node.
 */
struct ui_tree_model {
  /** Retrieves the number of root nodes. */
  size_t (*get_root_count)(void *user_data);

  /** Retrieves a root node by index. */
  void *(*get_root_node)(size_t index, void *user_data);

  /** Retrieves the parent of a node. Returns NULL for root nodes. */
  void *(*get_parent)(void *node_id, void *user_data);

  /** Retrieves the number of children for a given node. */
  size_t (*get_child_count)(void *node_id, void *user_data);

  /** Retrieves a child node by index. */
  void *(*get_child)(void *node_id, size_t index, void *user_data);

  /** Renders the user-facing content of the node into cell_node. */
  enum ui_error (*render_node)(void *node_id, struct ui_dom_node *cell_node,
                               void *user_data);

  void *user_data;
};

/**
 * @brief Creates a new tree base component.
 *
 * @param out_tree Pointer to receive the allocated tree base.
 * @param model Data model providing dimensions, traversal, and rendering
 * callbacks.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_tree_base_create(struct ui_tree_base **out_tree,
                                  const struct ui_tree_model *model);

/**
 * @brief Destroys a tree component.
 *
 * @param tree The tree to destroy.
 */
enum ui_error ui_tree_base_destroy(struct ui_tree_base *tree);

/**
 * @brief Gets the selection model attached to this tree.
 *
 * @param tree The tree.
 * @return The selection model, or NULL.
 */
enum ui_error
ui_tree_base_get_selection_model(struct ui_tree_base *tree,
                                 struct ui_selection_model **out_model);

/**
 * @brief Checks if a specific node is expanded.
 *
 * @param tree The tree.
 * @param node_id The stable node identifier.
 * @return 1 if expanded, 0 if collapsed.
 */
enum ui_error ui_tree_base_is_expanded(const struct ui_tree_base *tree,
                                       void *node_id, int *out_is_expanded);

/**
 * @brief Sets the expanded state of a specific node.
 *
 * @param tree The tree.
 * @param node_id The stable node identifier.
 * @param expanded 1 to expand, 0 to collapse.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_tree_base_set_expanded(struct ui_tree_base *tree,
                                        void *node_id, int expanded);

/**
 * @brief Toggles the expanded state of a specific node.
 *
 * @param tree The tree.
 * @param node_id The stable node identifier.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_tree_base_toggle_node(struct ui_tree_base *tree,
                                       void *node_id);

/**
 * @brief Sets the active/focused node for keyboard navigation.
 *
 * @param tree The tree.
 * @param node_id The stable node identifier, or NULL to clear focus.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_tree_base_set_active_node(struct ui_tree_base *tree,
                                           void *node_id);

/**
 * @brief Gets the active/focused node.
 *
 * @param tree The tree.
 * @return The active node identifier, or NULL if none.
 */
enum ui_error ui_tree_base_get_active_node(const struct ui_tree_base *tree,
                                           void **out_node);

/**
 * @brief Handles keyboard events for ARIA standard tree traversal.
 *
 * Supports Up/Down for moving focus linearly across visible nodes,
 * and Left/Right for expanding/collapsing or moving to parents/children.
 *
 * @param tree The tree.
 * @param event The keyboard event.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_tree_base_handle_key_event(struct ui_tree_base *tree,
                              const struct ui_keyboard_event *event);

/**
 * @brief Generates the tree DOM structure including full ARIA mappings
 * (`role="tree"`, etc.).
 *
 * @param tree The tree.
 * @param container The container DOM node where the generated tree root will be
 * appended.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_tree_base_render(struct ui_tree_base *tree,
                                  struct ui_dom_node *container);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_tree_base_bind_data(struct ui_tree_base *widget,
                                     struct ui_computed *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TREE_BASE_H */
