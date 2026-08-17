/**
 * @file ui_grid_list_base.h
 * @brief Logic and layout state for grid lists.
 *
 * This header defines the base structure and logic for a grid list component,
 * handling grid columns, items, and row span logic.
 */

#ifndef UI_GRID_LIST_BASE_H
#define UI_GRID_LIST_BASE_H

struct ui_computed;
struct ui_signal;

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a grid list component base.
 */
struct ui_grid_list_base;

/**
 * @brief Represents a single item inside a grid list.
 */
struct ui_grid_list_item {
  int rowspan; /**< The number of rows this item spans. */
  int colspan; /**< The number of columns this item spans. */
  /* Other internal data can be added as needed */
};

/**
 * @brief Creates a new unstyled grid list base component.
 *
 * @param out_grid_list Pointer to receive the allocated grid list base
 * structure.
 * @param columns The number of columns in the grid.
 * @return `UI_ERROR_NONE` on success, `UI_ERROR_OUT_OF_MEMORY` on allocation
 * failure, `UI_ERROR_INVALID_ARGUMENT` if columns < 1.
 */
ui_error_t ui_grid_list_base_create(struct ui_grid_list_base **out_grid_list,
                                    int columns);

/**
 * @brief Destroys a grid list base component and frees all items.
 *
 * @param grid_list Pointer to the grid list to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_grid_list_base_destroy(struct ui_grid_list_base *grid_list);

/**
 * @brief Sets the number of columns in the grid.
 *
 * @param grid_list Pointer to the grid list.
 * @param columns The new number of columns.
 * @return `UI_ERROR_NONE` on success, `UI_ERROR_INVALID_ARGUMENT` on null
 * pointer or columns < 1.
 */
ui_error_t ui_grid_list_base_set_columns(struct ui_grid_list_base *grid_list,
                                         int columns);

/**
 * @brief Gets the number of columns in the grid.
 *
 * @param grid_list Pointer to the grid list.
 * @param out_columns Pointer to receive the number of columns.
 * @return `UI_ERROR_NONE` on success, `UI_ERROR_INVALID_ARGUMENT` on null
 * pointer.
 */
ui_error_t
ui_grid_list_base_get_columns(const struct ui_grid_list_base *grid_list,
                              int *out_columns);

/**
 * @brief Adds an item to the grid list with specified row and column span.
 *
 * @param grid_list Pointer to the grid list.
 * @param rowspan The row span of the item (must be >= 1).
 * @param colspan The column span of the item (must be >= 1).
 * @return `UI_ERROR_NONE` on success, `UI_ERROR_INVALID_ARGUMENT` on null
 * pointer or invalid spans, `UI_ERROR_OUT_OF_MEMORY` on allocation failure.
 */
ui_error_t ui_grid_list_base_add_item(struct ui_grid_list_base *grid_list,
                                      int rowspan, int colspan);

/**
 * @brief Gets the number of items in the grid list.
 *
 * @param grid_list Pointer to the grid list.
 * @param out_count Pointer to receive the number of items.
 * @return `UI_ERROR_NONE` on success, `UI_ERROR_INVALID_ARGUMENT` on null
 * pointer.
 */
ui_error_t
ui_grid_list_base_get_item_count(const struct ui_grid_list_base *grid_list,
                                 size_t *out_count);

/**
 * @brief Gets the item at the specified index.
 *
 * @param grid_list Pointer to the grid list.
 * @param index The index of the item.
 * @param out_item Pointer to receive the item pointer.
 * @return `UI_ERROR_NONE` on success, `UI_ERROR_INVALID_ARGUMENT` on null
 * pointer, `UI_ERROR_OUT_OF_BOUNDS` if index is invalid.
 */
ui_error_t
ui_grid_list_base_get_item(const struct ui_grid_list_base *grid_list,
                           size_t index,
                           const struct ui_grid_list_item **out_item);

/**
 * @brief Calculates the layout, determining the maximum number of rows based on
 * the items and their spans.
 *
 * This function computes the spanning logic based on current columns and items,
 * simulating placement of items into a grid.
 *
 * @param grid_list Pointer to the grid list.
 * @param out_rows Pointer to receive the calculated number of rows required.
 * @return `UI_ERROR_NONE` on success, `UI_ERROR_INVALID_ARGUMENT` on null
 * pointer, `UI_ERROR_OUT_OF_MEMORY` on temporary allocation failure.
 */
ui_error_t
ui_grid_list_base_calculate_rows(const struct ui_grid_list_base *grid_list,
                                 int *out_rows);

/**
 * @brief Binds a data signal to the grid list.
 *
 * @param widget Pointer to the grid list.
 * @param signal Pointer to the computed signal to bind to.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_grid_list_base_bind_data(struct ui_grid_list_base *widget,
                                       struct ui_computed *signal);

#ifdef __cplusplus
}
#endif

#endif /* UI_GRID_LIST_BASE_H */
