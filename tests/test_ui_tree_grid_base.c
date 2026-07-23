/* clang-format off */
#include "../include/ui_tree_grid_base.h"
#include "../include/ui_error.h"
#include "../include/ui_dom_node.h"
#include <stdio.h>
/* clang-format on */

static size_t mock_get_root_count(void *user_data) {
  (void)user_data;
  return 1;
}

static void *mock_get_root_node(size_t index, void *user_data) {
  (void)index;
  (void)user_data;
  return (void *)0x1;
}

static void *mock_get_parent(void *node_id, void *user_data) {
  (void)node_id;
  (void)user_data;
  return NULL;
}

static size_t mock_get_child_count(void *node_id, void *user_data) {
  (void)node_id;
  (void)user_data;
  return 2;
}

static void *mock_get_child(void *node_id, size_t index, void *user_data) {
  (void)node_id;
  (void)index;
  (void)user_data;
  return (void *)(0x10 + index);
}

static size_t mock_get_column_count(void *user_data) {
  (void)user_data;
  return 3;
}

static enum ui_error mock_render_cell(void *node_id, size_t col_index,
                                      struct ui_dom_node *cell_node,
                                      void *user_data) {
  (void)node_id;
  (void)col_index;
  (void)cell_node;
  (void)user_data;
  return UI_ERROR_NONE;
}

static int test_tree_grid_lifecycle(void) {
  struct ui_tree_grid_base *tree_grid = NULL;
  struct ui_tree_grid_model model;
  enum ui_error rc;
  int failed = 0;

  model.get_root_count = mock_get_root_count;
  model.get_root_node = mock_get_root_node;
  model.get_parent = mock_get_parent;
  model.get_child_count = mock_get_child_count;
  model.get_child = mock_get_child;
  model.get_column_count = mock_get_column_count;
  model.render_cell = mock_render_cell;
  model.user_data = NULL;

  rc = ui_tree_grid_base_create(&tree_grid, &model);
  failed |= (rc != UI_ERROR_NONE || tree_grid == NULL);

  /* Force mock calls via model struct */
  failed |= (model.get_root_count(model.user_data) != 1);
  failed |= (model.get_root_node(0, model.user_data) != (void *)0x1);
  failed |= (model.get_parent((void *)0x1, model.user_data) != NULL);
  failed |= (model.get_child_count((void *)0x1, model.user_data) != 2);
  failed |= (model.get_child((void *)0x1, 0, model.user_data) != (void *)0x10);
  failed |= (model.get_column_count(model.user_data) != 3);
  failed |= (model.render_cell((void *)0x1, 0, NULL, model.user_data) !=
             UI_ERROR_NONE);

  ui_tree_grid_base_destroy(tree_grid);
  return failed;
}

static int test_tree_grid_expansion(void) {
  struct ui_tree_grid_base *tree_grid = NULL;
  struct ui_tree_grid_model model;
  enum ui_error rc;
  int expanded;
  void *node1 = (void *)0x1;
  int failed = 0;

  model.get_root_count = mock_get_root_count;
  model.get_root_node = mock_get_root_node;
  model.get_parent = mock_get_parent;
  model.get_child_count = mock_get_child_count;
  model.get_child = mock_get_child;
  model.get_column_count = mock_get_column_count;
  model.render_cell = mock_render_cell;
  model.user_data = NULL;

  rc = ui_tree_grid_base_create(&tree_grid, &model);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tree_grid_base_is_expanded(tree_grid, node1, &expanded);
  failed |= (rc != UI_ERROR_NONE || expanded);

  rc = ui_tree_grid_base_set_expanded(tree_grid, node1, 1);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tree_grid_base_is_expanded(tree_grid, node1, &expanded);
  failed |= (rc != UI_ERROR_NONE || !expanded);

  rc = ui_tree_grid_base_toggle_node(tree_grid, node1);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tree_grid_base_is_expanded(tree_grid, node1, &expanded);
  failed |= (rc != UI_ERROR_NONE || expanded);

  ui_tree_grid_base_destroy(tree_grid);
  return failed;
}

int main(void) {
  int failed = 0;
  printf("Running ui_tree_grid_base tests...\n");

  failed |= test_tree_grid_lifecycle();
  failed |= test_tree_grid_expansion();

  if (!failed) {
    printf("All tests passed.\n");
  }
  return failed;
}
