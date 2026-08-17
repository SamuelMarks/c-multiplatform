/**
 * @file ui_table_base.h
 * @brief Defines the table base component and related models.
 */
#ifndef UI_TABLE_BASE_H
#define UI_TABLE_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_selection_model.h"
#include <stddef.h>
/* clang-format on */

/**
 * @struct ui_table_base
 * @brief Opaque handle for a table base component.
 */
struct ui_table_base;

struct ui_dom_node;

/**
 * @enum ui_table_sort_direction
 * @brief Sort direction for a table column.
 */
enum ui_table_sort_direction {
  /** @brief No active sorting. */
  UI_TABLE_SORT_NONE,
  /** @brief Ascending sort order. */
  UI_TABLE_SORT_ASCENDING,
  /** @brief Descending sort order. */
  UI_TABLE_SORT_DESCENDING
};

/**
 * @struct ui_table_sort_config
 * @brief Configuration for active table sorting.
 */
struct ui_table_sort_config {
  /** @brief The index of the currently sorted column. */
  size_t active_column_index;
  /** @brief The direction of the sort. */
  enum ui_table_sort_direction direction;
};

/**
 * @struct ui_table_pagination_config
 * @brief Configuration for table pagination.
 */
struct ui_table_pagination_config {
  /** @brief Number of items per page. */
  size_t page_size;
  /** @brief The current page index (0-based). */
  size_t current_page;
};

/**
 * @enum ui_table_column_sizing
 * @brief Column sizing constraint strategy.
 */
enum ui_table_column_sizing {
  /** @brief Column size is flexible based on ratio. */
  UI_TABLE_COLUMN_FLEX,
  /** @brief Column size is fixed. */
  UI_TABLE_COLUMN_FIXED
};

/**
 * @struct ui_table_column_config
 * @brief Configuration for a single column.
 */
struct ui_table_column_config {
  /** @brief The sizing strategy for the column. */
  enum ui_table_column_sizing sizing;
  /** @brief Fixed width or flex ratio, depending on the sizing strategy. */
  float width;
  /** @brief The minimum width for the column. */
  float min_width;
  /** @brief The maximum width for the column. */
  float max_width;
};

/**
 * @struct ui_table_model
 * @brief Data model interface for a table.
 */
struct ui_table_model {
  /**
   * @brief Callback to get the total number of rows.
   * @param user_data User data.
   * @return The number of rows.
   */
  size_t (*get_row_count)(void *user_data);

  /**
   * @brief Callback to get the total number of columns.
   * @param user_data User data.
   * @return The number of columns.
   */
  size_t (*get_column_count)(void *user_data);

  /**
   * @brief Callback to render a specific cell.
   * @param row The row index.
   * @param col The column index.
   * @param cell_node The DOM node to render into.
   * @param user_data User data.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*render_cell)(size_t row, size_t col,
                            struct ui_dom_node *cell_node, void *user_data);

  /**
   * @brief Callback to render a specific header cell.
   * @param col The column index.
   * @param header_node The DOM node to render into.
   * @param user_data User data.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*render_header)(size_t col, struct ui_dom_node *header_node,
                              void *user_data);

  /** @brief Opaque user data passed to callbacks. */
  void *user_data;
};

/**
 * @brief Creates a new table base component.
 *
 * @param out_table Pointer to receive the allocated table base.
 * @param model Data model providing dimensions and rendering callbacks.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_table_base_create(struct ui_table_base **out_table,
                                const struct ui_table_model *model);

/**
 * @brief Destroys a table component.
 *
 * @param table The table to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_table_base_destroy(struct ui_table_base *table);

/**
 * @brief Gets the selection model attached to this table.
 *
 * @param table The table.
 * @param out_model Pointer to receive the selection model.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_table_base_get_selection_model(struct ui_table_base *table,
                                  struct ui_selection_model **out_model);

/**
 * @brief Sets constraints and sizing rules for a specific column.
 *
 * @param table The table.
 * @param col_index The index of the column.
 * @param config The column sizing configuration.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_table_base_set_column_config(struct ui_table_base *table, size_t col_index,
                                const struct ui_table_column_config *config);

/**
 * @brief Sets the active sort tracking state.
 *
 * @param table The table.
 * @param config The sort configuration.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_table_base_set_sort_config(struct ui_table_base *table,
                              const struct ui_table_sort_config *config);

/**
 * @brief Sets pagination parameters.
 *
 * @param table The table.
 * @param config The pagination configuration.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_table_base_set_pagination_config(
    struct ui_table_base *table,
    const struct ui_table_pagination_config *config);

/**
 * @brief Generates the table DOM structure including full ARIA mappings,
 * sorting, and pagination logic.
 *
 * @param table The table.
 * @param container The container DOM node where the generated table root will
 * be appended.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_table_base_render(struct ui_table_base *table,
                                struct ui_dom_node *container);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_table_base_bind_data(struct ui_table_base *widget,
                                   struct ui_computed *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TABLE_BASE_H */
