#ifndef UI_MILLER_COLUMNS_BASE_H
#define UI_MILLER_COLUMNS_BASE_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_signal.h"
#include "ui_component.h"
#include "ui_tree_base.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct ui_miller_columns_base
 * @brief Opaque handle for the Miller Columns component.
 */
struct ui_miller_columns_base;

/**
 * @struct ui_miller_column_state
 * @brief Represents a specific column in the cascading view.
 */
struct ui_miller_column_state {
  /** @brief The index of this column in the sequence. */
  int column_index;
  /** @brief The node ID from the tree model that this column is
             displaying children of. NULL if root column. */
  void *parent_node_id;
  /** @brief The currently selected node in this column,
             which triggers the next column. */
  void *selected_child_id;
};

/**
 * @brief Creates a Miller Columns base component.
 *
 * @param arena The memory arena to use for allocation.
 * @param tree_model The hierarchical data model to navigate.
 * @param model_user_data Opaque data passed to tree model callbacks.
 * @param out_miller Pointer to receive the created component handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_miller_columns_base_create(
    struct ui_arena *arena, const struct ui_tree_model *tree_model,
    void *model_user_data, struct ui_miller_columns_base **out_miller);

/**
 * @brief Destroys a Miller Columns base component.
 *
 * @param miller The component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_miller_columns_base_destroy(struct ui_miller_columns_base *miller);

/**
 * @brief Selects an item in a specific column. This will automatically collapse
 * any trailing columns and spawn a new column if the selected item has
 * children.
 *
 * @param miller The component.
 * @param column_index The index of the column containing the selection.
 * @param node_id The ID of the item being selected.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_miller_columns_base_select_item(struct ui_miller_columns_base *miller,
                                   int column_index, void *node_id);

/**
 * @brief Navigates left (focus parent column).
 *
 * @param miller The component.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if already at root.
 */
ui_error_t
ui_miller_columns_base_navigate_left(struct ui_miller_columns_base *miller);

/**
 * @brief Navigates right (focus child column if selection has children).
 *
 * @param miller The component.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if no child column
 * exists.
 */
ui_error_t
ui_miller_columns_base_navigate_right(struct ui_miller_columns_base *miller);

/**
 * @brief Retrieves the number of currently active/visible columns.
 *
 * @param miller The component.
 * @param out_count Pointer to receive the count.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_miller_columns_base_get_column_count(
    const struct ui_miller_columns_base *miller, int *out_count);

/**
 * @brief Retrieves the signal emitted when the column topology changes (e.g.
 * column added/removed). Payload is an int representing the total number of
 * active columns.
 *
 * @param miller The component.
 * @param out_signal Pointer to receive the signal handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_miller_columns_base_get_topology_changed_signal(
    struct ui_miller_columns_base *miller, ui_signal_t **out_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_MILLER_COLUMNS_BASE_H */
