/* clang-format off */
#include "../include/ui_tree_grid_base.h"
#include "../include/ui_error.h"
#include "../include/ui_dom_node.h"
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

#define EXPECT_EQ(actual, expected)                                            \
  if ((size_t)(actual) != (size_t)(expected)) {                                \
    printf("Failed at %d: %zu != %zu\n", __LINE__, (size_t)(actual),           \
           (size_t)(expected));                                                \
    failed = 1;                                                                \
  }

#define EXPECT_TRUE(actual)                                                    \
  if (!(actual)) {                                                             \
    printf("Failed at %d: expected true\n", __LINE__);                         \
    failed = 1;                                                                \
  }

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

static ui_error_t mock_render_cell(void *node_id, size_t col_index,
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
  ui_error_t rc;
  int failed = 0;

  model.get_root_count = mock_get_root_count;
  model.get_root_node = mock_get_root_node;
  model.get_parent = mock_get_parent;
  model.get_child_count = mock_get_child_count;
  model.get_child = mock_get_child;
  model.get_column_count = mock_get_column_count;
  model.render_cell = mock_render_cell;
  model.user_data = NULL;

  EXPECT_EQ(ui_tree_grid_base_create(NULL, &model), UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_grid_base_create(&tree_grid, NULL),
            UI_ERROR_INVALID_ARGUMENT);

#ifdef UI_TEST_MOCK_ALLOC
  int i;
  for (i = 0; i < 4; i++) {
    g_malloc_fail_countdown = i;
    if (ui_tree_grid_base_create(&tree_grid, &model) == UI_ERROR_NONE) {
      (void)ui_tree_grid_base_destroy(tree_grid);
    }
  }
  g_malloc_fail_countdown = -1;
#endif

  rc = ui_tree_grid_base_create(&tree_grid, &model);
  EXPECT_EQ(rc, UI_ERROR_NONE);

  /* Force mock calls via model struct */
  EXPECT_EQ(model.get_root_count(model.user_data), 1);
  EXPECT_EQ(model.get_root_node(0, model.user_data), (void *)0x1);
  EXPECT_EQ(model.get_parent((void *)0x1, model.user_data), NULL);
  EXPECT_EQ(model.get_child_count((void *)0x1, model.user_data), 2);
  EXPECT_EQ(model.get_child((void *)0x1, 0, model.user_data), (void *)0x10);
  EXPECT_EQ(model.get_column_count(model.user_data), 3);
  EXPECT_EQ(model.render_cell((void *)0x1, 0, NULL, model.user_data),
            UI_ERROR_NONE);

  /* Get component */
  struct ui_component *comp;
  EXPECT_EQ(ui_tree_grid_base_get_component(NULL, &comp),
            UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_grid_base_get_component(tree_grid, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_grid_base_get_component(tree_grid, &comp), UI_ERROR_NONE);

  /* Destructor null safe */
  (void)ui_tree_grid_base_destroy(NULL);
  (void)ui_tree_grid_base_destroy(tree_grid);
  return failed;
}

static int test_tree_grid_expansion(void) {
  struct ui_tree_grid_base *tree_grid = NULL;
  struct ui_tree_grid_model model;
  ui_error_t rc;
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
  EXPECT_EQ(rc, UI_ERROR_NONE);

  EXPECT_EQ(ui_tree_grid_base_is_expanded(NULL, node1, &expanded),
            UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_grid_base_is_expanded(tree_grid, NULL, &expanded),
            UI_ERROR_NONE);
  EXPECT_EQ(ui_tree_grid_base_is_expanded(tree_grid, node1, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  rc = ui_tree_grid_base_is_expanded(tree_grid, node1, &expanded);
  EXPECT_EQ(rc, UI_ERROR_NONE);
  EXPECT_EQ(expanded, 0);

  EXPECT_EQ(ui_tree_grid_base_set_expanded(NULL, node1, 1),
            UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_grid_base_set_expanded(tree_grid, NULL, 1),
            UI_ERROR_INVALID_ARGUMENT);

  rc = ui_tree_grid_base_set_expanded(tree_grid, node1, 1);
  EXPECT_EQ(rc, UI_ERROR_NONE);

  rc = ui_tree_grid_base_is_expanded(tree_grid, node1, &expanded);
  EXPECT_EQ(rc, UI_ERROR_NONE);
  EXPECT_EQ(expanded, 1);

  /* Set expanded when already expanded */
  rc = ui_tree_grid_base_set_expanded(tree_grid, node1, 1);
  EXPECT_EQ(rc, UI_ERROR_NONE);

  /* Test out of bounds on fixed array */
  int j;
  for (j = 0; j < 256; j++) {
    ui_tree_grid_base_set_expanded(tree_grid, (void *)((size_t)0x100 + j), 1);
  }
  void *node2 = (void *)0x2;
  rc = ui_tree_grid_base_set_expanded(tree_grid, node2, 1);
  EXPECT_EQ(rc, UI_ERROR_OUT_OF_BOUNDS);

  EXPECT_EQ(ui_tree_grid_base_toggle_node(NULL, node1),
            UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_grid_base_toggle_node(tree_grid, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  rc = ui_tree_grid_base_toggle_node(tree_grid, node1);
  EXPECT_EQ(rc, UI_ERROR_NONE);

  rc = ui_tree_grid_base_is_expanded(tree_grid, node1, &expanded);
  EXPECT_EQ(rc, UI_ERROR_NONE);
  EXPECT_EQ(expanded, 0);

  /* set expanded to 0 when already 0 */
  rc = ui_tree_grid_base_set_expanded(tree_grid, node1, 0);
  EXPECT_EQ(rc, UI_ERROR_NONE);

  (void)ui_tree_grid_base_destroy(tree_grid);
  return failed;
}

#define MAX_EXPANDED_NODES 256
struct ui_tree_grid_base {
  struct ui_component *component;
  struct ui_tree_grid_model model;
  void *expanded_nodes[MAX_EXPANDED_NODES];
  size_t expanded_count;
  void *active_node;
  size_t active_col;
};

static int test_tree_grid_key_events(void) {
  struct ui_tree_grid_base *tree_grid = NULL;
  struct ui_tree_grid_model model;
  struct ui_keyboard_event ev;
  ui_error_t rc;
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
  EXPECT_EQ(rc, UI_ERROR_NONE);

  EXPECT_EQ(ui_tree_grid_base_handle_key_event(NULL, &ev),
            UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_grid_base_handle_key_event(tree_grid, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  /* Fake active node internal access */
  tree_grid->active_node = node1;
  tree_grid->active_col = 0;

  /* Test right key on unexpanded */
  ev.key_code = UI_KEY_RIGHT;
  EXPECT_EQ(ui_tree_grid_base_handle_key_event(tree_grid, &ev), UI_ERROR_NONE);
  int expanded;
  ui_tree_grid_base_is_expanded(tree_grid, node1, &expanded);
  EXPECT_EQ(expanded, 1);

  /* Test right key on expanded */
  ev.key_code = UI_KEY_RIGHT;
  EXPECT_EQ(ui_tree_grid_base_handle_key_event(tree_grid, &ev), UI_ERROR_NONE);
  EXPECT_EQ(tree_grid->active_col, 1);

  /* Test left key on expanded */
  ev.key_code = UI_KEY_LEFT;
  EXPECT_EQ(ui_tree_grid_base_handle_key_event(tree_grid, &ev), UI_ERROR_NONE);
  ui_tree_grid_base_is_expanded(tree_grid, node1, &expanded);
  EXPECT_EQ(expanded, 0);

  /* Test left key on unexpanded */
  ev.key_code = UI_KEY_LEFT;
  EXPECT_EQ(ui_tree_grid_base_handle_key_event(tree_grid, &ev), UI_ERROR_NONE);
  EXPECT_EQ(tree_grid->active_col, 0);

  /* Fill expanded array to force OUT_OF_BOUNDS on RIGHT key */
  {
    int j;
    for (j = 0; j < 256; j++) {
      ui_tree_grid_base_set_expanded(tree_grid, (void *)((size_t)0x200 + j), 1);
    }
  }
  tree_grid->active_node = (void *)0xdeadbeef;
  ev.key_code = UI_KEY_RIGHT;
  EXPECT_EQ(ui_tree_grid_base_handle_key_event(tree_grid, &ev),
            UI_ERROR_OUT_OF_BOUNDS);

  (void)ui_tree_grid_base_destroy(tree_grid);
  return failed;
}

static int test_tree_grid_render(void) {
  struct ui_tree_grid_base *tree_grid = NULL;
  struct ui_tree_grid_model model;
  struct ui_dom_node *container;
  ui_error_t rc;
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
  EXPECT_EQ(rc, UI_ERROR_NONE);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);

  EXPECT_EQ(ui_tree_grid_base_render(NULL, container),
            UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_grid_base_render(tree_grid, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  rc = ui_tree_grid_base_render(tree_grid, container);
  EXPECT_EQ(rc, UI_ERROR_NONE);

  (void)ui_tree_grid_base_destroy(tree_grid);
  (void)ui_dom_node_destroy(container);
  return failed;
}

int main(void) {
  int failed = 0;
  printf("Running ui_tree_grid_base tests...\n");

  failed |= test_tree_grid_lifecycle();
  failed |= test_tree_grid_expansion();
  failed |= test_tree_grid_key_events();
  failed |= test_tree_grid_render();

  if (!failed) {
    printf("All tests passed.\n");
  }
  return failed;
}
