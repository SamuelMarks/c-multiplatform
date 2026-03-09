#include "m3/m3_data_table.h"

#include <string.h>

#ifdef CMP_TESTING
static cmp_u32 g_m3_data_table_test_fail_point = 0u;

CMP_API int CMP_CALL m3_data_table_test_set_fail_point(cmp_u32 fail_point);
CMP_API int CMP_CALL m3_data_table_test_clear_fail_points(void);

int CMP_CALL m3_data_table_test_set_fail_point(cmp_u32 fail_point) {
  g_m3_data_table_test_fail_point = fail_point;
  return CMP_OK;
}

int CMP_CALL m3_data_table_test_clear_fail_points(void) {
  g_m3_data_table_test_fail_point = 0u;
  return CMP_OK;
}
#endif

CMP_API int CMP_CALL m3_data_table_init(M3DataTable *table) {
  if (table == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_m3_data_table_test_fail_point == 1u) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
#endif

  table->bounds.x = 0;
  table->bounds.y = 0;
  table->bounds.width = 0;
  table->bounds.height = 0;
  table->columns = NULL;
  table->column_count = 0;
  table->row_count = 0;
  table->rows_per_page = 10;
  table->current_page = 0;
  table->selectable = CMP_FALSE;
  table->selected_rows = NULL;

  return CMP_OK;
}

CMP_API int CMP_CALL m3_data_table_draw_headers(CMPPaintContext *ctx,
                                                const M3DataTable *table) {
  if (ctx == NULL || table == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_m3_data_table_test_fail_point == 2u) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
#endif

  if (table->column_count == 0 || table->columns == NULL) {
    return CMP_OK; /* Nothing to draw */
  }

  /* Dummy drawing for headers */

  return CMP_OK;
}

CMP_API int CMP_CALL m3_data_table_draw_row(CMPPaintContext *ctx,
                                            const M3DataTable *table,
                                            cmp_u32 row_index,
                                            const char **cells) {
  if (ctx == NULL || table == NULL || cells == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_m3_data_table_test_fail_point == 3u) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
#endif

  if (row_index >= table->row_count) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  /* Dummy drawing for a row */

  return CMP_OK;
}

CMP_API int CMP_CALL m3_data_table_cleanup(M3DataTable *table) {
  if (table == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_m3_data_table_test_fail_point == 4u) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
#endif

  /* User owns selected_rows and columns, nothing to free internally */
  table->selected_rows = NULL;
  table->columns = NULL;

  return CMP_OK;
}