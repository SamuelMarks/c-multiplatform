/**
 * @file md3_data_hierarchy.h
 * @brief Material 3 & Expressive Data Tables and Hierarchical Presentation.
 *
 * Provides spec-compliant Material 3 implementations for:
 * - md3_table (wrapping ui_table_base & ui_datagrid_base)
 * - md3_paginator (wrapping ui_pagination_base)
 * - md3_sort_header (wrapping ui_sort_header_base)
 * - md3_tree & md3_tree_node (wrapping ui_tree_base)
 * - md3_tree_grid (wrapping ui_tree_grid_base & ui_datagrid_base)
 * - md3_transfer_list (wrapping ui_transfer_list_base)
 */

#ifndef MATERIAL3_MD3_DATA_HIERARCHY_H
#define MATERIAL3_MD3_DATA_HIERARCHY_H

/* clang-format off */
#include "ui_component.h"
#include "ui_datagrid_base.h"
#include "ui_error.h"
#include "ui_pagination_base.h"
#include "ui_sort_header_base.h"
#include "ui_table_base.h"
#include "ui_transfer_list_base.h"
#include "ui_tree_base.h"
#include "ui_tree_grid_base.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/* ========================================================================= */
/* md3_table                                                                 */
/* ========================================================================= */

/**
 * @enum md3_table_density
 * @brief Density metrics for Material 3 Table rows.
 */
enum md3_table_density {
  MD3_TABLE_DENSITY_DENSE = 0,  /**< 40dp row height */
  MD3_TABLE_DENSITY_NORMAL = 1, /**< 52dp row height */
  MD3_TABLE_DENSITY_RELAXED = 2 /**< 64dp row height */
};

/**
 * @struct md3_table
 * @brief Material 3 Data Table component wrapping ui_table_base and
 * ui_datagrid_base.
 */
struct md3_table {
  struct ui_table_base *table_base;
  struct ui_datagrid_base *datagrid_base;
  enum md3_table_density density;
  int sticky_header;
  int sticky_first_column;
  int sticky_last_column;
  int is_selectable;
  int glassmorphism_enabled;
  int virtual_scroll_enabled;
  size_t virtual_viewport_height;
  int horizontal_scroll_enabled;
  size_t hovered_row;
  int has_hovered_row;
  size_t selected_rows[64];
  size_t selected_count;
  size_t expanded_rows[64];
  size_t expanded_count;
  size_t total_rows;
};

/**
 * @brief Creates a Material 3 Data Table.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_table Pointer to receive allocated table.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_table_create(struct ui_engine *engine, struct md3_table **out_table);

/**
 * @brief Destroys a Material 3 Data Table.
 *
 * @param table Table instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_table_destroy(struct md3_table *table);

/**
 * @brief Configures row density metrics for the table.
 *
 * @param table Table instance.
 * @param density Target density (Dense, Normal, Relaxed).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_table_set_density(struct md3_table *table, enum md3_table_density density);

/**
 * @brief Retrieves row height in DP corresponding to current density.
 *
 * @param table Table instance.
 * @param out_height_dp Pointer to receive row height in DP.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_table_get_row_height(const struct md3_table *table, float *out_height_dp);

/**
 * @brief Sets the data model for the table.
 *
 * @param table Table instance.
 * @param model Data model supplying row/col counts and render callbacks.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_table_set_model(
    struct md3_table *table, const struct ui_table_model *model);

/**
 * @brief Enables or disables sticky column header behavior.
 *
 * @param table Table instance.
 * @param sticky 1 to enable sticky header, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_table_set_sticky_header(struct md3_table *table, int sticky);

/**
 * @brief Enables or disables sticky first column behavior.
 *
 * @param table Table instance.
 * @param sticky 1 to enable sticky first column, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_table_set_sticky_first_column(struct md3_table *table, int sticky);

/**
 * @brief Enables or disables sticky last column behavior.
 *
 * @param table Table instance.
 * @param sticky 1 to enable sticky last column, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_table_set_sticky_last_column(struct md3_table *table, int sticky);

/**
 * @brief Sets row selection mode on or off.
 *
 * @param table Table instance.
 * @param selectable 1 to enable selection checkboxes, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_table_set_selectable(struct md3_table *table, int selectable);

/**
 * @brief Selects or deselects a specific row index.
 *
 * @param table Table instance.
 * @param row_index Zero-based row index.
 * @param selected 1 to select, 0 to deselect.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_table_select_row(struct md3_table *table, size_t row_index, int selected);

/**
 * @brief Queries whether a specific row index is selected.
 *
 * @param table Table instance.
 * @param row_index Zero-based row index.
 * @param out_selected Pointer to receive 1 if selected, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_table_is_row_selected(
    const struct md3_table *table, size_t row_index, int *out_selected);

/**
 * @brief Selects or deselects all rows in the table.
 *
 * @param table Table instance.
 * @param selected 1 to select all, 0 to deselect all.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_table_select_all(struct md3_table *table, int selected);

/**
 * @brief Retrieves aggregate selection state (all selected vs indeterminate).
 *
 * @param table Table instance.
 * @param out_all_selected Pointer to receive 1 if all rows selected, 0
 * otherwise.
 * @param out_indeterminate Pointer to receive 1 if partially selected.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_table_get_selection_state(const struct md3_table *table,
                              int *out_all_selected, int *out_indeterminate);

/**
 * @brief Updates the actively hovered row for state layer highlight.
 *
 * @param table Table instance.
 * @param row_index Zero-based row index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_table_set_hovered_row(struct md3_table *table, size_t row_index);

/**
 * @brief Configures virtual scroll mode for large row sets.
 *
 * @param table Table instance.
 * @param enabled 1 to enable virtual scrolling, 0 to disable.
 * @param viewport_height Height in pixels of visible viewport.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_table_set_virtual_scroll(
    struct md3_table *table, int enabled, size_t viewport_height);

/**
 * @brief Enables or disables horizontal scrolling for overflow columns.
 *
 * @param table Table instance.
 * @param enabled 1 to enable horizontal scroll container, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_table_set_horizontal_scroll(struct md3_table *table, int enabled);

/**
 * @brief Enables or disables Expressive glassmorphism backdrop blur on sticky
 * header.
 *
 * @param table Table instance.
 * @param enabled 1 to enable glassmorphism blur, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_table_set_glassmorphism(struct md3_table *table, int enabled);

/**
 * @brief Toggles expandable detail row drawer for a given row index.
 *
 * @param table Table instance.
 * @param row_index Zero-based row index.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_table_toggle_row_expansion(struct md3_table *table, size_t row_index);

/**
 * @brief Queries whether a specific row index has its detail drawer expanded.
 *
 * @param table Table instance.
 * @param row_index Zero-based row index.
 * @param out_expanded Pointer to receive 1 if expanded, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_table_is_row_expanded(
    const struct md3_table *table, size_t row_index, int *out_expanded);

/**
 * @brief Interactively resizes a column width with spring settle snap.
 *
 * @param table Table instance.
 * @param col_index Column index to resize.
 * @param new_width Target width in pixels.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_table_resize_column(
    struct md3_table *table, size_t col_index, float new_width);

/* ========================================================================= */
/* md3_paginator                                                             */
/* ========================================================================= */

/**
 * @struct md3_paginator
 * @brief Material 3 Paginator component wrapping ui_pagination_base.
 */
struct md3_paginator {
  struct ui_pagination_base *base;
  size_t total_items;
  size_t page_size;
  size_t current_page;
  size_t page_size_options[8];
  size_t page_size_option_count;
  int is_compact;
  int is_expressive;
};

/**
 * @brief Creates a Material 3 Paginator.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_paginator Pointer to receive allocated paginator.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_paginator_create(
    struct ui_engine *engine, struct md3_paginator **out_paginator);

/**
 * @brief Destroys a Material 3 Paginator.
 *
 * @param paginator Paginator instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_paginator_destroy(struct md3_paginator *paginator);

/**
 * @brief Sets total item count and page size for the paginator.
 *
 * @param paginator Paginator instance.
 * @param total_items Total count of items.
 * @param page_size Items per page.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_paginator_set_config(
    struct md3_paginator *paginator, size_t total_items, size_t page_size);

/**
 * @brief Sets the active page index (0-based).
 *
 * @param paginator Paginator instance.
 * @param page_index Target page index.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_paginator_set_current_page(
    struct md3_paginator *paginator, size_t page_index);

/**
 * @brief Retrieves the active page index (0-based).
 *
 * @param paginator Paginator instance.
 * @param out_page Pointer to receive page index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_paginator_get_current_page(
    const struct md3_paginator *paginator, size_t *out_page);

/**
 * @brief Retrieves the calculated total page count.
 *
 * @param paginator Paginator instance.
 * @param out_total_pages Pointer to receive total pages.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_paginator_get_total_pages(
    const struct md3_paginator *paginator, size_t *out_total_pages);

/**
 * @brief Navigates to the next page.
 *
 * @param paginator Paginator instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_paginator_next(struct md3_paginator *paginator);

/**
 * @brief Navigates to the previous page.
 *
 * @param paginator Paginator instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_paginator_prev(struct md3_paginator *paginator);

/**
 * @brief Navigates to the first page (index 0).
 *
 * @param paginator Paginator instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_paginator_first(struct md3_paginator *paginator);

/**
 * @brief Navigates to the last page.
 *
 * @param paginator Paginator instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_paginator_last(struct md3_paginator *paginator);

/**
 * @brief Configures selectable page size options (e.g. 5, 10, 25, 100).
 *
 * @param paginator Paginator instance.
 * @param options Array of page size integers.
 * @param count Number of options in the array (max 8).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_paginator_set_page_size_options(
    struct md3_paginator *paginator, const size_t *options, size_t count);

/**
 * @brief Retrieves the active page size.
 *
 * @param paginator Paginator instance.
 * @param out_page_size Pointer to receive active page size.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_paginator_get_page_size(
    const struct md3_paginator *paginator, size_t *out_page_size);

/**
 * @brief Sets the active page size.
 *
 * @param paginator Paginator instance.
 * @param page_size Target page size.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_paginator_set_page_size(struct md3_paginator *paginator, size_t page_size);

/**
 * @brief Formats localized item range label display (e.g. "1 – 10 of 100").
 *
 * @param paginator Paginator instance.
 * @param buffer Output char buffer to receive formatted text.
 * @param buffer_size Size of output buffer.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_paginator_get_range_label(
    const struct md3_paginator *paginator, char *buffer, size_t buffer_size);

/**
 * @brief Enables or disables compact mobile presentation mode.
 *
 * @param paginator Paginator instance.
 * @param compact 1 for compact mode, 0 for standard mode.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_paginator_set_compact(struct md3_paginator *paginator, int compact);

/**
 * @brief Enables or disables Expressive sliding pill indicator and spring
 * transitions.
 *
 * @param paginator Paginator instance.
 * @param enabled 1 to enable Expressive animations, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_paginator_set_expressive(struct md3_paginator *paginator, int enabled);

/* ========================================================================= */
/* md3_sort_header                                                           */
/* ========================================================================= */

/**
 * @struct md3_sort_header
 * @brief Material 3 Sort Header component wrapping ui_sort_header_base.
 */
struct md3_sort_header {
  struct ui_sort_header_base *base;
  char column_id[64];
  enum ui_sort_direction direction;
  int allow_none;
  int priority;
  int is_expressive;
};

/**
 * @brief Creates a Material 3 Sort Header for a column.
 *
 * @param engine Pointer to ui_engine instance.
 * @param column_id Identifier string for the column.
 * @param out_header Pointer to receive allocated sort header.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_sort_header_create(struct ui_engine *engine, const char *column_id,
                       struct md3_sort_header **out_header);

/**
 * @brief Destroys a Material 3 Sort Header.
 *
 * @param header Sort header instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_sort_header_destroy(struct md3_sort_header *header);

/**
 * @brief Cycles through sort directions: None -> Ascending -> Descending ->
 * None.
 *
 * @param header Sort header instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_sort_header_cycle(struct md3_sort_header *header);

/**
 * @brief Sets an explicit sort direction.
 *
 * @param header Sort header instance.
 * @param direction Target sort direction (None, Ascending, Descending).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_sort_header_set_direction(
    struct md3_sort_header *header, enum ui_sort_direction direction);

/**
 * @brief Gets current sort direction.
 *
 * @param header Sort header instance.
 * @param out_direction Pointer to receive sort direction.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_sort_header_get_direction(const struct md3_sort_header *header,
                              enum ui_sort_direction *out_direction);

/**
 * @brief Configures whether sort cycling includes the NONE (unsorted) state.
 *
 * @param header Sort header instance.
 * @param allow_none 1 for tri-state (None->Asc->Desc->None), 0 for bi-state
 * (Asc<->Desc).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_sort_header_set_cycle_mode(struct md3_sort_header *header, int allow_none);

/**
 * @brief Computes arrow rotation angle in degrees (0 for ascending, 180 for
 * descending, -1 for none).
 *
 * @param header Sort header instance.
 * @param out_degrees Pointer to receive angle in degrees.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_sort_header_get_arrow_rotation(
    const struct md3_sort_header *header, float *out_degrees);

/**
 * @brief Sets multi-column sort priority badge (e.g. 1, 2). 0 clears priority.
 *
 * @param header Sort header instance.
 * @param priority Ranking index (1-based, or 0 for none).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_sort_header_set_priority(struct md3_sort_header *header, int priority);

/**
 * @brief Gets multi-column sort priority badge.
 *
 * @param header Sort header instance.
 * @param out_priority Pointer to receive priority ranking.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_sort_header_get_priority(
    const struct md3_sort_header *header, int *out_priority);

/**
 * @brief Enables or disables Expressive spring flip animation on direction
 * inversion.
 *
 * @param header Sort header instance.
 * @param enabled 1 to enable Expressive physics, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_sort_header_set_expressive(struct md3_sort_header *header, int enabled);

/* ========================================================================= */
/* md3_tree                                                                  */
/* ========================================================================= */

/**
 * @struct md3_tree
 * @brief Material 3 Hierarchical Tree component wrapping ui_tree_base.
 */
struct md3_tree {
  struct ui_tree_base *base;
  const struct ui_tree_model *model;
  float indent_px;
  int guide_lines_enabled;
  int multi_select;
  int virtual_scroll_enabled;
  int is_expressive;
  void *expanded_nodes[128];
  size_t expanded_count;
  void *selected_nodes[128];
  size_t selected_count;
  void *checked_nodes[128];
  int checked_states[128];
  size_t checked_count;
};

/**
 * @brief Creates a Material 3 Hierarchical Tree.
 *
 * @param engine Pointer to ui_engine instance.
 * @param model Data model providing tree structure and rendering callbacks.
 * @param out_tree Pointer to receive allocated tree.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_create(struct ui_engine *engine, const struct ui_tree_model *model,
                struct md3_tree **out_tree);

/**
 * @brief Destroys a Material 3 Tree.
 *
 * @param tree Tree instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_destroy(struct md3_tree *tree);

/**
 * @brief Sets expansion state for a node in the tree.
 *
 * @param tree Tree instance.
 * @param node_id Opaque node identifier.
 * @param expanded 1 to expand, 0 to collapse.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_set_node_expanded(struct md3_tree *tree, void *node_id, int expanded);

/**
 * @brief Queries expansion state for a node in the tree.
 *
 * @param tree Tree instance.
 * @param node_id Opaque node identifier.
 * @param out_expanded Pointer to receive 1 if expanded, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_tree_is_node_expanded(
    const struct md3_tree *tree, void *node_id, int *out_expanded);

/**
 * @brief Toggles expansion state for a node.
 *
 * @param tree Tree instance.
 * @param node_id Opaque node identifier.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_toggle_node_expanded(struct md3_tree *tree, void *node_id);

/**
 * @brief Configures child indentation step in pixels.
 *
 * @param tree Tree instance.
 * @param indent_px Indentation width in pixels.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_set_indent(struct md3_tree *tree, float indent_px);

/**
 * @brief Gets configured child indentation step.
 *
 * @param tree Tree instance.
 * @param out_indent_px Pointer to receive indentation width in pixels.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_get_indent(const struct md3_tree *tree, float *out_indent_px);

/**
 * @brief Enables or disables vertical indentation guide lines.
 *
 * @param tree Tree instance.
 * @param enabled 1 to enable guide lines, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_set_guide_lines(struct md3_tree *tree, int enabled);

/**
 * @brief Selects or deselects a node.
 *
 * @param tree Tree instance.
 * @param node_id Opaque node identifier.
 * @param selected 1 to select, 0 to deselect.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_select_node(struct md3_tree *tree, void *node_id, int selected);

/**
 * @brief Queries selection state for a node.
 *
 * @param tree Tree instance.
 * @param node_id Opaque node identifier.
 * @param out_selected Pointer to receive 1 if selected, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_tree_is_node_selected(
    const struct md3_tree *tree, void *node_id, int *out_selected);

/**
 * @brief Configures single vs multi-selection mode.
 *
 * @param tree Tree instance.
 * @param multi_select 1 for multi-select, 0 for single-select.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_set_multi_select(struct md3_tree *tree, int multi_select);

/**
 * @brief Sets tri-state checkbox state for a tree node (0=unchecked, 1=checked,
 * 2=indeterminate).
 *
 * @param tree Tree instance.
 * @param node_id Opaque node identifier.
 * @param state Checked state.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_tree_set_node_checked_state(
    struct md3_tree *tree, void *node_id, int state);

/**
 * @brief Queries tri-state checkbox state for a tree node.
 *
 * @param tree Tree instance.
 * @param node_id Opaque node identifier.
 * @param out_state Pointer to receive state (0=unchecked, 1=checked,
 * 2=indeterminate).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_tree_get_node_checked_state(
    const struct md3_tree *tree, void *node_id, int *out_state);

/**
 * @brief Enables or disables virtual scrolling for large tree hierarchies.
 *
 * @param tree Tree instance.
 * @param enabled 1 to enable virtual scrolling, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_set_virtual_scroll(struct md3_tree *tree, int enabled);

/**
 * @brief Enables or disables Expressive pill active highlights and accordion
 * animations.
 *
 * @param tree Tree instance.
 * @param enabled 1 to enable Expressive features, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_set_expressive(struct md3_tree *tree, int enabled);

/* ========================================================================= */
/* md3_tree_grid                                                             */
/* ========================================================================= */

/**
 * @struct md3_tree_grid
 * @brief Material 3 Hierarchical Data Grid wrapping ui_tree_grid_base and
 * ui_datagrid_base.
 */
struct md3_tree_grid {
  struct ui_tree_grid_base *base;
  struct ui_datagrid_base *datagrid;
  const struct ui_tree_model *model;
  size_t column_count;
  float indent_px;
  int sticky_headers;
  int zebra_shading;
  int is_expressive;
  void *expanded_nodes[128];
  size_t expanded_count;
  void *selected_rows[128];
  size_t selected_count;
};

/**
 * @brief Creates a Material 3 Tree Grid.
 *
 * @param engine Pointer to ui_engine instance.
 * @param model Data model supplying tree hierarchy and rendering.
 * @param out_tree_grid Pointer to receive allocated tree grid.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_tree_grid_create(
    struct ui_engine *engine, const struct ui_tree_model *model,
    struct md3_tree_grid **out_tree_grid);

/**
 * @brief Destroys a Material 3 Tree Grid.
 *
 * @param tg Tree grid instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_grid_destroy(struct md3_tree_grid *tg);

/**
 * @brief Sets total column count for the tree grid.
 *
 * @param tg Tree grid instance.
 * @param count Number of columns.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_grid_set_column_count(struct md3_tree_grid *tg, size_t count);

/**
 * @brief Sets node expansion state in the tree grid.
 *
 * @param tg Tree grid instance.
 * @param node_id Opaque node identifier.
 * @param expanded 1 to expand, 0 to collapse.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_tree_grid_set_node_expanded(
    struct md3_tree_grid *tg, void *node_id, int expanded);

/**
 * @brief Queries node expansion state in the tree grid.
 *
 * @param tg Tree grid instance.
 * @param node_id Opaque node identifier.
 * @param out_expanded Pointer to receive 1 if expanded, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_tree_grid_is_node_expanded(
    const struct md3_tree_grid *tg, void *node_id, int *out_expanded);

/**
 * @brief Selects or deselects a row in the tree grid.
 *
 * @param tg Tree grid instance.
 * @param node_id Opaque node identifier.
 * @param selected 1 to select, 0 to deselect.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_grid_select_row(struct md3_tree_grid *tg, void *node_id, int selected);

/**
 * @brief Queries row selection state in the tree grid.
 *
 * @param tg Tree grid instance.
 * @param node_id Opaque node identifier.
 * @param out_selected Pointer to receive 1 if selected, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_tree_grid_is_row_selected(
    const struct md3_tree_grid *tg, void *node_id, int *out_selected);

/**
 * @brief Sets hierarchical indentation width in pixels.
 *
 * @param tg Tree grid instance.
 * @param indent_px Indentation step in pixels.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_grid_set_indent(struct md3_tree_grid *tg, float indent_px);

/**
 * @brief Enables or disables sticky column headers.
 *
 * @param tg Tree grid instance.
 * @param sticky 1 to enable sticky headers, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_grid_set_sticky_headers(struct md3_tree_grid *tg, int sticky);

/**
 * @brief Enables or disables alternating zebra row shading.
 *
 * @param tg Tree grid instance.
 * @param enabled 1 to enable zebra shading, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_grid_set_zebra_shading(struct md3_tree_grid *tg, int enabled);

/**
 * @brief Enables or disables Expressive spring branch expansion.
 *
 * @param tg Tree grid instance.
 * @param enabled 1 to enable Expressive animations, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tree_grid_set_expressive(struct md3_tree_grid *tg, int enabled);

/* ========================================================================= */
/* md3_transfer_list                                                         */
/* ========================================================================= */

/**
 * @struct md3_transfer_item
 * @brief Item entry held within an md3_transfer_list.
 */
struct md3_transfer_item {
  int id;
  char label[64];
  void *data;
  int selected;
  struct md3_transfer_item *next;
};

/**
 * @struct md3_transfer_list
 * @brief Material 3 Dual-List Transfer component wrapping
 * ui_transfer_list_base.
 */
struct md3_transfer_list {
  struct ui_transfer_list_base base;
  struct ui_component comp;
  struct md3_transfer_item *left_head;
  struct md3_transfer_item *right_head;
  char left_filter[64];
  char right_filter[64];
  int is_expressive;
};

/**
 * @brief Creates a Material 3 Transfer List.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_transfer_list Pointer to receive allocated transfer list.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_transfer_list_create(
    struct ui_engine *engine, struct md3_transfer_list **out_transfer_list);

/**
 * @brief Destroys a Material 3 Transfer List.
 *
 * @param tl Transfer list instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_transfer_list_destroy(struct md3_transfer_list *tl);

/**
 * @brief Adds an item to either the left or right list.
 *
 * @param tl Transfer list instance.
 * @param to_right 1 to add to the right ("Selected") list, 0 to add to the left
 * ("Available") list.
 * @param id Unique item identifier.
 * @param label Text label for item.
 * @param data Opaque user data payload.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_transfer_list_add_item(struct md3_transfer_list *tl, int to_right, int id,
                           const char *label, void *data);

/**
 * @brief Selects or deselects an item in either list.
 *
 * @param tl Transfer list instance.
 * @param is_right 1 if target is in the right list, 0 if in the left list.
 * @param id Unique item identifier.
 * @param selected 1 to select, 0 to deselect.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_transfer_list_set_item_selected(
    struct md3_transfer_list *tl, int is_right, int id, int selected);

/**
 * @brief Transfers all selected items from the left list to the right list.
 *
 * @param tl Transfer list instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_transfer_list_move_selected_right(struct md3_transfer_list *tl);

/**
 * @brief Transfers all items from the left list to the right list.
 *
 * @param tl Transfer list instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_transfer_list_move_all_right(struct md3_transfer_list *tl);

/**
 * @brief Transfers all selected items from the right list to the left list.
 *
 * @param tl Transfer list instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_transfer_list_move_selected_left(struct md3_transfer_list *tl);

/**
 * @brief Transfers all items from the right list to the left list.
 *
 * @param tl Transfer list instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_transfer_list_move_all_left(struct md3_transfer_list *tl);

/**
 * @brief Sets filter query string for the left list.
 *
 * @param tl Transfer list instance.
 * @param query Filter string, or empty/NULL to clear.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_transfer_list_set_left_filter(
    struct md3_transfer_list *tl, const char *query);

/**
 * @brief Sets filter query string for the right list.
 *
 * @param tl Transfer list instance.
 * @param query Filter string, or empty/NULL to clear.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_transfer_list_set_right_filter(
    struct md3_transfer_list *tl, const char *query);

/**
 * @brief Retrieves counts of selected and total items in the left list.
 *
 * @param tl Transfer list instance.
 * @param out_selected Pointer to receive selected count.
 * @param out_total Pointer to receive total count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_transfer_list_get_left_counts(const struct md3_transfer_list *tl,
                                  size_t *out_selected, size_t *out_total);

/**
 * @brief Retrieves counts of selected and total items in the right list.
 *
 * @param tl Transfer list instance.
 * @param out_selected Pointer to receive selected count.
 * @param out_total Pointer to receive total count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_transfer_list_get_right_counts(const struct md3_transfer_list *tl,
                                   size_t *out_selected, size_t *out_total);

/**
 * @brief Enables or disables Expressive staggered spring animations and pill
 * highlights.
 *
 * @param tl Transfer list instance.
 * @param enabled 1 to enable Expressive features, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_transfer_list_set_expressive(struct md3_transfer_list *tl, int enabled);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_DATA_HIERARCHY_H */
