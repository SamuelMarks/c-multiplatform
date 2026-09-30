/**
 * @file md3_data_hierarchy.c
 * @brief Implementation of Material 3 Data Tables and Hierarchical
 * Presentation.
 */

/* clang-format off */
#include "material3/md3_data_hierarchy.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
/** @brief Global flag to simulate failures in mocked dependencies. */
int g_md3_data_hierarchy_mock_fail = 0;

/**
 * @brief Mock for ui_table_base_create.
 * @param out Output table.
 * @param m Table model.
 * @return ui_error_t result code.
 */
static ui_error_t mock_table_base_create(struct ui_table_base **out,
                                         const struct ui_table_model *m) {
  if (g_md3_data_hierarchy_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_table_base_create(out, m);
}

/**
 * @brief Mock for ui_table_base_destroy.
 * @param b Table pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_table_base_destroy(struct ui_table_base *b) {
  if (g_md3_data_hierarchy_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_table_base_destroy(b);
}

/**
 * @brief Mock for ui_datagrid_base_create.
 * @param out Output datagrid.
 * @return ui_error_t result code.
 */
static ui_error_t mock_datagrid_base_create(struct ui_datagrid_base **out) {
  if (g_md3_data_hierarchy_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_datagrid_base_create(out);
}

/**
 * @brief Mock for ui_datagrid_base_destroy.
 * @param b Datagrid pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_datagrid_base_destroy(struct ui_datagrid_base *b) {
  if (g_md3_data_hierarchy_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_datagrid_base_destroy(b);
}

/**
 * @brief Mock for ui_datagrid_base_resize_column.
 * @param b Datagrid pointer.
 * @param c Column index.
 * @param w Width.
 * @return ui_error_t result code.
 */
static ui_error_t mock_datagrid_base_resize_column(struct ui_datagrid_base *b,
                                                   int c, float w) {
  if (g_md3_data_hierarchy_mock_fail == 5) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_datagrid_base_resize_column(b, c, w);
}

/**
 * @brief Mock for ui_pagination_base_create.
 * @param out Output pagination pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_pagination_base_create(struct ui_pagination_base **out) {
  if (g_md3_data_hierarchy_mock_fail == 6) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_pagination_base_create(out);
}

/**
 * @brief Mock for ui_pagination_base_destroy.
 * @param b Pagination pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_pagination_base_destroy(struct ui_pagination_base *b) {
  if (g_md3_data_hierarchy_mock_fail == 7) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_pagination_base_destroy(b);
}

/**
 * @brief Mock for ui_pagination_base_set_config.
 * @param b Pagination pointer.
 * @param t Total items.
 * @param s Page size.
 * @return ui_error_t result code.
 */
static ui_error_t mock_pagination_base_set_config(struct ui_pagination_base *b,
                                                  size_t t, size_t s) {
  if (g_md3_data_hierarchy_mock_fail == 8) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_pagination_base_set_config(b, t, s);
}

/**
 * @brief Mock for ui_pagination_base_get_current_page.
 * @param b Pagination pointer.
 * @param out Output page index.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_pagination_base_get_current_page(const struct ui_pagination_base *b,
                                      size_t *out) {
  if (g_md3_data_hierarchy_mock_fail == 9) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_pagination_base_get_current_page(b, out);
}

/**
 * @brief Mock for ui_pagination_base_set_current_page.
 * @param b Pagination pointer.
 * @param p Page index.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_pagination_base_set_current_page(struct ui_pagination_base *b, size_t p) {
  if (g_md3_data_hierarchy_mock_fail == 10) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_pagination_base_set_current_page(b, p);
}

/**
 * @brief Mock for ui_pagination_base_get_total_pages.
 * @param b Pagination pointer.
 * @param out Output total pages.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_pagination_base_get_total_pages(const struct ui_pagination_base *b,
                                     size_t *out) {
  if (g_md3_data_hierarchy_mock_fail == 11) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_pagination_base_get_total_pages(b, out);
}

/**
 * @brief Mock for ui_pagination_base_next.
 * @param b Pagination pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_pagination_base_next(struct ui_pagination_base *b) {
  if (g_md3_data_hierarchy_mock_fail == 12) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_pagination_base_next(b);
}

/**
 * @brief Mock for ui_pagination_base_previous.
 * @param b Pagination pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_pagination_base_previous(struct ui_pagination_base *b) {
  if (g_md3_data_hierarchy_mock_fail == 13) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_pagination_base_previous(b);
}

/**
 * @brief Mock for ui_pagination_base_first.
 * @param b Pagination pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_pagination_base_first(struct ui_pagination_base *b) {
  if (g_md3_data_hierarchy_mock_fail == 14) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_pagination_base_first(b);
}

/**
 * @brief Mock for ui_pagination_base_last.
 * @param b Pagination pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_pagination_base_last(struct ui_pagination_base *b) {
  if (g_md3_data_hierarchy_mock_fail == 15) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_pagination_base_last(b);
}

/**
 * @brief Mock for ui_sort_header_base_create.
 * @param out Output sort header.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_sort_header_base_create(struct ui_sort_header_base **out) {
  if (g_md3_data_hierarchy_mock_fail == 16) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_sort_header_base_create(out);
}

/**
 * @brief Mock for ui_sort_header_base_destroy.
 * @param b Sort header pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_sort_header_base_destroy(struct ui_sort_header_base *b) {
  if (g_md3_data_hierarchy_mock_fail == 17) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_sort_header_base_destroy(b);
}

/**
 * @brief Mock for ui_sort_header_base_toggle.
 * @param b Sort header pointer.
 * @param id Column ID.
 * @return ui_error_t result code.
 */
static ui_error_t mock_sort_header_base_toggle(struct ui_sort_header_base *b,
                                               void *id) {
  if (g_md3_data_hierarchy_mock_fail == 18) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_sort_header_base_toggle(b, id);
}

/**
 * @brief Mock for ui_tree_base_create.
 * @param out Output tree pointer.
 * @param m Tree model.
 * @return ui_error_t result code.
 */
static ui_error_t mock_tree_base_create(struct ui_tree_base **out,
                                        const struct ui_tree_model *m) {
  if (g_md3_data_hierarchy_mock_fail == 19) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_tree_base_create(out, m);
}

/**
 * @brief Mock for ui_tree_base_destroy.
 * @param b Tree pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_tree_base_destroy(struct ui_tree_base *b) {
  if (g_md3_data_hierarchy_mock_fail == 20) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_tree_base_destroy(b);
}

/**
 * @brief Mock for ui_transfer_list_base_init.
 * @param b Transfer list base.
 * @param c Component.
 * @param a Accessor.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_transfer_list_base_init(struct ui_transfer_list_base *b,
                             struct ui_component *c,
                             struct ui_control_value_accessor *a) {
  if (g_md3_data_hierarchy_mock_fail == 21) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_transfer_list_base_init(b, c, a);
}

#undef ui_table_base_create
#define ui_table_base_create mock_table_base_create
#undef ui_table_base_destroy
#define ui_table_base_destroy mock_table_base_destroy
#undef ui_datagrid_base_create
#define ui_datagrid_base_create mock_datagrid_base_create
#undef ui_datagrid_base_destroy
#define ui_datagrid_base_destroy mock_datagrid_base_destroy
#undef ui_datagrid_base_resize_column
#define ui_datagrid_base_resize_column mock_datagrid_base_resize_column
#undef ui_pagination_base_create
#define ui_pagination_base_create mock_pagination_base_create
#undef ui_pagination_base_destroy
#define ui_pagination_base_destroy mock_pagination_base_destroy
#undef ui_pagination_base_set_config
#define ui_pagination_base_set_config mock_pagination_base_set_config
#undef ui_pagination_base_get_current_page
#define ui_pagination_base_get_current_page                                    \
  mock_pagination_base_get_current_page
#undef ui_pagination_base_set_current_page
#define ui_pagination_base_set_current_page                                    \
  mock_pagination_base_set_current_page
#undef ui_pagination_base_get_total_pages
#define ui_pagination_base_get_total_pages mock_pagination_base_get_total_pages
#undef ui_pagination_base_next
#define ui_pagination_base_next mock_pagination_base_next
#undef ui_pagination_base_previous
#define ui_pagination_base_previous mock_pagination_base_previous
#undef ui_pagination_base_first
#define ui_pagination_base_first mock_pagination_base_first
#undef ui_pagination_base_last
#define ui_pagination_base_last mock_pagination_base_last
#undef ui_sort_header_base_create
#define ui_sort_header_base_create mock_sort_header_base_create
#undef ui_sort_header_base_destroy
#define ui_sort_header_base_destroy mock_sort_header_base_destroy
#undef ui_sort_header_base_toggle
#define ui_sort_header_base_toggle mock_sort_header_base_toggle
#undef ui_tree_base_create
#define ui_tree_base_create mock_tree_base_create
#undef ui_tree_base_destroy
#define ui_tree_base_destroy mock_tree_base_destroy
#undef ui_transfer_list_base_init
#define ui_transfer_list_base_init mock_transfer_list_base_init
#endif

#ifdef UI_TEST_MOCK_ALLOC
/** @brief Global default table model instance for mock tests. */
struct ui_table_model g_default_table_model;
/** @brief Global default tree model instance for mock tests. */
struct ui_tree_model g_default_tree_model;
#define s_default_table_model g_default_table_model
#define s_default_tree_model g_default_tree_model
#else
static struct ui_table_model s_default_table_model;
static struct ui_tree_model s_default_tree_model;
#endif

/* ========================================================================= */
/* md3_table                                                                 */
/* ========================================================================= */

static size_t md3_default_row_count(void *user_data) {
  if (user_data != NULL) {
    return 0;
  }
  return 0;
}

static size_t md3_default_col_count(void *user_data) {
  if (user_data != NULL) {
    return 0;
  }
  return 0;
}

static ui_error_t md3_default_render_cell(size_t row, size_t col,
                                          struct ui_dom_node *cell_node,
                                          void *user_data) {
  if (row > 0 || col > 0 || cell_node != NULL || user_data != NULL) {
    return UI_ERROR_NONE;
  }
  return UI_ERROR_NONE;
}

static ui_error_t md3_default_render_header(size_t col,
                                            struct ui_dom_node *header_node,
                                            void *user_data) {
  if (col > 0 || header_node != NULL || user_data != NULL) {
    return UI_ERROR_NONE;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_table_create(struct ui_engine *engine,
                            struct md3_table **out_table) {
  struct md3_table *tbl;
  ui_error_t rc;

  if (engine == NULL || out_table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tbl = (struct md3_table *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_table));
  if (tbl == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(tbl, 0, sizeof(struct md3_table));

  s_default_table_model.get_row_count = md3_default_row_count;
  s_default_table_model.get_column_count = md3_default_col_count;
  s_default_table_model.render_cell = md3_default_render_cell;
  s_default_table_model.render_header = md3_default_render_header;
  s_default_table_model.user_data = NULL;

  rc = ui_table_base_create(&tbl->table_base, &s_default_table_model);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tbl);
    return rc;
  }

  rc = ui_datagrid_base_create(&tbl->datagrid_base);
  if (rc != UI_ERROR_NONE) {
    ui_table_base_destroy(tbl->table_base);
    C_MULTIPLATFORM_FREE(tbl);
    return rc;
  }

  tbl->density = MD3_TABLE_DENSITY_NORMAL;
  tbl->sticky_header = 1;
  tbl->sticky_first_column = 0;
  tbl->sticky_last_column = 0;
  tbl->is_selectable = 0;
  tbl->glassmorphism_enabled = 0;
  tbl->virtual_scroll_enabled = 0;
  tbl->virtual_viewport_height = 400;
  tbl->horizontal_scroll_enabled = 1;
  tbl->hovered_row = 0;
  tbl->has_hovered_row = 0;
  tbl->selected_count = 0;
  tbl->expanded_count = 0;
  tbl->total_rows = 0;

  *out_table = tbl;
  return UI_ERROR_NONE;
}

ui_error_t md3_table_destroy(struct md3_table *table) {
  ui_error_t rc;

  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (table->table_base != NULL) {
    rc = ui_table_base_destroy(table->table_base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    table->table_base = NULL;
  }

  if (table->datagrid_base != NULL) {
    rc = ui_datagrid_base_destroy(table->datagrid_base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    table->datagrid_base = NULL;
  }

  C_MULTIPLATFORM_FREE(table);
  return UI_ERROR_NONE;
}

ui_error_t md3_table_set_density(struct md3_table *table,
                                 enum md3_table_density density) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  table->density = density;
  return UI_ERROR_NONE;
}

ui_error_t md3_table_get_row_height(const struct md3_table *table,
                                    float *out_height_dp) {
  if (table == NULL || out_height_dp == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (table->density == MD3_TABLE_DENSITY_DENSE) {
    *out_height_dp = 40.0f;
  } else if (table->density == MD3_TABLE_DENSITY_RELAXED) {
    *out_height_dp = 64.0f;
  } else {
    *out_height_dp = 52.0f;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_table_set_model(struct md3_table *table,
                               const struct ui_table_model *model) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (model != NULL && model->get_row_count != NULL) {
    table->total_rows = model->get_row_count(model->user_data);
  } else {
    table->total_rows = 0;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_table_set_sticky_header(struct md3_table *table, int sticky) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  table->sticky_header = sticky ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_table_set_sticky_first_column(struct md3_table *table,
                                             int sticky) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  table->sticky_first_column = sticky ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_table_set_sticky_last_column(struct md3_table *table,
                                            int sticky) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  table->sticky_last_column = sticky ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_table_set_selectable(struct md3_table *table, int selectable) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  table->is_selectable = selectable ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_table_select_row(struct md3_table *table, size_t row_index,
                                int selected) {
  size_t i;
  int found = 0;
  size_t found_idx = 0;

  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < table->selected_count; ++i) {
    if (table->selected_rows[i] == row_index) {
      found = 1;
      found_idx = i;
      break;
    }
  }

  if (selected && !found) {
    if (table->selected_count < 64) {
      table->selected_rows[table->selected_count++] = row_index;
    }
  } else if (!selected && found) {
    for (i = found_idx; i + 1 < table->selected_count; ++i) {
      table->selected_rows[i] = table->selected_rows[i + 1];
    }
    table->selected_count--;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_table_is_row_selected(const struct md3_table *table,
                                     size_t row_index, int *out_selected) {
  size_t i;

  if (table == NULL || out_selected == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_selected = 0;
  for (i = 0; i < table->selected_count; ++i) {
    if (table->selected_rows[i] == row_index) {
      *out_selected = 1;
      break;
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_table_select_all(struct md3_table *table, int selected) {
  size_t i;
  size_t limit;

  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!selected) {
    table->selected_count = 0;
    return UI_ERROR_NONE;
  }

  limit = table->total_rows < 64 ? table->total_rows : 64;
  table->selected_count = limit;
  for (i = 0; i < limit; ++i) {
    table->selected_rows[i] = i;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_table_get_selection_state(const struct md3_table *table,
                                         int *out_all_selected,
                                         int *out_indeterminate) {
  if (table == NULL || out_all_selected == NULL || out_indeterminate == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (table->total_rows == 0 || table->selected_count == 0) {
    *out_all_selected = 0;
    *out_indeterminate = 0;
  } else if (table->selected_count >= table->total_rows) {
    *out_all_selected = 1;
    *out_indeterminate = 0;
  } else {
    *out_all_selected = 0;
    *out_indeterminate = 1;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_table_set_hovered_row(struct md3_table *table,
                                     size_t row_index) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  table->hovered_row = row_index;
  table->has_hovered_row = 1;
  return UI_ERROR_NONE;
}

ui_error_t md3_table_set_virtual_scroll(struct md3_table *table, int enabled,
                                        size_t viewport_height) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  table->virtual_scroll_enabled = enabled ? 1 : 0;
  table->virtual_viewport_height = viewport_height;
  return UI_ERROR_NONE;
}

ui_error_t md3_table_set_horizontal_scroll(struct md3_table *table,
                                           int enabled) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  table->horizontal_scroll_enabled = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_table_set_glassmorphism(struct md3_table *table, int enabled) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  table->glassmorphism_enabled = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_table_toggle_row_expansion(struct md3_table *table,
                                          size_t row_index) {
  size_t i;
  int found = 0;
  size_t found_idx = 0;

  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < table->expanded_count; ++i) {
    if (table->expanded_rows[i] == row_index) {
      found = 1;
      found_idx = i;
      break;
    }
  }

  if (found) {
    for (i = found_idx; i + 1 < table->expanded_count; ++i) {
      table->expanded_rows[i] = table->expanded_rows[i + 1];
    }
    table->expanded_count--;
  } else {
    if (table->expanded_count < 64) {
      table->expanded_rows[table->expanded_count++] = row_index;
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_table_is_row_expanded(const struct md3_table *table,
                                     size_t row_index, int *out_expanded) {
  size_t i;

  if (table == NULL || out_expanded == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_expanded = 0;
  for (i = 0; i < table->expanded_count; ++i) {
    if (table->expanded_rows[i] == row_index) {
      *out_expanded = 1;
      break;
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_table_resize_column(struct md3_table *table, size_t col_index,
                                   float new_width) {
  if (table == NULL || table->datagrid_base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_datagrid_base_resize_column(table->datagrid_base, (int)col_index,
                                        new_width);
}

/* ========================================================================= */
/* md3_paginator                                                             */
/* ========================================================================= */

ui_error_t md3_paginator_create(struct ui_engine *engine,
                                struct md3_paginator **out_paginator) {
  struct md3_paginator *p;
  ui_error_t rc;

  if (engine == NULL || out_paginator == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  p = (struct md3_paginator *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_paginator));
  if (p == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(p, 0, sizeof(struct md3_paginator));

  rc = ui_pagination_base_create(&p->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(p);
    return rc;
  }

  p->total_items = 0;
  p->page_size = 10;
  p->current_page = 0;
  p->is_compact = 0;
  p->is_expressive = 0;
  p->page_size_options[0] = 5;
  p->page_size_options[1] = 10;
  p->page_size_options[2] = 25;
  p->page_size_options[3] = 50;
  p->page_size_options[4] = 100;
  p->page_size_option_count = 5;

  *out_paginator = p;
  return UI_ERROR_NONE;
}

ui_error_t md3_paginator_destroy(struct md3_paginator *paginator) {
  ui_error_t rc;

  if (paginator == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (paginator->base != NULL) {
    rc = ui_pagination_base_destroy(paginator->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    paginator->base = NULL;
  }

  C_MULTIPLATFORM_FREE(paginator);
  return UI_ERROR_NONE;
}

ui_error_t md3_paginator_set_config(struct md3_paginator *paginator,
                                    size_t total_items, size_t page_size) {
  ui_error_t rc;

  if (paginator == NULL || paginator->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_pagination_base_set_config(paginator->base, total_items, page_size);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  paginator->total_items = total_items;
  paginator->page_size = page_size;
  rc = ui_pagination_base_get_current_page(paginator->base,
                                           &paginator->current_page);
  return rc;
}

ui_error_t md3_paginator_set_current_page(struct md3_paginator *paginator,
                                          size_t page_index) {
  ui_error_t rc;

  if (paginator == NULL || paginator->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_pagination_base_set_current_page(paginator->base, page_index);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_pagination_base_get_current_page(paginator->base,
                                           &paginator->current_page);
  return rc;
}

ui_error_t md3_paginator_get_current_page(const struct md3_paginator *paginator,
                                          size_t *out_page) {
  if (paginator == NULL || paginator->base == NULL || out_page == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_pagination_base_get_current_page(paginator->base, out_page);
}

ui_error_t md3_paginator_get_total_pages(const struct md3_paginator *paginator,
                                         size_t *out_total_pages) {
  if (paginator == NULL || paginator->base == NULL || out_total_pages == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_pagination_base_get_total_pages(paginator->base, out_total_pages);
}

ui_error_t md3_paginator_next(struct md3_paginator *paginator) {
  ui_error_t rc;

  if (paginator == NULL || paginator->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_pagination_base_next(paginator->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_pagination_base_get_current_page(paginator->base,
                                             &paginator->current_page);
}

ui_error_t md3_paginator_prev(struct md3_paginator *paginator) {
  ui_error_t rc;

  if (paginator == NULL || paginator->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_pagination_base_previous(paginator->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_pagination_base_get_current_page(paginator->base,
                                             &paginator->current_page);
}

ui_error_t md3_paginator_first(struct md3_paginator *paginator) {
  ui_error_t rc;

  if (paginator == NULL || paginator->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_pagination_base_first(paginator->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_pagination_base_get_current_page(paginator->base,
                                             &paginator->current_page);
}

ui_error_t md3_paginator_last(struct md3_paginator *paginator) {
  ui_error_t rc;

  if (paginator == NULL || paginator->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_pagination_base_last(paginator->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_pagination_base_get_current_page(paginator->base,
                                             &paginator->current_page);
}

ui_error_t md3_paginator_set_page_size_options(struct md3_paginator *paginator,
                                               const size_t *options,
                                               size_t count) {
  size_t i;

  if (paginator == NULL || options == NULL || count == 0 || count > 8) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  paginator->page_size_option_count = count;
  for (i = 0; i < count; ++i) {
    paginator->page_size_options[i] = options[i];
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_paginator_get_page_size(const struct md3_paginator *paginator,
                                       size_t *out_page_size) {
  if (paginator == NULL || out_page_size == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_page_size = paginator->page_size;
  return UI_ERROR_NONE;
}

ui_error_t md3_paginator_set_page_size(struct md3_paginator *paginator,
                                       size_t page_size) {
  if (paginator == NULL || page_size == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return md3_paginator_set_config(paginator, paginator->total_items, page_size);
}

ui_error_t md3_paginator_get_range_label(const struct md3_paginator *paginator,
                                         char *buffer, size_t buffer_size) {
  size_t start;
  size_t end;

  if (paginator == NULL || buffer == NULL || buffer_size == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (paginator->total_items == 0) {
    start = 0;
    end = 0;
  } else {
    start = paginator->current_page * paginator->page_size + 1;
    end = (paginator->current_page + 1) * paginator->page_size;
    if (end > paginator->total_items) {
      end = paginator->total_items;
    }
  }

#if defined(_MSC_VER)
  sprintf_s(buffer, buffer_size, "%lu - %lu of %lu", (unsigned long)start,
            (unsigned long)end, (unsigned long)paginator->total_items);
#else
  snprintf(buffer, buffer_size, "%lu - %lu of %lu", (unsigned long)start,
           (unsigned long)end, (unsigned long)paginator->total_items);
#endif

  return UI_ERROR_NONE;
}

ui_error_t md3_paginator_set_compact(struct md3_paginator *paginator,
                                     int compact) {
  if (paginator == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  paginator->is_compact = compact ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_paginator_set_expressive(struct md3_paginator *paginator,
                                        int enabled) {
  if (paginator == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  paginator->is_expressive = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_sort_header                                                           */
/* ========================================================================= */

ui_error_t md3_sort_header_create(struct ui_engine *engine,
                                  const char *column_id,
                                  struct md3_sort_header **out_header) {
  struct md3_sort_header *h;
  ui_error_t rc;

  if (engine == NULL || column_id == NULL || out_header == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  h = (struct md3_sort_header *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_sort_header));
  if (h == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(h, 0, sizeof(struct md3_sort_header));

  rc = ui_sort_header_base_create(&h->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(h);
    return rc;
  }

#if defined(_MSC_VER)
  strncpy_s(h->column_id, sizeof(h->column_id), column_id,
            sizeof(h->column_id) - 1);
#else
  strncpy(h->column_id, column_id, sizeof(h->column_id) - 1);
  h->column_id[sizeof(h->column_id) - 1] = '\0';
#endif

  h->direction = UI_SORT_NONE;
  h->allow_none = 1;
  h->priority = 0;
  h->is_expressive = 0;

  *out_header = h;
  return UI_ERROR_NONE;
}

ui_error_t md3_sort_header_destroy(struct md3_sort_header *header) {
  ui_error_t rc;

  if (header == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (header->base != NULL) {
    rc = ui_sort_header_base_destroy(header->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    header->base = NULL;
  }

  C_MULTIPLATFORM_FREE(header);
  return UI_ERROR_NONE;
}

ui_error_t md3_sort_header_cycle(struct md3_sort_header *header) {
  ui_error_t rc;

  if (header == NULL || header->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (header->direction == UI_SORT_NONE) {
    header->direction = UI_SORT_ASCENDING;
  } else if (header->direction == UI_SORT_ASCENDING) {
    header->direction = UI_SORT_DESCENDING;
  } else {
    header->direction = header->allow_none ? UI_SORT_NONE : UI_SORT_ASCENDING;
  }

  rc = ui_sort_header_base_toggle(header->base, (void *)header->column_id);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_sort_header_set_direction(struct md3_sort_header *header,
                                         enum ui_sort_direction direction) {
  if (header == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  header->direction = direction;
  return UI_ERROR_NONE;
}

ui_error_t
md3_sort_header_get_direction(const struct md3_sort_header *header,
                              enum ui_sort_direction *out_direction) {
  if (header == NULL || out_direction == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_direction = header->direction;
  return UI_ERROR_NONE;
}

ui_error_t md3_sort_header_set_cycle_mode(struct md3_sort_header *header,
                                          int allow_none) {
  if (header == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  header->allow_none = allow_none ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t
md3_sort_header_get_arrow_rotation(const struct md3_sort_header *header,
                                   float *out_degrees) {
  if (header == NULL || out_degrees == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (header->direction == UI_SORT_ASCENDING) {
    *out_degrees = 0.0f;
  } else if (header->direction == UI_SORT_DESCENDING) {
    *out_degrees = 180.0f;
  } else {
    *out_degrees = -1.0f;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_sort_header_set_priority(struct md3_sort_header *header,
                                        int priority) {
  if (header == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  header->priority = priority;
  return UI_ERROR_NONE;
}

ui_error_t md3_sort_header_get_priority(const struct md3_sort_header *header,
                                        int *out_priority) {
  if (header == NULL || out_priority == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_priority = header->priority;
  return UI_ERROR_NONE;
}

ui_error_t md3_sort_header_set_expressive(struct md3_sort_header *header,
                                          int enabled) {
  if (header == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  header->is_expressive = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_tree                                                                  */
/* ========================================================================= */

static size_t md3_default_tree_root_count(void *user_data) {
  if (user_data != NULL) {
    return 0;
  }
  return 0;
}

static void *md3_default_tree_root_node(size_t index, void *user_data) {
  if (index > 0 || user_data != NULL) {
    return NULL;
  }
  return NULL;
}

static void *md3_default_tree_parent(void *node, void *user_data) {
  if (node != NULL || user_data != NULL) {
    return NULL;
  }
  return NULL;
}

static size_t md3_default_tree_child_count(void *node, void *user_data) {
  if (node != NULL || user_data != NULL) {
    return 0;
  }
  return 0;
}

static void *md3_default_tree_child(void *node, size_t index, void *user_data) {
  if (node != NULL || index > 0 || user_data != NULL) {
    return NULL;
  }
  return NULL;
}

static ui_error_t md3_default_tree_render(void *node,
                                          struct ui_dom_node *dom_node,
                                          void *user_data) {
  if (node != NULL || dom_node != NULL || user_data != NULL) {
    return UI_ERROR_NONE;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_create(struct ui_engine *engine,
                           const struct ui_tree_model *model,
                           struct md3_tree **out_tree) {
  struct md3_tree *t;
  const struct ui_tree_model *effective_model = model;
  ui_error_t rc;

  if (engine == NULL || out_tree == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  t = (struct md3_tree *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_tree));
  if (t == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(t, 0, sizeof(struct md3_tree));

  if (model == NULL || !model->get_root_count || !model->get_root_node ||
      !model->get_parent || !model->get_child_count || !model->get_child ||
      !model->render_node) {
    s_default_tree_model.get_root_count = (model && model->get_root_count)
                                              ? model->get_root_count
                                              : md3_default_tree_root_count;
    s_default_tree_model.get_root_node = (model && model->get_root_node)
                                             ? model->get_root_node
                                             : md3_default_tree_root_node;
    s_default_tree_model.get_parent = (model && model->get_parent)
                                          ? model->get_parent
                                          : md3_default_tree_parent;
    s_default_tree_model.get_child_count = (model && model->get_child_count)
                                               ? model->get_child_count
                                               : md3_default_tree_child_count;
    s_default_tree_model.get_child =
        (model && model->get_child) ? model->get_child : md3_default_tree_child;
    s_default_tree_model.render_node = (model && model->render_node)
                                           ? model->render_node
                                           : md3_default_tree_render;
    s_default_tree_model.user_data = model ? model->user_data : NULL;
    effective_model = &s_default_tree_model;
  }

  rc = ui_tree_base_create(&t->base, effective_model);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(t);
    return rc;
  }

  t->model = effective_model;
  t->indent_px = 24.0f;
  t->guide_lines_enabled = 1;
  t->multi_select = 0;
  t->virtual_scroll_enabled = 0;
  t->is_expressive = 0;
  t->expanded_count = 0;
  t->selected_count = 0;
  t->checked_count = 0;

  *out_tree = t;
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_destroy(struct md3_tree *tree) {
  ui_error_t rc;

  if (tree == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tree->base != NULL) {
    rc = ui_tree_base_destroy(tree->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    tree->base = NULL;
  }

  C_MULTIPLATFORM_FREE(tree);
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_set_node_expanded(struct md3_tree *tree, void *node_id,
                                      int expanded) {
  size_t i;
  int found = 0;
  size_t found_idx = 0;

  if (tree == NULL || node_id == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < tree->expanded_count; ++i) {
    if (tree->expanded_nodes[i] == node_id) {
      found = 1;
      found_idx = i;
      break;
    }
  }

  if (expanded && !found) {
    if (tree->expanded_count < 128) {
      tree->expanded_nodes[tree->expanded_count++] = node_id;
    }
  } else if (!expanded && found) {
    for (i = found_idx; i + 1 < tree->expanded_count; ++i) {
      tree->expanded_nodes[i] = tree->expanded_nodes[i + 1];
    }
    tree->expanded_count--;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_is_node_expanded(const struct md3_tree *tree, void *node_id,
                                     int *out_expanded) {
  size_t i;

  if (tree == NULL || node_id == NULL || out_expanded == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_expanded = 0;
  for (i = 0; i < tree->expanded_count; ++i) {
    if (tree->expanded_nodes[i] == node_id) {
      *out_expanded = 1;
      break;
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_toggle_node_expanded(struct md3_tree *tree, void *node_id) {
  int is_exp = 0;
  ui_error_t rc;

  rc = md3_tree_is_node_expanded(tree, node_id, &is_exp);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return md3_tree_set_node_expanded(tree, node_id, is_exp ? 0 : 1);
}

ui_error_t md3_tree_set_indent(struct md3_tree *tree, float indent_px) {
  if (tree == NULL || indent_px < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tree->indent_px = indent_px;
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_get_indent(const struct md3_tree *tree,
                               float *out_indent_px) {
  if (tree == NULL || out_indent_px == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_indent_px = tree->indent_px;
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_set_guide_lines(struct md3_tree *tree, int enabled) {
  if (tree == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tree->guide_lines_enabled = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_select_node(struct md3_tree *tree, void *node_id,
                                int selected) {
  size_t i;
  int found = 0;
  size_t found_idx = 0;

  if (tree == NULL || node_id == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < tree->selected_count; ++i) {
    if (tree->selected_nodes[i] == node_id) {
      found = 1;
      found_idx = i;
      break;
    }
  }

  if (selected && !found) {
    if (!tree->multi_select) {
      tree->selected_count = 0;
    }
    if (tree->selected_count < 128) {
      tree->selected_nodes[tree->selected_count++] = node_id;
    }
  } else if (!selected && found) {
    for (i = found_idx; i + 1 < tree->selected_count; ++i) {
      tree->selected_nodes[i] = tree->selected_nodes[i + 1];
    }
    tree->selected_count--;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_is_node_selected(const struct md3_tree *tree, void *node_id,
                                     int *out_selected) {
  size_t i;

  if (tree == NULL || node_id == NULL || out_selected == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_selected = 0;
  for (i = 0; i < tree->selected_count; ++i) {
    if (tree->selected_nodes[i] == node_id) {
      *out_selected = 1;
      break;
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_set_multi_select(struct md3_tree *tree, int multi_select) {
  if (tree == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tree->multi_select = multi_select ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_set_node_checked_state(struct md3_tree *tree, void *node_id,
                                           int state) {
  size_t i;

  if (tree == NULL || node_id == NULL || state < 0 || state > 2) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < tree->checked_count; ++i) {
    if (tree->checked_nodes[i] == node_id) {
      tree->checked_states[i] = state;
      return UI_ERROR_NONE;
    }
  }

  if (tree->checked_count < 128) {
    tree->checked_nodes[tree->checked_count] = node_id;
    tree->checked_states[tree->checked_count] = state;
    tree->checked_count++;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_get_node_checked_state(const struct md3_tree *tree,
                                           void *node_id, int *out_state) {
  size_t i;

  if (tree == NULL || node_id == NULL || out_state == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_state = 0;
  for (i = 0; i < tree->checked_count; ++i) {
    if (tree->checked_nodes[i] == node_id) {
      *out_state = tree->checked_states[i];
      break;
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_set_virtual_scroll(struct md3_tree *tree, int enabled) {
  if (tree == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tree->virtual_scroll_enabled = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_set_expressive(struct md3_tree *tree, int enabled) {
  if (tree == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tree->is_expressive = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_tree_grid                                                             */
/* ========================================================================= */

ui_error_t md3_tree_grid_create(struct ui_engine *engine,
                                const struct ui_tree_model *model,
                                struct md3_tree_grid **out_tree_grid) {
  struct md3_tree_grid *tg;
  ui_error_t rc;

  if (engine == NULL || out_tree_grid == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tg = (struct md3_tree_grid *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_tree_grid));
  if (tg == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(tg, 0, sizeof(struct md3_tree_grid));

  rc = ui_datagrid_base_create(&tg->datagrid);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tg);
    return rc;
  }

  tg->model = model;
  tg->column_count = 1;
  tg->indent_px = 24.0f;
  tg->sticky_headers = 1;
  tg->zebra_shading = 0;
  tg->is_expressive = 0;
  tg->expanded_count = 0;
  tg->selected_count = 0;

  *out_tree_grid = tg;
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_grid_destroy(struct md3_tree_grid *tg) {
  ui_error_t rc;

  if (tg == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tg->datagrid != NULL) {
    rc = ui_datagrid_base_destroy(tg->datagrid);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    tg->datagrid = NULL;
  }

  C_MULTIPLATFORM_FREE(tg);
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_grid_set_column_count(struct md3_tree_grid *tg,
                                          size_t count) {
  if (tg == NULL || count == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tg->column_count = count;
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_grid_set_node_expanded(struct md3_tree_grid *tg,
                                           void *node_id, int expanded) {
  size_t i;
  int found = 0;
  size_t found_idx = 0;

  if (tg == NULL || node_id == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < tg->expanded_count; ++i) {
    if (tg->expanded_nodes[i] == node_id) {
      found = 1;
      found_idx = i;
      break;
    }
  }

  if (expanded && !found) {
    if (tg->expanded_count < 128) {
      tg->expanded_nodes[tg->expanded_count++] = node_id;
    }
  } else if (!expanded && found) {
    for (i = found_idx; i + 1 < tg->expanded_count; ++i) {
      tg->expanded_nodes[i] = tg->expanded_nodes[i + 1];
    }
    tg->expanded_count--;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_grid_is_node_expanded(const struct md3_tree_grid *tg,
                                          void *node_id, int *out_expanded) {
  size_t i;

  if (tg == NULL || node_id == NULL || out_expanded == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_expanded = 0;
  for (i = 0; i < tg->expanded_count; ++i) {
    if (tg->expanded_nodes[i] == node_id) {
      *out_expanded = 1;
      break;
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_grid_select_row(struct md3_tree_grid *tg, void *node_id,
                                    int selected) {
  size_t i;
  int found = 0;
  size_t found_idx = 0;

  if (tg == NULL || node_id == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < tg->selected_count; ++i) {
    if (tg->selected_rows[i] == node_id) {
      found = 1;
      found_idx = i;
      break;
    }
  }

  if (selected && !found) {
    if (tg->selected_count < 128) {
      tg->selected_rows[tg->selected_count++] = node_id;
    }
  } else if (!selected && found) {
    for (i = found_idx; i + 1 < tg->selected_count; ++i) {
      tg->selected_rows[i] = tg->selected_rows[i + 1];
    }
    tg->selected_count--;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_grid_is_row_selected(const struct md3_tree_grid *tg,
                                         void *node_id, int *out_selected) {
  size_t i;

  if (tg == NULL || node_id == NULL || out_selected == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_selected = 0;
  for (i = 0; i < tg->selected_count; ++i) {
    if (tg->selected_rows[i] == node_id) {
      *out_selected = 1;
      break;
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_grid_set_indent(struct md3_tree_grid *tg, float indent_px) {
  if (tg == NULL || indent_px < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tg->indent_px = indent_px;
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_grid_set_sticky_headers(struct md3_tree_grid *tg,
                                            int sticky) {
  if (tg == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tg->sticky_headers = sticky ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_grid_set_zebra_shading(struct md3_tree_grid *tg,
                                           int enabled) {
  if (tg == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tg->zebra_shading = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_tree_grid_set_expressive(struct md3_tree_grid *tg, int enabled) {
  if (tg == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tg->is_expressive = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_transfer_list                                                         */
/* ========================================================================= */

ui_error_t
md3_transfer_list_create(struct ui_engine *engine,
                         struct md3_transfer_list **out_transfer_list) {
  struct md3_transfer_list *tl;
  ui_error_t rc;

  if (engine == NULL || out_transfer_list == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tl = (struct md3_transfer_list *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_transfer_list));
  if (tl == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(tl, 0, sizeof(struct md3_transfer_list));

  rc = ui_transfer_list_base_init(&tl->base, &tl->comp, NULL);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tl);
    return rc;
  }

  tl->left_head = NULL;
  tl->right_head = NULL;
  tl->left_filter[0] = '\0';
  tl->right_filter[0] = '\0';
  tl->is_expressive = 0;

  *out_transfer_list = tl;
  return UI_ERROR_NONE;
}

ui_error_t md3_transfer_list_destroy(struct md3_transfer_list *tl) {
  struct md3_transfer_item *curr;
  struct md3_transfer_item *next;

  if (tl == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = tl->left_head;
  while (curr != NULL) {
    next = curr->next;
    C_MULTIPLATFORM_FREE(curr);
    curr = next;
  }

  curr = tl->right_head;
  while (curr != NULL) {
    next = curr->next;
    C_MULTIPLATFORM_FREE(curr);
    curr = next;
  }

  C_MULTIPLATFORM_FREE(tl);
  return UI_ERROR_NONE;
}

ui_error_t md3_transfer_list_add_item(struct md3_transfer_list *tl,
                                      int to_right, int id, const char *label,
                                      void *data) {
  struct md3_transfer_item *item;
  struct md3_transfer_item **head;

  if (tl == NULL || label == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item = (struct md3_transfer_item *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_transfer_item));
  if (item == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(item, 0, sizeof(struct md3_transfer_item));

  item->id = id;
  item->data = data;
  item->selected = 0;

#if defined(_MSC_VER)
  strncpy_s(item->label, sizeof(item->label), label, sizeof(item->label) - 1);
#else
  strncpy(item->label, label, sizeof(item->label) - 1);
  item->label[sizeof(item->label) - 1] = '\0';
#endif

  head = to_right ? &tl->right_head : &tl->left_head;
  item->next = *head;
  *head = item;

  return UI_ERROR_NONE;
}

ui_error_t md3_transfer_list_set_item_selected(struct md3_transfer_list *tl,
                                               int is_right, int id,
                                               int selected) {
  struct md3_transfer_item *curr;

  if (tl == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = is_right ? tl->right_head : tl->left_head;
  while (curr != NULL) {
    if (curr->id == id) {
      curr->selected = selected ? 1 : 0;
      return UI_ERROR_NONE;
    }
    curr = curr->next;
  }

  return UI_ERROR_NOT_FOUND;
}

ui_error_t md3_transfer_list_move_selected_right(struct md3_transfer_list *tl) {
  struct md3_transfer_item *curr;
  struct md3_transfer_item *prev = NULL;
  struct md3_transfer_item *next;

  if (tl == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = tl->left_head;
  while (curr != NULL) {
    next = curr->next;
    if (curr->selected) {
      if (prev == NULL) {
        tl->left_head = next;
      } else {
        prev->next = next;
      }
      curr->selected = 0;
      curr->next = tl->right_head;
      tl->right_head = curr;
    } else {
      prev = curr;
    }
    curr = next;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_transfer_list_move_all_right(struct md3_transfer_list *tl) {
  struct md3_transfer_item *curr;
  struct md3_transfer_item *next;

  if (tl == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = tl->left_head;
  while (curr != NULL) {
    next = curr->next;
    curr->selected = 0;
    curr->next = tl->right_head;
    tl->right_head = curr;
    curr = next;
  }
  tl->left_head = NULL;
  return UI_ERROR_NONE;
}

ui_error_t md3_transfer_list_move_selected_left(struct md3_transfer_list *tl) {
  struct md3_transfer_item *curr;
  struct md3_transfer_item *prev = NULL;
  struct md3_transfer_item *next;

  if (tl == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = tl->right_head;
  while (curr != NULL) {
    next = curr->next;
    if (curr->selected) {
      if (prev == NULL) {
        tl->right_head = next;
      } else {
        prev->next = next;
      }
      curr->selected = 0;
      curr->next = tl->left_head;
      tl->left_head = curr;
    } else {
      prev = curr;
    }
    curr = next;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_transfer_list_move_all_left(struct md3_transfer_list *tl) {
  struct md3_transfer_item *curr;
  struct md3_transfer_item *next;

  if (tl == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = tl->right_head;
  while (curr != NULL) {
    next = curr->next;
    curr->selected = 0;
    curr->next = tl->left_head;
    tl->left_head = curr;
    curr = next;
  }
  tl->right_head = NULL;
  return UI_ERROR_NONE;
}

ui_error_t md3_transfer_list_set_left_filter(struct md3_transfer_list *tl,
                                             const char *query) {
  if (tl == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (query == NULL) {
    tl->left_filter[0] = '\0';
  } else {
#if defined(_MSC_VER)
    strncpy_s(tl->left_filter, sizeof(tl->left_filter), query,
              sizeof(tl->left_filter) - 1);
#else
    strncpy(tl->left_filter, query, sizeof(tl->left_filter) - 1);
    tl->left_filter[sizeof(tl->left_filter) - 1] = '\0';
#endif
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_transfer_list_set_right_filter(struct md3_transfer_list *tl,
                                              const char *query) {
  if (tl == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (query == NULL) {
    tl->right_filter[0] = '\0';
  } else {
#if defined(_MSC_VER)
    strncpy_s(tl->right_filter, sizeof(tl->right_filter), query,
              sizeof(tl->right_filter) - 1);
#else
    strncpy(tl->right_filter, query, sizeof(tl->right_filter) - 1);
    tl->right_filter[sizeof(tl->right_filter) - 1] = '\0';
#endif
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_transfer_list_get_left_counts(const struct md3_transfer_list *tl,
                                             size_t *out_selected,
                                             size_t *out_total) {
  const struct md3_transfer_item *curr;
  size_t selected = 0;
  size_t total = 0;

  if (tl == NULL || out_selected == NULL || out_total == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = tl->left_head;
  while (curr != NULL) {
    total++;
    if (curr->selected) {
      selected++;
    }
    curr = curr->next;
  }

  *out_selected = selected;
  *out_total = total;
  return UI_ERROR_NONE;
}

ui_error_t
md3_transfer_list_get_right_counts(const struct md3_transfer_list *tl,
                                   size_t *out_selected, size_t *out_total) {
  const struct md3_transfer_item *curr;
  size_t selected = 0;
  size_t total = 0;

  if (tl == NULL || out_selected == NULL || out_total == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = tl->right_head;
  while (curr != NULL) {
    total++;
    if (curr->selected) {
      selected++;
    }
    curr = curr->next;
  }

  *out_selected = selected;
  *out_total = total;
  return UI_ERROR_NONE;
}

ui_error_t md3_transfer_list_set_expressive(struct md3_transfer_list *tl,
                                            int enabled) {
  if (tl == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tl->is_expressive = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}
