#ifndef M3_DATA_TABLE_H
#define M3_DATA_TABLE_H

/**
 * @file m3_data_table.h
 * @brief Material 3 Data Tables.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_text.h"

/**
 * @brief Sorting direction for a table column.
 */
typedef enum M3SortDirection {
  M3_SORT_ASCENDING = 0,
  M3_SORT_DESCENDING = 1,
  M3_SORT_NONE = 2
} M3SortDirection;

/**
 * @brief Column definition for a data table.
 */
typedef struct M3DataTableColumn {
  const char* label;              /**< Column header text. */
  CMPBool numeric;                /**< CMP_TRUE if data is numeric (right-aligned). */
  CMPBool sortable;               /**< CMP_TRUE if column is sortable. */
  M3SortDirection sort_direction; /**< Current sort direction if sorted. */
  const char* tooltip;            /**< Optional tooltip text. */
} M3DataTableColumn;

/**
 * @brief State for a data table.
 */
typedef struct M3DataTable {
  CMPRect bounds;                 /**< Bounding rectangle for the table. */
  M3DataTableColumn* columns;     /**< Array of columns. */
  cmp_u32 column_count;           /**< Number of columns. */
  cmp_u32 row_count;              /**< Total number of rows. */
  cmp_u32 rows_per_page;          /**< Rows per page for pagination. */
  cmp_u32 current_page;           /**< Current page (0-based). */
  CMPBool selectable;             /**< CMP_TRUE if rows can be selected. */
  CMPBool* selected_rows;         /**< Array of booleans indicating selection state (size: row_count). User managed. */
} M3DataTable;

/**
 * @brief Initialize a new data table.
 * @param table The table to initialize.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL m3_data_table_init(M3DataTable* table);

/**
 * @brief Render the data table headers.
 * @param ctx The visual context.
 * @param table The table definition.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_data_table_draw_headers(CMPPaintContext* ctx, const M3DataTable* table);

/**
 * @brief Render a row of the data table.
 * @param ctx The visual context.
 * @param table The table definition.
 * @param row_index The index of the row.
 * @param cells Array of text strings for the cells in this row.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_data_table_draw_row(CMPPaintContext* ctx, const M3DataTable* table, cmp_u32 row_index, const char** cells);

/**
 * @brief Free resources associated with a data table.
 * @param table The table to cleanup.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_data_table_cleanup(M3DataTable* table);

#ifdef __cplusplus
}
#endif

#endif /* M3_DATA_TABLE_H */