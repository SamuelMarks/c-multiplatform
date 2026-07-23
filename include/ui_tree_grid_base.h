/* clang-format off */
#ifndef UI_TREE_GRID_BASE_H
#define UI_TREE_GRID_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_error.h"
#include "ui_event.h"
#include "ui_selection_model.h"
#include "ui_component.h"
#include <stddef.h>

/* clang-format on */

struct ui_tree_grid_base;
struct ui_dom_node;
struct ui_signal;

/**
 * @brief Data model interface for a tree grid.
 * The implementation MUST guarantee stable void* identifiers for each node.
 */
struct ui_tree_grid_model {
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

  /** Retrieves the number of columns in the grid. */
  size_t (*get_column_count)(void *user_data);

  /** Renders the user-facing content of the specific cell into cell_node. */
  enum ui_error (*render_cell)(void *node_id, size_t col_index,
                               struct ui_dom_node *cell_node, void *user_data);

  void *user_data;
};

/**
 * @brief Creates a new tree grid base component.
 *
 * @param out_tree_grid Pointer to receive the allocated component.
 * @param model Data model providing dimensions, traversal, and rendering.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_tree_grid_base_create(struct ui_tree_grid_base **out_tree_grid,
                                       const struct ui_tree_grid_model *model);

/**
 * @brief Destroys a tree grid component.
 *
 * @param tree_grid The tree grid.
 */
void ui_tree_grid_base_destroy(struct ui_tree_grid_base *tree_grid);

/**
 * @brief Gets the underlying component instance for DOM mounting.
 *
 * @param tree_grid The tree grid.
 * @return The underlying component.
 */
enum ui_error
ui_tree_grid_base_get_component(struct ui_tree_grid_base *tree_grid,
                                struct ui_component **out_component);

/**
 * @brief Sets the expanded state of a specific node.
 *
 * @param tree_grid The tree grid.
 * @param node_id The stable node identifier.
 * @param expanded 1 to expand, 0 to collapse.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_tree_grid_base_set_expanded(struct ui_tree_grid_base *tree_grid,
                               void *node_id, int expanded);

/**
 * @brief Toggles the expanded state of a specific node.
 *
 * @param tree_grid The tree grid.
 * @param node_id The stable node identifier.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_tree_grid_base_toggle_node(struct ui_tree_grid_base *tree_grid,
                                            void *node_id);

/**
 * @brief Checks if a specific node is expanded.
 *
 * @param tree_grid The tree grid.
 * @param node_id The stable node identifier.
 * @param out_is_expanded Pointer to store result.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_tree_grid_base_is_expanded(const struct ui_tree_grid_base *tree_grid,
                              void *node_id, int *out_is_expanded);

/**
 * @brief Handles keyboard events for ARIA standard treegrid traversal.
 * Supports 2D navigation (Up/Down for rows, Left/Right for columns/expansion).
 *
 * @param tree_grid The tree grid.
 * @param event The keyboard event.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_tree_grid_base_handle_key_event(struct ui_tree_grid_base *tree_grid,
                                   const struct ui_keyboard_event *event);

/**
 * @brief Renders the tree grid into a container.
 *
 * @param tree_grid The tree grid.
 * @param container The container DOM node.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_tree_grid_base_render(struct ui_tree_grid_base *tree_grid,
                                       struct ui_dom_node *container);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TREE_GRID_BASE_H */
