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

struct ui_table_base;
struct ui_dom_node;

/**
 * @brief Sort direction for a table column.
 */
enum ui_table_sort_direction {
  UI_TABLE_SORT_NONE,
  UI_TABLE_SORT_ASCENDING,
  UI_TABLE_SORT_DESCENDING
};

/**
 * @brief Configuration for active table sorting.
 */
struct ui_table_sort_config {
  size_t active_column_index;
  enum ui_table_sort_direction direction;
};

/**
 * @brief Configuration for table pagination.
 */
struct ui_table_pagination_config {
  size_t page_size;
  size_t current_page;
};

/**
 * @brief Column sizing constraint strategy.
 */
enum ui_table_column_sizing { UI_TABLE_COLUMN_FLEX, UI_TABLE_COLUMN_FIXED };

/**
 * @brief Configuration for a single column.
 */
struct ui_table_column_config {
  enum ui_table_column_sizing sizing;
  float width; /* Fixed width or flex ratio */
  float min_width;
  float max_width;
};

/**
 * @brief Data model interface for a table.
 */
struct ui_table_model {
  size_t (*get_row_count)(void *user_data);
  size_t (*get_column_count)(void *user_data);
  enum ui_error (*render_cell)(size_t row, size_t col,
                               struct ui_dom_node *cell_node, void *user_data);
  enum ui_error (*render_header)(size_t col, struct ui_dom_node *header_node,
                                 void *user_data);
  void *user_data;
};

/**
 * @brief Creates a new table base component.
 *
 * @param out_table Pointer to receive the allocated table base.
 * @param model Data model providing dimensions and rendering callbacks.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_table_base_create(struct ui_table_base **out_table,
                                   const struct ui_table_model *model);

/**
 * @brief Destroys a table component.
 *
 * @param table The table to destroy.
 */
void ui_table_base_destroy(struct ui_table_base *table);

/**
 * @brief Gets the selection model attached to this table.
 *
 * @param table The table.
 * @return The selection model, or NULL.
 */
enum ui_error
ui_table_base_get_selection_model(struct ui_table_base *table,
                                  struct ui_selection_model **out_model);

/**
 * @brief Sets constraints and sizing rules for a specific column.
 *
 * @param table The table.
 * @param col_index The index of the column.
 * @param config The column sizing configuration.
 * @return UI_ERROR_NONE on success, or UI_ERROR_OUT_OF_BOUNDS.
 */
enum ui_error
ui_table_base_set_column_config(struct ui_table_base *table, size_t col_index,
                                const struct ui_table_column_config *config);

/**
 * @brief Sets the active sort tracking state.
 *
 * @param table The table.
 * @param config The sort configuration.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_table_base_set_sort_config(struct ui_table_base *table,
                              const struct ui_table_sort_config *config);

/**
 * @brief Sets pagination parameters.
 *
 * @param table The table.
 * @param config The pagination configuration.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_table_base_set_pagination_config(
    struct ui_table_base *table,
    const struct ui_table_pagination_config *config);

/**
 * @brief Generates the table DOM structure including full ARIA mappings,
 * sorting, and pagination logic.
 *
 * @param table The table.
 * @param container The container DOM node where the generated table root will
 * be appended.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_table_base_render(struct ui_table_base *table,
                                   struct ui_dom_node *container);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_table_base_bind_data(struct ui_table_base *widget,
                                      struct ui_computed *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TABLE_BASE_H */
