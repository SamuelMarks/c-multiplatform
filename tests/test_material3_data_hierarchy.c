/**
 * @file test_material3_data_hierarchy.c
 * @brief Unit tests for Material 3 Data Tables & Hierarchical Presentation.
 */

/* clang-format off */
#include "material3/md3_data_hierarchy.h"
#include "ui_engine.h"
#include "ui_test_mock_mem.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_md3_data_hierarchy_mock_fail;
extern int g_malloc_fail_countdown;
extern struct ui_table_model g_default_table_model;
extern struct ui_tree_model g_default_tree_model;
#endif

static size_t mock_get_row_count(void *user_data) {
  if (user_data != NULL) {
    return 10;
  }
  return 10;
}

static size_t mock_get_col_count(void *user_data) {
  if (user_data != NULL) {
    return 3;
  }
  return 3;
}

static size_t mock_tree_root_count(void *user_data) {
  if (user_data != NULL) {
    return 2;
  }
  return 2;
}

TEST test_md3_table_lifecycle_and_features(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_table *table = NULL;
  struct ui_table_model model;
  float height = 0.0f;
  int selected = 0;
  int all_sel = 0;
  int indet = 0;
  int expanded = 0;
  ui_error_t rc;

  /* Null checks */
  rc = md3_table_create(NULL, &table);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create table */
  rc = md3_table_create(dummy_engine, &table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(table != NULL);

  /* Density */
  rc = md3_table_set_density(NULL, MD3_TABLE_DENSITY_DENSE);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_get_row_height(NULL, &height);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_get_row_height(table, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_table_set_density(table, MD3_TABLE_DENSITY_DENSE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_get_row_height(table, &height);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(40.0f, height);

  rc = md3_table_set_density(table, MD3_TABLE_DENSITY_RELAXED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_get_row_height(table, &height);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(64.0f, height);

  rc = md3_table_set_density(table, MD3_TABLE_DENSITY_NORMAL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_get_row_height(table, &height);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(52.0f, height);

  /* Model */
  memset(&model, 0, sizeof(model));
  model.get_row_count = mock_get_row_count;
  model.get_column_count = mock_get_col_count;
  rc = md3_table_set_model(NULL, &model);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_model(table, &model);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10, (int)table->total_rows);

  rc = md3_table_set_model(table, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, (int)table->total_rows);
  rc = md3_table_set_model(table, &model);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Sticky headers & columns */
  rc = md3_table_set_sticky_header(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_sticky_header(table, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_set_sticky_first_column(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_sticky_first_column(table, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_set_sticky_last_column(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_sticky_last_column(table, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Selectable and Row Selection */
  rc = md3_table_set_selectable(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_selectable(table, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_table_select_row(NULL, 0, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_is_row_selected(NULL, 0, &selected);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_is_row_selected(table, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_table_select_row(table, 2, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_is_row_selected(table, 2, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, selected);

  rc = md3_table_is_row_selected(table, 1, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, selected);

  /* Selection state check */
  rc = md3_table_get_selection_state(NULL, &all_sel, &indet);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_get_selection_state(table, &all_sel, &indet);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, all_sel);
  ASSERT_EQ(1, indet);

  rc = md3_table_select_all(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_select_all(table, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_get_selection_state(table, &all_sel, &indet);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, all_sel);
  ASSERT_EQ(0, indet);

  rc = md3_table_select_all(table, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_get_selection_state(table, &all_sel, &indet);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, all_sel);
  ASSERT_EQ(0, indet);

  /* Hover, Virtual scroll, Horizontal scroll, Glassmorphism */
  rc = md3_table_set_hovered_row(NULL, 3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_hovered_row(table, 3);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_table_set_virtual_scroll(NULL, 1, 500);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_virtual_scroll(table, 1, 500);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_table_set_horizontal_scroll(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_horizontal_scroll(table, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_table_set_glassmorphism(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_glassmorphism(table, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Expandable detail row */
  rc = md3_table_toggle_row_expansion(NULL, 4);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_is_row_expanded(NULL, 4, &expanded);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_is_row_expanded(table, 4, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_table_toggle_row_expansion(table, 4);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_is_row_expanded(table, 4, &expanded);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, expanded);

  rc = md3_table_toggle_row_expansion(table, 4);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_is_row_expanded(table, 4, &expanded);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, expanded);

  /* Resize column */
  rc = md3_table_resize_column(NULL, 0, 120.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_resize_column(table, 0, 120.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy */
  rc = md3_table_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_paginator_lifecycle_and_navigation(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_paginator *paginator = NULL;
  size_t page = 0;
  size_t total_pages = 0;
  size_t page_size = 0;
  size_t options[3] = {5, 10, 20};
  char label[64];
  ui_error_t rc;

  /* Null checks */
  rc = md3_paginator_create(NULL, &paginator);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_paginator_create(dummy_engine, &paginator);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(paginator != NULL);

  /* Config */
  rc = md3_paginator_set_config(NULL, 100, 10);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_set_config(paginator, 100, 10);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_paginator_get_total_pages(NULL, &total_pages);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_get_total_pages(paginator, &total_pages);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10, (int)total_pages);

  /* Page navigation */
  rc = md3_paginator_set_current_page(NULL, 2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_set_current_page(paginator, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_paginator_get_current_page(NULL, &page);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_get_current_page(paginator, &page);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, (int)page);

  rc = md3_paginator_next(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_next(paginator);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, (int)paginator->current_page);

  rc = md3_paginator_prev(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_prev(paginator);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, (int)paginator->current_page);

  rc = md3_paginator_first(paginator);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, (int)paginator->current_page);

  rc = md3_paginator_last(paginator);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(9, (int)paginator->current_page);

  /* Page size & options */
  rc = md3_paginator_set_page_size_options(NULL, options, 3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_set_page_size_options(paginator, options, 3);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_paginator_get_page_size(NULL, &page_size);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_get_page_size(paginator, &page_size);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10, (int)page_size);

  rc = md3_paginator_set_page_size(NULL, 25);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_set_page_size(paginator, 25);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(25, (int)paginator->page_size);

  /* Range label */
  rc = md3_paginator_set_current_page(paginator, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_paginator_get_range_label(NULL, label, sizeof(label));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_get_range_label(paginator, label, sizeof(label));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(strlen(label) > 0);

  /* Compact and Expressive */
  rc = md3_paginator_set_compact(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_set_compact(paginator, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, paginator->is_compact);

  rc = md3_paginator_set_expressive(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_set_expressive(paginator, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, paginator->is_expressive);

  /* Clean up */
  rc = md3_paginator_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_destroy(paginator);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_sort_header_lifecycle_and_cycling(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_sort_header *header = NULL;
  enum ui_sort_direction dir;
  float degrees = 0.0f;
  int priority = 0;
  ui_error_t rc;

  /* Null checks */
  rc = md3_sort_header_create(NULL, "name", &header);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_create(dummy_engine, NULL, &header);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_create(dummy_engine, "name", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_sort_header_create(dummy_engine, "name", &header);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(header != NULL);
  ASSERT_STR_EQ("name", header->column_id);

  /* Cycle: None -> Ascending -> Descending -> None */
  rc = md3_sort_header_get_direction(header, &dir);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_SORT_NONE, dir);

  rc = md3_sort_header_cycle(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_cycle(header);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_SORT_ASCENDING, header->direction);

  rc = md3_sort_header_get_arrow_rotation(header, &degrees);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, degrees);

  rc = md3_sort_header_cycle(header);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_SORT_DESCENDING, header->direction);

  rc = md3_sort_header_get_arrow_rotation(header, &degrees);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(180.0f, degrees);

  rc = md3_sort_header_cycle(header);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_SORT_NONE, header->direction);

  rc = md3_sort_header_get_arrow_rotation(header, &degrees);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(-1.0f, degrees);

  /* Bi-state cycle mode */
  rc = md3_sort_header_set_cycle_mode(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_set_cycle_mode(header, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_sort_header_set_direction(NULL, UI_SORT_DESCENDING);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_set_direction(header, UI_SORT_DESCENDING);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_sort_header_cycle(header);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_SORT_ASCENDING, header->direction);

  /* Multi-column priority */
  rc = md3_sort_header_set_priority(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_set_priority(header, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_sort_header_get_priority(NULL, &priority);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_get_priority(header, &priority);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, priority);

  /* Expressive */
  rc = md3_sort_header_set_expressive(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_set_expressive(header, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, header->is_expressive);

  /* Clean up */
  rc = md3_sort_header_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_destroy(header);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_tree_lifecycle_and_features(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_tree *tree = NULL;
  struct ui_tree_model model;
  void *node1 = (void *)0x100;
  void *node2 = (void *)0x200;
  int expanded = 0;
  int selected = 0;
  int chk_state = 0;
  float indent = 0.0f;
  ui_error_t rc;

  memset(&model, 0, sizeof(model));
  model.get_root_count = mock_tree_root_count;

  /* Null checks */
  rc = md3_tree_create(NULL, &model, &tree);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_create(dummy_engine, &model, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_tree_create(dummy_engine, &model, &tree);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tree != NULL);

  /* Expansion */
  rc = md3_tree_set_node_expanded(NULL, node1, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_set_node_expanded(tree, NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_tree_set_node_expanded(tree, node1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_is_node_expanded(tree, node1, &expanded);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, expanded);

  rc = md3_tree_toggle_node_expanded(NULL, node1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_toggle_node_expanded(tree, node1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_is_node_expanded(tree, node1, &expanded);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, expanded);

  /* Indentation & guide lines */
  rc = md3_tree_set_indent(NULL, 16.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_set_indent(tree, -5.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_set_indent(tree, 32.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_get_indent(tree, &indent);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(32.0f, indent);

  rc = md3_tree_set_guide_lines(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_set_guide_lines(tree, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, tree->guide_lines_enabled);

  /* Selection: single & multi */
  rc = md3_tree_select_node(NULL, node1, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_select_node(tree, node1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_is_node_selected(tree, node1, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, selected);

  rc = md3_tree_select_node(tree, node2, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_is_node_selected(tree, node1, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, selected); /* Single select cleared node1 */

  rc = md3_tree_set_multi_select(tree, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_select_node(tree, node1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_is_node_selected(tree, node1, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, selected);
  rc = md3_tree_is_node_selected(tree, node2, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, selected);

  /* Tri-state checkboxes */
  rc = md3_tree_set_node_checked_state(NULL, node1, 2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_set_node_checked_state(tree, node1, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_get_node_checked_state(tree, node1, &chk_state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, chk_state);

  /* Virtual scroll & Expressive */
  rc = md3_tree_set_virtual_scroll(tree, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_set_expressive(tree, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Clean up */
  rc = md3_tree_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_destroy(tree);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_tree_grid_lifecycle_and_features(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_tree_grid *tg = NULL;
  struct ui_tree_model model;
  void *node1 = (void *)0x100;
  int expanded = 0;
  int selected = 0;
  ui_error_t rc;

  memset(&model, 0, sizeof(model));
  model.get_root_count = mock_tree_root_count;

  /* Null checks */
  rc = md3_tree_grid_create(NULL, &model, &tg);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_create(dummy_engine, &model, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_tree_grid_create(dummy_engine, &model, &tg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tg != NULL);

  /* Column count */
  rc = md3_tree_grid_set_column_count(NULL, 4);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_set_column_count(tg, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_set_column_count(tg, 4);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, (int)tg->column_count);

  /* Expansion & selection */
  rc = md3_tree_grid_set_node_expanded(tg, node1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_grid_is_node_expanded(tg, node1, &expanded);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, expanded);

  rc = md3_tree_grid_select_row(tg, node1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_grid_is_row_selected(tg, node1, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, selected);

  /* Indent, sticky headers, zebra shading, expressive */
  rc = md3_tree_grid_set_indent(tg, 20.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_grid_set_sticky_headers(tg, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_grid_set_zebra_shading(tg, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, tg->zebra_shading);
  rc = md3_tree_grid_set_expressive(tg, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, tg->is_expressive);

  /* Clean up */
  rc = md3_tree_grid_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_destroy(tg);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_transfer_list_lifecycle_and_moves(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_transfer_list *tl = NULL;
  size_t sel_cnt = 0;
  size_t tot_cnt = 0;
  ui_error_t rc;

  /* Null checks */
  rc = md3_transfer_list_create(NULL, &tl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_transfer_list_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_transfer_list_create(dummy_engine, &tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tl != NULL);

  /* Add items */
  rc = md3_transfer_list_add_item(NULL, 0, 1, "Item 1", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_transfer_list_add_item(tl, 0, 1, "Item 1", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_add_item(tl, 0, 2, "Item 2", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_add_item(tl, 1, 3, "Item 3", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Counts */
  rc = md3_transfer_list_get_left_counts(tl, &sel_cnt, &tot_cnt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, (int)sel_cnt);
  ASSERT_EQ(2, (int)tot_cnt);

  rc = md3_transfer_list_get_right_counts(tl, &sel_cnt, &tot_cnt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, (int)sel_cnt);
  ASSERT_EQ(1, (int)tot_cnt);

  /* Select and move right */
  rc = md3_transfer_list_set_item_selected(tl, 0, 1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_get_left_counts(tl, &sel_cnt, &tot_cnt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, (int)sel_cnt);

  rc = md3_transfer_list_move_selected_right(tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_get_left_counts(tl, &sel_cnt, &tot_cnt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, (int)tot_cnt);
  rc = md3_transfer_list_get_right_counts(tl, &sel_cnt, &tot_cnt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, (int)tot_cnt);

  /* Move all right */
  rc = md3_transfer_list_move_all_right(tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_get_left_counts(tl, &sel_cnt, &tot_cnt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, (int)tot_cnt);
  rc = md3_transfer_list_get_right_counts(tl, &sel_cnt, &tot_cnt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, (int)tot_cnt);

  /* Move all left */
  rc = md3_transfer_list_move_all_left(tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_get_left_counts(tl, &sel_cnt, &tot_cnt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, (int)tot_cnt);
  rc = md3_transfer_list_get_right_counts(tl, &sel_cnt, &tot_cnt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, (int)tot_cnt);

  /* Move selected left */
  rc = md3_transfer_list_move_all_right(tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_set_item_selected(tl, 1, 2, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_move_selected_left(tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_get_left_counts(tl, &sel_cnt, &tot_cnt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, (int)tot_cnt);

  /* Filters */
  rc = md3_transfer_list_set_left_filter(tl, "Item");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_set_right_filter(tl, "Item");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Expressive */
  rc = md3_transfer_list_set_expressive(tl, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, tl->is_expressive);

  /* Clean up */
  rc = md3_transfer_list_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_transfer_list_destroy(tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_data_hierarchy_branches(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_table *table = NULL;
  struct md3_paginator *paginator = NULL;
  struct md3_sort_header *header = NULL;
  struct md3_tree *tree = NULL;
  struct md3_tree_grid *tg = NULL;
  struct md3_transfer_list *tl = NULL;
  struct ui_dom_node dummy_node;
  char hex_buf[64];
  float f_val = 0.0f;
  size_t s_val = 0;
  size_t s_val2 = 0;
  int i_val = 0;
  int i_val2 = 0;
  enum ui_sort_direction dir;
  size_t i;
  ui_error_t rc;

  memset(&dummy_node, 0, sizeof(dummy_node));

  /* 1. Table */
  rc = md3_table_create(dummy_engine, &table);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  /* Invoke default table model callbacks */
  g_default_table_model.get_row_count(NULL);
  g_default_table_model.get_row_count((void *)1);
  g_default_table_model.get_column_count(NULL);
  g_default_table_model.get_column_count((void *)1);
  g_default_table_model.render_cell(0, 0, NULL, NULL);
  g_default_table_model.render_cell(1, 0, NULL, NULL);
  g_default_table_model.render_cell(0, 1, NULL, NULL);
  g_default_table_model.render_cell(0, 0, &dummy_node, NULL);
  g_default_table_model.render_cell(0, 0, NULL, (void *)1);
  g_default_table_model.render_header(0, NULL, NULL);
  g_default_table_model.render_header(1, NULL, NULL);
  g_default_table_model.render_header(0, &dummy_node, NULL);
  g_default_table_model.render_header(0, NULL, (void *)1);
#endif

  rc = md3_table_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  table->table_base = NULL;
  rc = md3_table_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_table_create(dummy_engine, &table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  table->datagrid_base = NULL;
  rc = md3_table_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_table_create(dummy_engine, &table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_set_density(NULL, MD3_TABLE_DENSITY_NORMAL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_density(table, (enum md3_table_density) - 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_get_row_height(table, &f_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(52.0f, f_val);

  rc = md3_table_set_density(table, MD3_TABLE_DENSITY_DENSE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_get_row_height(table, &f_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(40.0f, f_val);

  rc = md3_table_set_density(table, MD3_TABLE_DENSITY_NORMAL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_get_row_height(table, &f_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(52.0f, f_val);

  rc = md3_table_set_density(table, MD3_TABLE_DENSITY_RELAXED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_get_row_height(table, &f_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(64.0f, f_val);
  rc = md3_table_get_row_height(NULL, &f_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_get_row_height(table, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_model(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  {
    struct ui_table_model null_rc_model;
    memset(&null_rc_model, 0, sizeof(null_rc_model));
    rc = md3_table_set_model(table, &null_rc_model);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_table_set_sticky_header(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_sticky_first_column(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_sticky_last_column(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_selectable(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_glassmorphism(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_virtual_scroll(NULL, 1, 400);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_horizontal_scroll(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_select_row(NULL, 0, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_select_all(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_is_row_selected(NULL, 0, &i_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_is_row_selected(table, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_get_selection_state(NULL, &i_val, &i_val2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_get_selection_state(table, NULL, &i_val2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_get_selection_state(table, &i_val, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_toggle_row_expansion(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_is_row_expanded(NULL, 0, &i_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_is_row_expanded(table, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_set_hovered_row(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_table_resize_column(NULL, 0, 100);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  {
    struct ui_datagrid_base *saved_grid = table->datagrid_base;
    table->datagrid_base = NULL;
    rc = md3_table_resize_column(table, 0, 100);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    table->datagrid_base = saved_grid;
  }

  /* Table selection operations */
  rc = md3_table_select_row(table, 1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_select_row(table, 2, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_select_row(table, 1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_select_row(table, 1, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_select_row(table, 1, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 64; ++i) {
    rc = md3_table_select_row(table, i, 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_table_select_row(table, 999, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Table select all with total_rows < 64 and >= 64 */
  table->total_rows = 10;
  rc = md3_table_select_all(table, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  table->total_rows = 100;
  rc = md3_table_select_all(table, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Table get_selection_state branches */
  table->total_rows = 0;
  rc = md3_table_get_selection_state(table, &i_val, &i_val2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, i_val);
  ASSERT_EQ(0, i_val2);

  table->total_rows = 10;
  table->selected_count = 10;
  rc = md3_table_get_selection_state(table, &i_val, &i_val2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, i_val);
  ASSERT_EQ(0, i_val2);

  table->selected_count = 5;
  rc = md3_table_get_selection_state(table, &i_val, &i_val2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, i_val);
  ASSERT_EQ(1, i_val2);

  /* Table expansion operations */
  table->expanded_count = 0;
  rc = md3_table_toggle_row_expansion(table, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_is_row_expanded(table, 5, &i_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, i_val);
  rc = md3_table_is_row_expanded(table, 999, &i_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, i_val);
  rc = md3_table_toggle_row_expansion(table, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  table->expanded_count = 0;
  rc = md3_table_toggle_row_expansion(table, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_toggle_row_expansion(table, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_table_toggle_row_expansion(table, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 64; ++i) {
    rc = md3_table_toggle_row_expansion(table, i);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  table->expanded_count = 64;
  rc = md3_table_toggle_row_expansion(table, 999);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_table_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 2. Paginator */
  rc = md3_paginator_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_create(dummy_engine, &paginator);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  paginator->base = NULL;
  rc = md3_paginator_destroy(paginator);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_paginator_create(dummy_engine, &paginator);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_paginator_set_config(NULL, 100, 10);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_set_config(paginator, 100, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_set_current_page(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_get_current_page(NULL, &s_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_get_current_page(paginator, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_get_total_pages(NULL, &s_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_get_total_pages(paginator, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_next(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_prev(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_first(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_last(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_set_page_size(NULL, 10);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_set_page_size(paginator, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_get_page_size(NULL, &s_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_get_page_size(paginator, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_get_range_label(NULL, hex_buf, sizeof(hex_buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_get_range_label(paginator, NULL, sizeof(hex_buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_get_range_label(paginator, hex_buf, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  paginator->total_items = 0;
  rc = md3_paginator_get_range_label(paginator, hex_buf, sizeof(hex_buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  paginator->total_items = 25;
  paginator->page_size = 10;
  paginator->current_page = 0;
  rc = md3_paginator_get_range_label(paginator, hex_buf, sizeof(hex_buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  paginator->current_page = 2;
  rc = md3_paginator_get_range_label(paginator, hex_buf, sizeof(hex_buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  {
    size_t opts[10] = {5, 10, 20, 25, 50, 100, 200, 500, 1000, 2000};
    rc = md3_paginator_set_page_size_options(NULL, opts, 3);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_paginator_set_page_size_options(paginator, NULL, 3);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_paginator_set_page_size_options(paginator, opts, 0);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_paginator_set_page_size_options(paginator, opts, 9);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_paginator_set_page_size_options(paginator, opts, 3);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_paginator_set_compact(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_paginator_set_expressive(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Paginator NULL base checks */
  {
    struct ui_pagination_base *saved_pbase = paginator->base;
    paginator->base = NULL;
    rc = md3_paginator_set_config(paginator, 100, 10);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_paginator_set_current_page(paginator, 0);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_paginator_get_current_page(paginator, &s_val);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_paginator_get_total_pages(paginator, &s_val);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_paginator_next(paginator);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_paginator_prev(paginator);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_paginator_first(paginator);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_paginator_last(paginator);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_paginator_set_page_size(paginator, 10);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    paginator->base = saved_pbase;
  }

  rc = md3_paginator_destroy(paginator);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 3. Sort Header */
  rc = md3_sort_header_create(dummy_engine, NULL, &header);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_create(dummy_engine, "col", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_sort_header_create(dummy_engine, "col", &header);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  header->base = NULL;
  rc = md3_sort_header_destroy(header);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_sort_header_create(dummy_engine, "col", &header);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_sort_header_cycle(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_set_direction(NULL, UI_SORT_ASCENDING);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_set_direction(header, UI_SORT_ASCENDING);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_sort_header_get_arrow_rotation(header, &f_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, f_val);
  rc = md3_sort_header_set_direction(header, UI_SORT_DESCENDING);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_sort_header_get_arrow_rotation(header, &f_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(180.0f, f_val);
  rc = md3_sort_header_set_direction(header, UI_SORT_NONE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_sort_header_get_arrow_rotation(header, &f_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(-1.0f, f_val);
  rc = md3_sort_header_get_direction(NULL, &dir);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_get_direction(header, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_set_cycle_mode(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_set_priority(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_get_priority(NULL, &i_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_get_priority(header, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_get_arrow_rotation(NULL, &f_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_get_arrow_rotation(header, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  {
    struct ui_sort_header_base *saved_hbase = header->base;
    header->base = NULL;
    rc = md3_sort_header_cycle(header);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    header->base = saved_hbase;
  }

  rc = md3_sort_header_set_expressive(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_sort_header_destroy(header);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 4. Tree with NULL model */
  rc = md3_tree_create(dummy_engine, NULL, &tree);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tree != NULL);

#ifdef UI_TEST_MOCK_ALLOC
  /* Invoke default tree callbacks */
  g_default_tree_model.get_root_count(NULL);
  g_default_tree_model.get_root_count((void *)1);
  g_default_tree_model.get_root_node(0, NULL);
  g_default_tree_model.get_root_node(1, NULL);
  g_default_tree_model.get_root_node(0, (void *)1);
  g_default_tree_model.get_parent(NULL, NULL);
  g_default_tree_model.get_parent((void *)1, NULL);
  g_default_tree_model.get_parent(NULL, (void *)1);
  g_default_tree_model.get_child_count(NULL, NULL);
  g_default_tree_model.get_child_count((void *)1, NULL);
  g_default_tree_model.get_child_count(NULL, (void *)1);
  g_default_tree_model.get_child(NULL, 0, NULL);
  g_default_tree_model.get_child((void *)1, 0, NULL);
  g_default_tree_model.get_child(NULL, 1, NULL);
  g_default_tree_model.get_child(NULL, 0, (void *)1);
  g_default_tree_model.render_node(NULL, NULL, NULL);
  g_default_tree_model.render_node((void *)1, NULL, NULL);
  g_default_tree_model.render_node(NULL, &dummy_node, NULL);
  g_default_tree_model.render_node(NULL, NULL, (void *)1);
#endif

  rc = md3_tree_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  tree->base = NULL;
  rc = md3_tree_destroy(tree);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Partial models */
  {
    struct ui_tree_model part_model;
    struct md3_tree *ptree = NULL;

    /* 1. Missing get_root_count */
    memset(&part_model, 0, sizeof(part_model));
    part_model.get_root_node = (void *(*)(size_t, void *))0x1;
    part_model.get_parent = (void *(*)(void *, void *))0x1;
    part_model.get_child_count = (size_t(*)(void *, void *))0x1;
    part_model.get_child = (void *(*)(void *, size_t, void *))0x1;
    part_model.render_node =
        (ui_error_t(*)(void *, struct ui_dom_node *, void *))0x1;
    rc = md3_tree_create(dummy_engine, &part_model, &ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_tree_destroy(ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* 2. Missing get_root_node */
    memset(&part_model, 0, sizeof(part_model));
    part_model.get_root_count = mock_tree_root_count;
    part_model.get_parent = (void *(*)(void *, void *))0x1;
    part_model.get_child_count = (size_t(*)(void *, void *))0x1;
    part_model.get_child = (void *(*)(void *, size_t, void *))0x1;
    part_model.render_node =
        (ui_error_t(*)(void *, struct ui_dom_node *, void *))0x1;
    rc = md3_tree_create(dummy_engine, &part_model, &ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_tree_destroy(ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* 3. Missing get_parent */
    memset(&part_model, 0, sizeof(part_model));
    part_model.get_root_count = mock_tree_root_count;
    part_model.get_root_node = (void *(*)(size_t, void *))0x1;
    part_model.get_child_count = (size_t(*)(void *, void *))0x1;
    part_model.get_child = (void *(*)(void *, size_t, void *))0x1;
    part_model.render_node =
        (ui_error_t(*)(void *, struct ui_dom_node *, void *))0x1;
    rc = md3_tree_create(dummy_engine, &part_model, &ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_tree_destroy(ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* 4. Missing get_child_count */
    memset(&part_model, 0, sizeof(part_model));
    part_model.get_root_count = mock_tree_root_count;
    part_model.get_root_node = (void *(*)(size_t, void *))0x1;
    part_model.get_parent = (void *(*)(void *, void *))0x1;
    part_model.get_child = (void *(*)(void *, size_t, void *))0x1;
    part_model.render_node =
        (ui_error_t(*)(void *, struct ui_dom_node *, void *))0x1;
    rc = md3_tree_create(dummy_engine, &part_model, &ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_tree_destroy(ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* 5. Missing get_child */
    memset(&part_model, 0, sizeof(part_model));
    part_model.get_root_count = mock_tree_root_count;
    part_model.get_root_node = (void *(*)(size_t, void *))0x1;
    part_model.get_parent = (void *(*)(void *, void *))0x1;
    part_model.get_child_count = (size_t(*)(void *, void *))0x1;
    part_model.render_node =
        (ui_error_t(*)(void *, struct ui_dom_node *, void *))0x1;
    rc = md3_tree_create(dummy_engine, &part_model, &ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_tree_destroy(ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* 6. Missing render_node */
    memset(&part_model, 0, sizeof(part_model));
    part_model.get_root_count = mock_tree_root_count;
    part_model.get_root_node = (void *(*)(size_t, void *))0x1;
    part_model.get_parent = (void *(*)(void *, void *))0x1;
    part_model.get_child_count = (size_t(*)(void *, void *))0x1;
    part_model.get_child = (void *(*)(void *, size_t, void *))0x1;
    rc = md3_tree_create(dummy_engine, &part_model, &ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_tree_destroy(ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* 7. All fields non-null with NULL user_data */
    memset(&part_model, 0, sizeof(part_model));
    part_model.get_root_count = mock_tree_root_count;
    part_model.get_root_node = (void *(*)(size_t, void *))0x1;
    part_model.get_parent = (void *(*)(void *, void *))0x1;
    part_model.get_child_count = (size_t(*)(void *, void *))0x1;
    part_model.get_child = (void *(*)(void *, size_t, void *))0x1;
    part_model.render_node =
        (ui_error_t(*)(void *, struct ui_dom_node *, void *))0x1;
    rc = md3_tree_create(dummy_engine, &part_model, &ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_tree_destroy(ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* 8. All fields non-null with non-null user_data */
    part_model.user_data = (void *)1;
    rc = md3_tree_create(dummy_engine, &part_model, &ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_tree_destroy(ptree);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_tree_create(dummy_engine, NULL, &tree);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Tree expansion with shift */
  rc = md3_tree_set_node_expanded(tree, (void *)100, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_set_node_expanded(tree, (void *)101, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_set_node_expanded(tree, (void *)100, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Tree expansion */
  rc = md3_tree_set_node_expanded(NULL, (void *)1, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_set_node_expanded(tree, NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_set_node_expanded(tree, (void *)1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_set_node_expanded(tree, (void *)1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_set_node_expanded(tree, (void *)1, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_set_node_expanded(tree, (void *)1, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 128; ++i) {
    rc = md3_tree_set_node_expanded(tree, (void *)(uintptr_t)(i + 1), 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_tree_set_node_expanded(tree, (void *)9999, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_tree_is_node_expanded(NULL, (void *)1, &i_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_is_node_expanded(tree, NULL, &i_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_is_node_expanded(tree, (void *)1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_is_node_expanded(tree, (void *)1, &i_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, i_val);
  rc = md3_tree_is_node_expanded(tree, (void *)9999, &i_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, i_val);

  rc = md3_tree_toggle_node_expanded(NULL, (void *)1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_toggle_node_expanded(tree, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_toggle_node_expanded(tree, (void *)1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_tree_set_indent(NULL, 10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_set_indent(tree, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_get_indent(NULL, &f_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_get_indent(tree, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_set_guide_lines(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Tree selection */
  rc = md3_tree_select_node(NULL, (void *)1, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_select_node(tree, NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_select_node(tree, (void *)1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_select_node(tree, (void *)1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_select_node(tree, (void *)1, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_select_node(tree, (void *)1, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_tree_set_multi_select(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_set_multi_select(tree, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_select_node(tree, (void *)100, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_select_node(tree, (void *)101, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_select_node(tree, (void *)100, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 128; ++i) {
    rc = md3_tree_select_node(tree, (void *)(uintptr_t)(i + 1), 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_tree_select_node(tree, (void *)9999, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_tree_is_node_selected(NULL, (void *)1, &i_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_is_node_selected(tree, NULL, &i_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_is_node_selected(tree, (void *)1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_is_node_selected(tree, (void *)1, &i_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, i_val);
  rc = md3_tree_is_node_selected(tree, (void *)9999, &i_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, i_val);

  /* Tree checked state */
  rc = md3_tree_set_node_checked_state(NULL, (void *)1, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_set_node_checked_state(tree, NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_set_node_checked_state(tree, (void *)1, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_set_node_checked_state(tree, (void *)1, 3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_set_node_checked_state(tree, (void *)1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_set_node_checked_state(tree, (void *)1, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 1; i < 128; ++i) {
    rc = md3_tree_set_node_checked_state(tree, (void *)(uintptr_t)(i + 1), 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_tree_set_node_checked_state(tree, (void *)9999, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_tree_get_node_checked_state(NULL, (void *)1, &i_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_get_node_checked_state(tree, NULL, &i_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_get_node_checked_state(tree, (void *)1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_get_node_checked_state(tree, (void *)1, &i_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, i_val);
  rc = md3_tree_get_node_checked_state(tree, (void *)9999, &i_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, i_val);

  rc = md3_tree_set_virtual_scroll(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_set_expressive(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_tree_destroy(tree);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 5. Tree Grid */
  rc = md3_tree_grid_create(dummy_engine, NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_tree_grid_create(dummy_engine, NULL, &tg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  tg->datagrid = NULL;
  rc = md3_tree_grid_destroy(tg);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_tree_grid_create(dummy_engine, NULL, &tg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_grid_set_column_count(NULL, 5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_set_column_count(tg, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Tree Grid expansion */
  rc = md3_tree_grid_set_node_expanded(tg, (void *)100, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_grid_set_node_expanded(tg, (void *)101, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_grid_set_node_expanded(tg, (void *)100, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_tree_grid_set_node_expanded(NULL, (void *)1, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_set_node_expanded(tg, NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_set_node_expanded(tg, (void *)1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_grid_set_node_expanded(tg, (void *)1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_grid_set_node_expanded(tg, (void *)1, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_grid_set_node_expanded(tg, (void *)1, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 128; ++i) {
    rc = md3_tree_grid_set_node_expanded(tg, (void *)(uintptr_t)(i + 1), 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_tree_grid_set_node_expanded(tg, (void *)9999, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_tree_grid_is_node_expanded(NULL, (void *)1, &i_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_is_node_expanded(tg, NULL, &i_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_is_node_expanded(tg, (void *)1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_is_node_expanded(tg, (void *)1, &i_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, i_val);
  rc = md3_tree_grid_is_node_expanded(tg, (void *)9999, &i_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, i_val);

  /* Tree Grid row selection */
  rc = md3_tree_grid_select_row(tg, (void *)100, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_grid_select_row(tg, (void *)101, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_grid_select_row(tg, (void *)100, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_tree_grid_select_row(NULL, (void *)1, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_select_row(tg, NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_select_row(tg, (void *)1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_grid_select_row(tg, (void *)1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_grid_select_row(tg, (void *)1, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_tree_grid_select_row(tg, (void *)1, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 128; ++i) {
    rc = md3_tree_grid_select_row(tg, (void *)(uintptr_t)(i + 1), 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_tree_grid_select_row(tg, (void *)9999, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_tree_grid_is_row_selected(NULL, (void *)1, &i_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_is_row_selected(tg, NULL, &i_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_is_row_selected(tg, (void *)1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_is_row_selected(tg, (void *)1, &i_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, i_val);
  rc = md3_tree_grid_is_row_selected(tg, (void *)9999, &i_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, i_val);

  rc = md3_tree_grid_set_indent(NULL, 10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_set_indent(tg, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_set_sticky_headers(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_set_zebra_shading(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tree_grid_set_expressive(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_tree_grid_destroy(tg);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 6. Transfer list */
  rc = md3_transfer_list_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_transfer_list_create(dummy_engine, &tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_transfer_list_add_item(NULL, 0, 1, "label", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_transfer_list_add_item(tl, 0, 1, NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_transfer_list_add_item(tl, 0, 1, "Left 1", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_add_item(tl, 0, 2, "Left 2", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_add_item(tl, 1, 3, "Right 1", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_add_item(tl, 1, 4, "Right 2", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_transfer_list_set_item_selected(NULL, 0, 1, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_transfer_list_set_item_selected(tl, 0, 999, 1);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);
  rc = md3_transfer_list_set_item_selected(tl, 1, 999, 1);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);

  /* Select 2nd item (not head) to test prev != NULL */
  rc = md3_transfer_list_set_item_selected(tl, 0, 1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_move_selected_right(tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_transfer_list_set_item_selected(tl, 1, 3, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_move_selected_left(tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_transfer_list_move_selected_right(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_transfer_list_move_all_right(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_transfer_list_move_selected_left(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_transfer_list_move_all_left(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_transfer_list_set_left_filter(NULL, "q");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_transfer_list_set_left_filter(tl, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_set_right_filter(NULL, "q");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_transfer_list_set_right_filter(tl, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_transfer_list_get_left_counts(NULL, &s_val, &s_val2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_transfer_list_get_left_counts(tl, NULL, &s_val2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_transfer_list_get_left_counts(tl, &s_val, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_transfer_list_get_right_counts(NULL, &s_val, &s_val2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_transfer_list_get_right_counts(tl, NULL, &s_val2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_transfer_list_get_right_counts(tl, &s_val, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Select right item and test right count with selection */
  rc = md3_transfer_list_set_item_selected(tl, 1, 4, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_transfer_list_get_right_counts(tl, &s_val, &s_val2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, (int)s_val);

  rc = md3_transfer_list_set_expressive(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_transfer_list_destroy(tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Transfer list move selected when item is not head */
  {
    struct md3_transfer_list *tl2 = NULL;
    rc = md3_transfer_list_create(dummy_engine, &tl2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_transfer_list_add_item(tl2, 0, 1, "A", NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_transfer_list_add_item(tl2, 0, 2, "B", NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_transfer_list_add_item(tl2, 0, 3, "C", NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    /* Select C (head) and move right */
    rc = md3_transfer_list_set_item_selected(tl2, 0, 3, 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_transfer_list_move_selected_right(tl2);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* Select A (not head) and move right */
    rc = md3_transfer_list_set_item_selected(tl2, 0, 1, 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_transfer_list_move_selected_right(tl2);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* On right side, add D, E */
    rc = md3_transfer_list_add_item(tl2, 1, 10, "D", NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_transfer_list_add_item(tl2, 1, 11, "E", NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    /* E is head on right */
    rc = md3_transfer_list_set_item_selected(tl2, 1, 11, 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_transfer_list_move_selected_left(tl2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    /* D is not head */
    rc = md3_transfer_list_set_item_selected(tl2, 1, 10, 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_transfer_list_move_selected_left(tl2);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = md3_transfer_list_destroy(tl2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  PASS();
}

TEST test_md3_data_hierarchy_mock_failures(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_table *table = NULL;
  struct md3_paginator *paginator = NULL;
  struct md3_sort_header *header = NULL;
  struct md3_tree *tree = NULL;
  struct md3_tree_grid *tg = NULL;
  struct md3_transfer_list *tl = NULL;
  size_t s_val;
  ui_error_t rc;

  /* Mock 1: table base create fails */
  g_md3_data_hierarchy_mock_fail = 1;
  rc = md3_table_create(dummy_engine, &table);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 2: table base destroy fails */
  g_md3_data_hierarchy_mock_fail = 0;
  rc = md3_table_create(dummy_engine, &table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_data_hierarchy_mock_fail = 2;
  rc = md3_table_destroy(table);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;

  /* Mock 3: datagrid base create fails */
  g_md3_data_hierarchy_mock_fail = 3;
  rc = md3_table_create(dummy_engine, &table);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 4: datagrid base destroy fails */
  g_md3_data_hierarchy_mock_fail = 0;
  rc = md3_table_create(dummy_engine, &table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_data_hierarchy_mock_fail = 4;
  rc = md3_table_destroy(table);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;
  rc = md3_table_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 5: datagrid resize column fails */
  rc = md3_table_create(dummy_engine, &table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_data_hierarchy_mock_fail = 5;
  rc = md3_table_resize_column(table, 0, 100);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;
  rc = md3_table_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 6: pagination base create fails */
  g_md3_data_hierarchy_mock_fail = 6;
  rc = md3_paginator_create(dummy_engine, &paginator);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 7: pagination base destroy fails */
  g_md3_data_hierarchy_mock_fail = 0;
  rc = md3_paginator_create(dummy_engine, &paginator);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_data_hierarchy_mock_fail = 7;
  rc = md3_paginator_destroy(paginator);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;

  /* Mock 8: pagination set_config fails */
  g_md3_data_hierarchy_mock_fail = 8;
  rc = md3_paginator_set_config(paginator, 100, 10);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;

  /* Mock 9: pagination get_current_page fails */
  g_md3_data_hierarchy_mock_fail = 9;
  rc = md3_paginator_get_current_page(paginator, &s_val);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;

  /* Mock 10: pagination set_current_page fails */
  g_md3_data_hierarchy_mock_fail = 10;
  rc = md3_paginator_set_current_page(paginator, 2);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;

  /* Mock 11: pagination get_total_pages fails */
  g_md3_data_hierarchy_mock_fail = 11;
  rc = md3_paginator_get_total_pages(paginator, &s_val);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;

  /* Mock 12: pagination next fails */
  g_md3_data_hierarchy_mock_fail = 12;
  rc = md3_paginator_next(paginator);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;

  /* Mock 13: pagination previous fails */
  g_md3_data_hierarchy_mock_fail = 13;
  rc = md3_paginator_prev(paginator);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;

  /* Mock 14: pagination first fails */
  g_md3_data_hierarchy_mock_fail = 14;
  rc = md3_paginator_first(paginator);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;

  /* Mock 15: pagination last fails */
  g_md3_data_hierarchy_mock_fail = 15;
  rc = md3_paginator_last(paginator);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;
  rc = md3_paginator_destroy(paginator);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 16: sort header base create fails */
  g_md3_data_hierarchy_mock_fail = 16;
  rc = md3_sort_header_create(dummy_engine, "col", &header);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 17: sort header base destroy fails */
  g_md3_data_hierarchy_mock_fail = 0;
  rc = md3_sort_header_create(dummy_engine, "col", &header);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_data_hierarchy_mock_fail = 17;
  rc = md3_sort_header_destroy(header);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;

  /* Mock 18: sort header base toggle fails */
  g_md3_data_hierarchy_mock_fail = 18;
  rc = md3_sort_header_cycle(header);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;
  rc = md3_sort_header_destroy(header);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 19: tree base create fails */
  g_md3_data_hierarchy_mock_fail = 19;
  rc = md3_tree_create(dummy_engine, NULL, &tree);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 20: tree base destroy fails */
  g_md3_data_hierarchy_mock_fail = 0;
  rc = md3_tree_create(dummy_engine, NULL, &tree);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_data_hierarchy_mock_fail = 20;
  rc = md3_tree_destroy(tree);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;
  rc = md3_tree_destroy(tree);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Tree Grid datagrid base create fails */
  g_md3_data_hierarchy_mock_fail = 3;
  rc = md3_tree_grid_create(dummy_engine, NULL, &tg);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Tree Grid datagrid base destroy fails */
  g_md3_data_hierarchy_mock_fail = 0;
  rc = md3_tree_grid_create(dummy_engine, NULL, &tg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_data_hierarchy_mock_fail = 4;
  rc = md3_tree_grid_destroy(tg);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;
  rc = md3_tree_grid_destroy(tg);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 21: transfer list base init fails */
  g_md3_data_hierarchy_mock_fail = 21;
  rc = md3_transfer_list_create(dummy_engine, &tl);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_data_hierarchy_mock_fail = 0;

#endif
  PASS();
}

TEST test_md3_data_hierarchy_oom_alloc(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_table *table = NULL;
  struct md3_paginator *paginator = NULL;
  struct md3_sort_header *header = NULL;
  struct md3_tree *tree = NULL;
  struct md3_tree_grid *tg = NULL;
  struct md3_transfer_list *tl = NULL;
  ui_error_t rc;

  g_malloc_fail_countdown = 0;
  rc = md3_table_create(dummy_engine, &table);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_paginator_create(dummy_engine, &paginator);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_sort_header_create(dummy_engine, "col", &header);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_tree_create(dummy_engine, NULL, &tree);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_tree_grid_create(dummy_engine, NULL, &tg);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_transfer_list_create(dummy_engine, &tl);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  rc = md3_transfer_list_create(dummy_engine, &tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = 0;
  rc = md3_transfer_list_add_item(tl, 0, 1, "test", NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
  rc = md3_transfer_list_destroy(tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#endif
  PASS();
}

SUITE(suite_material3_data_hierarchy) {
  RUN_TEST(test_md3_table_lifecycle_and_features);
  RUN_TEST(test_md3_paginator_lifecycle_and_navigation);
  RUN_TEST(test_md3_sort_header_lifecycle_and_cycling);
  RUN_TEST(test_md3_tree_lifecycle_and_features);
  RUN_TEST(test_md3_tree_grid_lifecycle_and_features);
  RUN_TEST(test_md3_transfer_list_lifecycle_and_moves);
  RUN_TEST(test_md3_data_hierarchy_branches);
  RUN_TEST(test_md3_data_hierarchy_mock_failures);
  RUN_TEST(test_md3_data_hierarchy_oom_alloc);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(suite_material3_data_hierarchy);
  GREATEST_MAIN_END();
}
