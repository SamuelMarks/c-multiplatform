/* clang-format off */
/**
 * @file ui_tree_grid_base.h
 * @brief Tree grid base component for displaying tabular hierarchical data.
 */
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

/**
 * @struct ui_tree_grid_base
 * @brief Opaque handle for a tree grid base component.
 */
struct ui_tree_grid_base;

struct ui_dom_node;
struct ui_signal;

/**
 * @struct ui_tree_grid_model
 * @brief Data model interface for a tree grid.
 * The implementation MUST guarantee stable void* identifiers for each node.
 */
struct ui_tree_grid_model {
  /**
   * @brief Retrieves the number of root nodes.
   * @param user_data User data.
   * @return The number of root nodes.
   */
  size_t (*get_root_count)(void *user_data);

  /**
   * @brief Retrieves a root node by index.
   * @param index The index.
   * @param user_data User data.
   * @return A stable identifier for the node.
   */
  void *(*get_root_node)(size_t index, void *user_data);

  /**
   * @brief Retrieves the parent of a node. Returns NULL for root nodes.
   * @param node_id The node identifier.
   * @param user_data User data.
   * @return A stable identifier for the parent node, or NULL.
   */
  void *(*get_parent)(void *node_id, void *user_data);

  /**
   * @brief Retrieves the number of children for a given node.
   * @param node_id The node identifier.
   * @param user_data User data.
   * @return The number of children.
   */
  size_t (*get_child_count)(void *node_id, void *user_data);

  /**
   * @brief Retrieves a child node by index.
   * @param node_id The parent node identifier.
   * @param index The child index.
   * @param user_data User data.
   * @return A stable identifier for the child node.
   */
  void *(*get_child)(void *node_id, size_t index, void *user_data);

  /**
   * @brief Retrieves the number of columns in the grid.
   * @param user_data User data.
   * @return The number of columns.
   */
  size_t (*get_column_count)(void *user_data);

  /**
   * @brief Renders the user-facing content of the specific cell into cell_node.
   * @param node_id The node identifier.
   * @param col_index The column index.
   * @param cell_node The cell node to render into.
   * @param user_data User data.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*render_cell)(void *node_id, size_t col_index,
                            struct ui_dom_node *cell_node, void *user_data);

  /** @brief Opaque user data for the model callbacks. */
  void *user_data;
};

/**
 * @brief Creates a new tree grid base component.
 *
 * @param out_tree_grid Pointer to receive the allocated component.
 * @param model Data model providing dimensions, traversal, and rendering.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_tree_grid_base_create(struct ui_tree_grid_base **out_tree_grid,
                                    const struct ui_tree_grid_model *model);

/**
 * @brief Destroys a tree grid component.
 *
 * @param tree_grid The tree grid.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_tree_grid_base_destroy(struct ui_tree_grid_base *tree_grid);

/**
 * @brief Gets the underlying component instance for DOM mounting.
 *
 * @param tree_grid The tree grid.
 * @param out_component Pointer to receive the underlying component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_tree_grid_base_get_component(struct ui_tree_grid_base *tree_grid,
                                           struct ui_component **out_component);

/**
 * @brief Sets the expanded state of a specific node.
 *
 * @param tree_grid The tree grid.
 * @param node_id The stable node identifier.
 * @param expanded 1 to expand, 0 to collapse.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_tree_grid_base_set_expanded(struct ui_tree_grid_base *tree_grid,
                                          void *node_id, int expanded);

/**
 * @brief Toggles the expanded state of a specific node.
 *
 * @param tree_grid The tree grid.
 * @param node_id The stable node identifier.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_tree_grid_base_toggle_node(struct ui_tree_grid_base *tree_grid,
                                         void *node_id);

/**
 * @brief Checks if a specific node is expanded.
 *
 * @param tree_grid The tree grid.
 * @param node_id The stable node identifier.
 * @param out_is_expanded Pointer to store result (1 if expanded, 0 if
 * collapsed).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_tree_grid_base_is_expanded(const struct ui_tree_grid_base *tree_grid,
                              void *node_id, int *out_is_expanded);

/**
 * @brief Handles keyboard events for ARIA standard treegrid traversal.
 * Supports 2D navigation (Up/Down for rows, Left/Right for columns/expansion).
 *
 * @param tree_grid The tree grid.
 * @param event The keyboard event.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_tree_grid_base_handle_key_event(struct ui_tree_grid_base *tree_grid,
                                   const struct ui_keyboard_event *event);

/**
 * @brief Renders the tree grid into a container.
 *
 * @param tree_grid The tree grid.
 * @param container The container DOM node.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_tree_grid_base_render(struct ui_tree_grid_base *tree_grid,
                                    struct ui_dom_node *container);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TREE_GRID_BASE_H */
