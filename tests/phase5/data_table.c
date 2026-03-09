#include "m3/m3_data_table.h"
#include "test_utils.h"

extern int CMP_CALL m3_data_table_test_set_fail_point(cmp_u32 fail_point);
extern int CMP_CALL m3_data_table_test_clear_fail_points(void);

static int test_data_table_init(void) {
  M3DataTable table;

  CMP_TEST_EXPECT(m3_data_table_init(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_data_table_test_set_fail_point(1u));
  CMP_TEST_EXPECT(m3_data_table_init(&table), CMP_ERR_OUT_OF_MEMORY);
  CMP_TEST_OK(m3_data_table_test_clear_fail_points());

  CMP_TEST_OK(m3_data_table_init(&table));
  CMP_TEST_ASSERT(table.column_count == 0);
  CMP_TEST_ASSERT(table.row_count == 0);

  return 0;
}

static int test_data_table_draw_headers(void) {
  M3DataTable table;
  CMPPaintContext ctx;

  CMP_TEST_OK(m3_data_table_init(&table));

  CMP_TEST_EXPECT(m3_data_table_draw_headers(NULL, &table),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_data_table_draw_headers(&ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_data_table_test_set_fail_point(2u));
  CMP_TEST_EXPECT(m3_data_table_draw_headers(&ctx, &table),
                  CMP_ERR_OUT_OF_MEMORY);
  CMP_TEST_OK(m3_data_table_test_clear_fail_points());

  /* Empty table */
  CMP_TEST_OK(m3_data_table_draw_headers(&ctx, &table));

  table.column_count = 1;
  table.columns = NULL;
  CMP_TEST_OK(m3_data_table_draw_headers(&ctx, &table));

  /* Non-empty table */
  table.column_count = 1;
  M3DataTableColumn col = {"Test", CMP_FALSE, CMP_FALSE, M3_SORT_NONE, NULL};
  table.columns = &col;

  CMP_TEST_OK(m3_data_table_draw_headers(&ctx, &table));

  return 0;
}

static int test_data_table_draw_row(void) {
  M3DataTable table;
  CMPPaintContext ctx;
  const char *cells[] = {"Cell1"};

  CMP_TEST_OK(m3_data_table_init(&table));

  CMP_TEST_EXPECT(m3_data_table_draw_row(NULL, &table, 0, cells),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_data_table_draw_row(&ctx, NULL, 0, cells),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_data_table_draw_row(&ctx, &table, 0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_data_table_test_set_fail_point(3u));
  CMP_TEST_EXPECT(m3_data_table_draw_row(&ctx, &table, 0, cells),
                  CMP_ERR_OUT_OF_MEMORY);
  CMP_TEST_OK(m3_data_table_test_clear_fail_points());

  /* Invalid row index */
  CMP_TEST_EXPECT(m3_data_table_draw_row(&ctx, &table, 0, cells),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Valid row index */
  table.row_count = 1;
  CMP_TEST_OK(m3_data_table_draw_row(&ctx, &table, 0, cells));

  return 0;
}

static int test_data_table_cleanup(void) {
  M3DataTable table;
  CMP_TEST_OK(m3_data_table_init(&table));

  CMP_TEST_EXPECT(m3_data_table_cleanup(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_data_table_test_set_fail_point(4u));
  CMP_TEST_EXPECT(m3_data_table_cleanup(&table), CMP_ERR_OUT_OF_MEMORY);
  CMP_TEST_OK(m3_data_table_test_clear_fail_points());

  CMP_TEST_OK(m3_data_table_cleanup(&table));

  return 0;
}

int main(void) {
  int result = 0;
  result |= test_data_table_init();
  result |= test_data_table_draw_headers();
  result |= test_data_table_draw_row();
  result |= test_data_table_cleanup();
  return result == 0 ? 0 : 1;
}
