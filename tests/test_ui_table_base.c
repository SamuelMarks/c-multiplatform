/* clang-format off */
#include "ui_table_base.h"
#include "ui_dom_node.h"
#include "ui_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
int g_mock_append_child_fail_countdown = -1;

#define ACCUM_ERR(failed, expr) failed |= ((expr) != UI_ERROR_NONE)
#define ACCUM_FAIL(failed, expr) failed |= (expr)

static int test_sort_render_impl(void);

static size_t mock_get_row_count(void *user_data) {
  (void)user_data;
  return 105; /* 105 rows for pagination tests */
}

static size_t mock_get_col_count(void *user_data) {
  (void)user_data;
  return 3;
}

static ui_error_t mock_render_cell(size_t row, size_t col,
                                   struct ui_dom_node *cell_node,
                                   void *user_data) {
  char buf[64];
  (void)user_data;
#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "Cell %lu,%lu", (unsigned long)row,
            (unsigned long)col);
#else
  sprintf(buf, "Cell %lu,%lu", (unsigned long)row, (unsigned long)col);
#endif
  return ui_dom_node_set_attribute(cell_node, "data-content", buf);
}

static ui_error_t mock_render_header(size_t col,
                                     struct ui_dom_node *header_node,
                                     void *user_data) {
  char buf[64];
  (void)user_data;
#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "Header %lu", (unsigned long)col);
#else
  sprintf(buf, "Header %lu", (unsigned long)col);
#endif
  return ui_dom_node_set_attribute(header_node, "data-content", buf);
}

static int test_null_args(void) {
  struct ui_table_base *table = NULL;
  struct ui_table_model model;
  struct ui_table_model bad_model;
  struct ui_selection_model *sel_model;
  struct ui_table_column_config col_cfg;
  struct ui_table_sort_config sort_cfg;
  struct ui_table_pagination_config page_cfg;
  int failed = 0;

  model.get_row_count = mock_get_row_count;
  model.get_column_count = mock_get_col_count;
  model.render_cell = mock_render_cell;
  model.render_header = mock_render_header;
  model.user_data = NULL;

  bad_model = model;
  bad_model.get_row_count = NULL;
  failed |=
      (ui_table_base_create(&table, &bad_model) != UI_ERROR_INVALID_ARGUMENT);

  bad_model = model;
  bad_model.get_column_count = NULL;
  failed |=
      (ui_table_base_create(&table, &bad_model) != UI_ERROR_INVALID_ARGUMENT);

  bad_model = model;
  bad_model.render_cell = NULL;
  failed |=
      (ui_table_base_create(&table, &bad_model) != UI_ERROR_INVALID_ARGUMENT);

  bad_model = model;
  bad_model.render_header = NULL;
  failed |=
      (ui_table_base_create(&table, &bad_model) != UI_ERROR_INVALID_ARGUMENT);

  failed |= (ui_table_base_create(NULL, &model) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_table_base_create(&table, NULL) != UI_ERROR_INVALID_ARGUMENT);

  (void)ui_table_base_destroy(NULL);

  failed |= (ui_table_base_get_selection_model(NULL, &sel_model) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_table_base_set_column_config(NULL, 0, &col_cfg) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_table_base_set_sort_config(NULL, &sort_cfg) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_table_base_set_pagination_config(NULL, &page_cfg) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_table_base_render(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_table_base_bind_data(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT);

  ACCUM_ERR(failed, ui_table_base_create(&table, &model));
  failed |= (ui_table_base_get_selection_model(table, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_table_base_set_column_config(table, 0, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_table_base_set_sort_config(table, NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_table_base_set_pagination_config(table, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_table_base_render(table, NULL) != UI_ERROR_INVALID_ARGUMENT);

  (void)ui_table_base_destroy(table);
  return failed;
}

static int test_table_render_aria(void) {
  struct ui_table_model model;
  struct ui_table_base *table;
  struct ui_dom_node *container;
  struct ui_table_sort_config sort_cfg;
  struct ui_selection_model *sel_model;
  int failed = 0;

  model.get_row_count = mock_get_row_count;
  model.get_column_count = mock_get_col_count;
  model.render_cell = mock_render_cell;
  model.render_header = mock_render_header;
  model.user_data = NULL;

  ACCUM_ERR(failed, ui_table_base_create(&table, &model));

  sort_cfg.active_column_index = 1;
  sort_cfg.direction = UI_TABLE_SORT_DESCENDING;
  ACCUM_ERR(failed, ui_table_base_set_sort_config(table, &sort_cfg));

  sort_cfg.direction = UI_TABLE_SORT_ASCENDING;
  ACCUM_ERR(failed, ui_table_base_set_sort_config(table, &sort_cfg));

  sort_cfg.active_column_index = 99; /* OOB */
  failed |= (ui_table_base_set_sort_config(table, &sort_cfg) !=
             UI_ERROR_OUT_OF_BOUNDS);

  /* Select a row to hit is_selected */
  ACCUM_ERR(failed, ui_table_base_get_selection_model(table, &sel_model));
  ui_selection_model_select(sel_model, (void *)(size_t)2);

  /* Bind data */
  ACCUM_ERR(failed, ui_table_base_bind_data(table, NULL));

  ACCUM_ERR(failed, ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container));
  ACCUM_ERR(failed, ui_table_base_render(table, container));

  (void)ui_table_base_destroy(table);
  (void)ui_dom_node_destroy(container);
  return failed;
}

static int test_table_pagination(void) {
  struct ui_table_model model;
  struct ui_table_base *table;
  struct ui_dom_node *container;
  struct ui_table_pagination_config page_cfg;
  int failed = 0;

  model.get_row_count = mock_get_row_count;
  model.get_column_count = mock_get_col_count;
  model.render_cell = mock_render_cell;
  model.render_header = mock_render_header;
  model.user_data = NULL;

  ACCUM_ERR(failed, ui_table_base_create(&table, &model));

  page_cfg.page_size = 10;
  page_cfg.current_page = 10; /* Last page, should show 5 items */
  ACCUM_ERR(failed, ui_table_base_set_pagination_config(table, &page_cfg));

  ACCUM_ERR(failed, ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container));
  ACCUM_ERR(failed, ui_table_base_render(table, container));

  /* Page that does not exceed bounds (e.g. page 0) */
  page_cfg.current_page = 0;
  ACCUM_ERR(failed, ui_table_base_set_pagination_config(table, &page_cfg));
  ACCUM_ERR(failed, ui_table_base_render(table, container));

  /* Page past the end */
  page_cfg.current_page = 20;
  ACCUM_ERR(failed, ui_table_base_set_pagination_config(table, &page_cfg));
  ACCUM_ERR(failed, ui_table_base_render(table, container));

  (void)ui_table_base_destroy(table);
  (void)ui_dom_node_destroy(container);
  return failed;
}

static int test_table_column_sizing(void) {
  struct ui_table_model model;
  struct ui_table_base *table;
  struct ui_dom_node *container;
  struct ui_table_column_config col_cfg;
  int failed = 0;

  model.get_row_count = mock_get_row_count;
  model.get_column_count = mock_get_col_count;
  model.render_cell = mock_render_cell;
  model.render_header = mock_render_header;
  model.user_data = NULL;

  ACCUM_ERR(failed, ui_table_base_create(&table, &model));

  col_cfg.sizing = UI_TABLE_COLUMN_FIXED;
  col_cfg.width = 250.0f;
  col_cfg.min_width = 100.0f;
  col_cfg.max_width = 500.0f;
  ACCUM_ERR(failed, ui_table_base_set_column_config(table, 0, &col_cfg));

  failed |= (ui_table_base_set_column_config(table, 5, &col_cfg) !=
             UI_ERROR_OUT_OF_BOUNDS);

  ACCUM_ERR(failed, ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container));
  ACCUM_ERR(failed, ui_table_base_render(table, container));

  (void)ui_table_base_destroy(table);
  (void)ui_dom_node_destroy(container);
  return failed;
}

static size_t mock_get_zero_col_count(void *user_data) {
  (void)user_data;
  return 0;
}

static int test_zero_cols(void) {
  struct ui_table_model model;
  struct ui_table_base *table;
  struct ui_dom_node *container;
  int failed = 0;

  model.get_row_count = mock_get_row_count;
  model.get_column_count = mock_get_zero_col_count;
  model.render_cell = mock_render_cell;
  model.render_header = mock_render_header;
  model.user_data = NULL;

  ACCUM_ERR(failed, ui_table_base_create(&table, &model));
  ACCUM_ERR(failed, ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container));
  ACCUM_ERR(failed, ui_table_base_render(table, container));

  (void)ui_table_base_destroy(table);
  (void)ui_dom_node_destroy(container);
  return failed;
}

static ui_error_t mock_render_cell_fail(size_t row, size_t col,
                                        struct ui_dom_node *cell_node,
                                        void *user_data) {
  (void)row;
  (void)col;
  (void)cell_node;
  (void)user_data;
  return UI_ERROR_UNKNOWN;
}
static ui_error_t mock_render_header_fail(size_t col,
                                          struct ui_dom_node *header_node,
                                          void *user_data) {
  (void)col;
  (void)header_node;
  (void)user_data;
  return UI_ERROR_UNKNOWN;
}

static int test_render_fails(void) {
  struct ui_table_model model;
  struct ui_table_base *table;
  struct ui_dom_node *container;
  int failed = 0;

  model.get_row_count = mock_get_row_count;
  model.get_column_count = mock_get_col_count;
  model.render_cell = mock_render_cell;
  model.render_header = mock_render_header;
  model.user_data = NULL;

  ACCUM_ERR(failed, ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container));

  /* Test header render fail */
  model.render_header = mock_render_header_fail;
  ACCUM_ERR(failed, ui_table_base_create(&table, &model));
  failed |= (ui_table_base_render(table, container) != UI_ERROR_UNKNOWN);
  (void)ui_table_base_destroy(table);

  /* Test cell render fail */
  model.render_header = mock_render_header;
  model.render_cell = mock_render_cell_fail;
  ACCUM_ERR(failed, ui_table_base_create(&table, &model));
  failed |= (ui_table_base_render(table, container) != UI_ERROR_UNKNOWN);
  (void)ui_table_base_destroy(table);

  (void)ui_dom_node_destroy(container);
  return failed;
}

static int test_oom(void) {
  int failed = 0;
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_table_model model;
  struct ui_table_base *table;
  struct ui_dom_node *container;
  int i;

  model.get_row_count = mock_get_row_count;
  model.get_column_count = mock_get_col_count;
  model.render_cell = mock_render_cell;
  model.render_header = mock_render_header;
  model.user_data = NULL;

  g_malloc_fail_countdown = 0;
  failed |= (ui_table_base_create(&table, &model) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 1;
  failed |= (ui_table_base_create(&table, &model) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 2; /* inside col_configs array */
  failed |= (ui_table_base_create(&table, &model) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ACCUM_ERR(failed, ui_table_base_create(&table, &model));
  ACCUM_ERR(failed, ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container));

  /* Trigger OOM during render node creations. We loop to hit various branches
   */
  for (i = 0; i < 150; i++) {
    g_malloc_fail_countdown = i;
    ui_table_base_render(table, container);
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 500; i++) {
    g_mock_append_child_fail_countdown = i;
    ui_table_base_render(table, container);
  }
  g_mock_append_child_fail_countdown = -1;

  (void)ui_table_base_destroy(table);
  (void)ui_dom_node_destroy(container);
#endif
  return failed;
}

static int test_sort_render_impl(void) {
  struct ui_table_model model;
  struct ui_table_base *table;
  struct ui_dom_node *container;
  struct ui_table_sort_config sort_cfg;

  model.get_row_count = mock_get_row_count;
  model.get_column_count = mock_get_col_count;
  model.render_cell = mock_render_cell;
  model.render_header = mock_render_header;
  model.user_data = NULL;

  ui_table_base_create(&table, &model);

  sort_cfg.active_column_index = 0;
  sort_cfg.direction = UI_TABLE_SORT_ASCENDING;
  ui_table_base_set_sort_config(table, &sort_cfg);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
  ui_table_base_render(table, container);

  sort_cfg.direction = UI_TABLE_SORT_DESCENDING;
  ui_table_base_set_sort_config(table, &sort_cfg);
  ui_table_base_render(table, container);

  sort_cfg.direction = UI_TABLE_SORT_NONE;
  ui_table_base_set_sort_config(table, &sort_cfg);
  ui_table_base_render(table, container);

  (void)ui_table_base_destroy(table);
  (void)ui_dom_node_destroy(container);
  return 0;
}

int main(void) {
  int failed = 0;
  failed |= test_null_args();
  failed |= test_table_render_aria();
  failed |= test_table_pagination();
  failed |= test_table_column_sizing();
  failed |= test_zero_cols();
  failed |= test_render_fails();
  failed |= test_oom();
  failed |= test_sort_render_impl();

  if (!failed) {
    printf("All ui_table_base tests passed.\n");
  }
  return failed;
}
