/* clang-format off */
#include "greatest.h"
#include "ui_table_base.h"
#include "ui_dom_node.h"
#include "ui_error.h"
#include "ui_computed.h"
#include "ui_test_mock_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_table_base {
  struct ui_table_model model;
  struct ui_table_column_config *col_configs;
  size_t num_cols;
  struct ui_table_sort_config sort_config;
  struct ui_table_pagination_config pagination_config;
  struct ui_selection_model *selection_model;
  struct ui_computed *data_signal;
};

extern int g_malloc_fail_countdown;
int g_mock_append_child_fail_countdown = -1;
extern int g_table_mock_fail;
extern int g_table_mock_set_attr_fail_target;

static size_t mock_get_row_count(void *user_data) {
  if (user_data) {
  }
  return 105; /* 105 rows for pagination tests */
}

static size_t mock_get_col_count(void *user_data) {
  if (user_data) {
  }
  return 3;
}

static size_t mock_get_zero_count(void *user_data) {
  if (user_data) {
  }
  return 0;
}

static ui_error_t mock_render_cell(size_t row, size_t col,
                                   struct ui_dom_node *cell_node,
                                   void *user_data) {
  char buf[64];
  if (user_data) {
  }
#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "Cell %lu,%lu", (unsigned long)row,
            (unsigned long)col);
#else
  sprintf(buf, "Cell %lu,%lu", (unsigned long)row, (unsigned long)col);
#endif
  return ui_dom_node_set_attribute(cell_node, "data-content", buf);
}

static ui_error_t mock_render_cell_fail(size_t row, size_t col,
                                        struct ui_dom_node *cell_node,
                                        void *user_data) {
  int unused_r = (int)row;
  int unused_c = (int)col;
  struct ui_dom_node *unused_n = cell_node;
  void *unused_u = user_data;
  row = (size_t)unused_r;
  col = (size_t)unused_c;
  cell_node = unused_n;
  user_data = unused_u;
  return UI_ERROR_UNKNOWN;
}

static ui_error_t mock_render_header(size_t col,
                                     struct ui_dom_node *header_node,
                                     void *user_data) {
  char buf[64];
  if (user_data) {
  }
#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "Header %lu", (unsigned long)col);
#else
  sprintf(buf, "Header %lu", (unsigned long)col);
#endif
  return ui_dom_node_set_attribute(header_node, "data-content", buf);
}

static ui_error_t mock_render_header_fail(size_t col,
                                          struct ui_dom_node *header_node,
                                          void *user_data) {
  int unused_c = (int)col;
  struct ui_dom_node *unused_n = header_node;
  void *unused_u = user_data;
  col = (size_t)unused_c;
  header_node = unused_n;
  user_data = unused_u;
  return UI_ERROR_UNKNOWN;
}

TEST test_table_null_args(void) {
  struct ui_table_base *table = NULL;
  struct ui_table_model model;
  struct ui_table_model bad_model;
  struct ui_selection_model *sel_model;
  struct ui_table_column_config col_cfg;
  struct ui_table_sort_config sort_cfg;
  struct ui_table_pagination_config page_cfg;
  ui_error_t rc;

  model.get_row_count = mock_get_row_count;
  model.get_column_count = mock_get_col_count;
  model.render_cell = mock_render_cell;
  model.render_header = mock_render_header;
  model.user_data = NULL;

  bad_model = model;
  bad_model.get_row_count = NULL;
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_table_base_create(&table, &bad_model));

  bad_model = model;
  bad_model.get_column_count = NULL;
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_table_base_create(&table, &bad_model));

  bad_model = model;
  bad_model.render_cell = NULL;
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_table_base_create(&table, &bad_model));

  bad_model = model;
  bad_model.render_header = NULL;
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_table_base_create(&table, &bad_model));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_table_base_create(NULL, &model));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_table_base_create(&table, NULL));

  ASSERT_EQ(UI_ERROR_NONE, ui_table_base_destroy(NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_table_base_get_selection_model(NULL, &sel_model));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_table_base_set_column_config(NULL, 0, &col_cfg));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_table_base_set_sort_config(NULL, &sort_cfg));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_table_base_set_pagination_config(NULL, &page_cfg));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_table_base_render(NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_table_base_bind_data(NULL, NULL));

  rc = ui_table_base_create(&table, &model);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_table_base_get_selection_model(table, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_table_base_set_column_config(table, 0, NULL));
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS,
            ui_table_base_set_column_config(table, 100, &col_cfg));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_table_base_set_sort_config(table, NULL));
  sort_cfg.active_column_index = 100;
  sort_cfg.direction = UI_TABLE_SORT_ASCENDING;
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS,
            ui_table_base_set_sort_config(table, &sort_cfg));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_table_base_set_pagination_config(table, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_table_base_render(table, NULL));
  ASSERT_EQ(UI_ERROR_NONE, ui_table_base_bind_data(table, NULL));

  rc = ui_table_base_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_table_render_and_aria(void) {
  struct ui_table_model model;
  struct ui_table_base *table;
  struct ui_dom_node *container;
  struct ui_table_sort_config sort_cfg;
  struct ui_selection_model *sel_model = NULL;
  ui_error_t rc;

  model.get_row_count = mock_get_row_count;
  model.get_column_count = mock_get_col_count;
  model.render_cell = mock_render_cell;
  model.render_header = mock_render_header;
  model.user_data = NULL;

  rc = ui_table_base_create(&table, &model);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_table_base_get_selection_model(table, &sel_model);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sel_model != NULL);

  rc = ui_selection_model_select(sel_model, (void *)(size_t)0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  sort_cfg.active_column_index = 0;
  sort_cfg.direction = UI_TABLE_SORT_ASCENDING;
  rc = ui_table_base_set_sort_config(table, &sort_cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_table_base_render(table, container);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  sort_cfg.direction = UI_TABLE_SORT_DESCENDING;
  rc = ui_table_base_set_sort_config(table, &sort_cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_table_base_render(table, container);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  sort_cfg.direction = UI_TABLE_SORT_NONE;
  rc = ui_table_base_set_sort_config(table, &sort_cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_table_base_render(table, container);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_table_base_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(container);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_table_pagination_and_sizing(void) {
  struct ui_table_model model;
  struct ui_table_base *table;
  struct ui_dom_node *container;
  struct ui_table_pagination_config page_cfg;
  struct ui_table_column_config col_cfg;
  ui_error_t rc;

  model.get_row_count = mock_get_row_count;
  model.get_column_count = mock_get_col_count;
  model.render_cell = mock_render_cell;
  model.render_header = mock_render_header;
  model.user_data = NULL;

  rc = ui_table_base_create(&table, &model);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test flex and fixed column sizing */
  col_cfg.sizing = UI_TABLE_COLUMN_FLEX;
  col_cfg.width = 2.0f;
  col_cfg.min_width = 10.0f;
  col_cfg.max_width = 100.0f;
  rc = ui_table_base_set_column_config(table, 0, &col_cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  col_cfg.sizing = UI_TABLE_COLUMN_FIXED;
  col_cfg.width = 150.0f;
  rc = ui_table_base_set_column_config(table, 1, &col_cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Pagination: page 0, size 10 */
  page_cfg.page_size = 10;
  page_cfg.current_page = 0;
  rc = ui_table_base_set_pagination_config(table, &page_cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_table_base_render(table, container);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Pagination: last page extending past total rows */
  page_cfg.page_size = 10;
  page_cfg.current_page = 10;
  rc = ui_table_base_set_pagination_config(table, &page_cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_table_base_render(table, container);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Pagination: current page far beyond total rows */
  page_cfg.page_size = 10;
  page_cfg.current_page = 50;
  rc = ui_table_base_set_pagination_config(table, &page_cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_table_base_render(table, container);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_table_base_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(container);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Table with zero columns and zero rows */
  model.get_row_count = mock_get_zero_count;
  model.get_column_count = mock_get_zero_count;
  rc = ui_table_base_create(&table, &model);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_table_base_render(table, container);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_table_base_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(container);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_table_error_branches(void) {
  struct ui_table_model model;
  struct ui_table_base *table;
  struct ui_dom_node *container;
  ui_error_t rc;
  int i;

  model.get_row_count = mock_get_row_count;
  model.get_column_count = mock_get_col_count;
  model.render_cell = mock_render_cell;
  model.render_header = mock_render_header;
  model.user_data = NULL;

  rc = ui_table_base_create(&table, &model);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Select row 0 so is_selected attribute branch is tested */
  rc = ui_selection_model_select(table->selection_model, (void *)(size_t)0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 1. Header render failure */
  table->model.render_header = mock_render_header_fail;
  rc = ui_table_base_render(table, container);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  table->model.render_header = mock_render_header;

  /* 2. Cell render failure */
  table->model.render_cell = mock_render_cell_fail;
  rc = ui_table_base_render(table, container);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  table->model.render_cell = mock_render_cell;

  /* 3. Mock set_attribute failures across all targets (1..15) */
  for (i = 1; i <= 15; i++) {
    g_table_mock_set_attr_fail_target = i;
    rc = ui_table_base_render(table, container);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_table_mock_set_attr_fail_target = 0;
  }

  /* 4. Mock selection_model failure (line 400) */
  g_table_mock_fail = 3;
  rc = ui_table_base_render(table, container);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_table_mock_fail = 0;

  /* 5. Mock append_child failures with destroy mock (g_table_mock_fail = 1,
   * true branch) */
  for (i = 0; i < 10; i++) {
    g_mock_append_child_fail_countdown = i;
    g_table_mock_fail = 1;
    rc = ui_table_base_render(table, container);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_table_mock_fail = 0;
    g_mock_append_child_fail_countdown = -1;
  }

  /* 6. Mock append_child failures without destroy mock (false branch) */
  for (i = 0; i < 10; i++) {
    g_mock_append_child_fail_countdown = i;
    rc = ui_table_base_render(table, container);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_mock_append_child_fail_countdown = -1;
  }

  rc = ui_table_base_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(container);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 7. Test final append to container failure (1 row, 1 col) */
  {
    struct ui_table_model m1;
    m1.get_row_count = mock_get_col_count;    /* 3 rows */
    m1.get_column_count = mock_get_col_count; /* 3 cols */
    m1.render_cell = mock_render_cell;
    m1.render_header = mock_render_header;
    m1.user_data = NULL;
    rc = ui_table_base_create(&table, &m1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* Total append calls = 1 (thead) + 1 (header_row) + 3 (header_cells) +
       1 (tbody) + 3*(1 + 3) (rows and cells = 12) = 18.
       Append call 18 is table_root to container! */
    g_mock_append_child_fail_countdown = 18;
    rc = ui_table_base_render(table, container);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_mock_append_child_fail_countdown = -1;

    rc = ui_table_base_destroy(table);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_destroy(container);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  PASS();
}

static size_t mock_get_one_count(void *user_data) {
  if (user_data) {
  }
  return 1;
}

TEST test_table_oom(void) {
  struct ui_table_model model;
  struct ui_table_base *table = NULL;
  struct ui_dom_node *container;
  ui_error_t rc;
  int i;

  model.get_row_count = mock_get_row_count;
  model.get_column_count = mock_get_col_count;
  model.render_cell = mock_render_cell;
  model.render_header = mock_render_header;
  model.user_data = NULL;

  /* Creation OOM loop */
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    table = NULL;
    rc = ui_table_base_create(&table, &model);
    if (rc == UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      rc = ui_table_base_destroy(table);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    ASSERT(table == NULL);
  }
  g_malloc_fail_countdown = -1;

  /* Render OOM loop on a 1-row, 1-col table so all node creations are tested */
  model.get_row_count = mock_get_one_count;
  model.get_column_count = mock_get_one_count;
  rc = ui_table_base_create(&table, &model);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 50; i++) {
    g_malloc_fail_countdown = i;
    ui_table_base_render(table, container);
  }
  g_malloc_fail_countdown = -1;

  rc = ui_table_base_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(container);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(ui_table_base_suite) {
  RUN_TEST(test_table_null_args);
  RUN_TEST(test_table_render_and_aria);
  RUN_TEST(test_table_pagination_and_sizing);
  RUN_TEST(test_table_error_branches);
  RUN_TEST(test_table_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_table_base_suite);
  GREATEST_MAIN_END();
}
