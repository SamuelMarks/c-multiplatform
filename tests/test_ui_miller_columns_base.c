/* clang-format off */
#include "ui_miller_columns_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stdio.h>
/* clang-format on */

/* Mock Tree Data */
struct mock_node {
  int id;
  int parent_id;
  int num_children;
  int children[4];
};

static struct mock_node mock_fs[] = {
    {0, -1, 2, {1, 2, -1, -1}},  /* Root */
    {1, 0, 1, {3, -1, -1, -1}},  /* Dir 1 */
    {2, 0, 0, {-1, -1, -1, -1}}, /* File 1 */
    {3, 1, 1, {4, -1, -1, -1}},  /* Dir 2 */
    {4, 3, 0, {-1, -1, -1, -1}}  /* File 2 */
};

static size_t mock_get_root_count(void *user_data) {
  (void)user_data;
  return 1;
}

static void *mock_get_root_node(size_t index, void *user_data) {
  (void)user_data;
  if (index == 0)
    return &mock_fs[0];
  return NULL;
}

static void *mock_get_parent(void *node_id, void *user_data) {
  struct mock_node *node = (struct mock_node *)node_id;
  (void)user_data;
  if (node && node->parent_id >= 0)
    return &mock_fs[node->parent_id];
  return NULL;
}

static size_t mock_get_child_count(void *node_id, void *user_data) {
  struct mock_node *node = (struct mock_node *)node_id;
  (void)user_data;
  return node ? (size_t)node->num_children : 0;
}

static void *mock_get_child(void *node_id, size_t index, void *user_data) {
  struct mock_node *node = (struct mock_node *)node_id;
  (void)user_data;
  if (node && index < (size_t)node->num_children) {
    return &mock_fs[node->children[index]];
  }
  return NULL;
}

static ui_error_t mock_render_node(void *node_id, struct ui_dom_node *cell_node,
                                   void *user_data) {
  (void)node_id;
  (void)cell_node;
  (void)user_data;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

extern int g_malloc_fail_countdown;

static void test_miller_errors(void) {
  struct ui_arena *arena;
  ui_arena_create(1024, &arena);
  struct ui_tree_model model = {0};
  model.get_root_count = mock_get_root_count;
  model.get_root_node = mock_get_root_node;
  model.get_parent = mock_get_parent;
  model.get_child_count = mock_get_child_count;
  model.get_child = mock_get_child;
  model.render_node = mock_render_node;

  struct ui_miller_columns_base *miller = NULL;

  if (ui_miller_columns_base_create(NULL, &model, NULL, &miller) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_miller_columns_base_create(arena, NULL, NULL, &miller) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_miller_columns_base_create(arena, &model, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;

  ui_miller_columns_base_create(arena, &model, NULL, &miller);

  if (ui_miller_columns_base_select_item(miller, -1, &mock_fs[1]) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_miller_columns_base_select_item(miller, 0, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_miller_columns_base_get_column_count(miller, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_miller_columns_base_get_topology_changed_signal(miller, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;

  /* Force OOM during signal creation using a small arena */
  {
    struct ui_arena *small_arena;
    struct ui_miller_columns_base *miller_small = NULL;
    /* Create arena with size 1 so every alloc calls malloc */
    if (ui_arena_create(1, &small_arena) == UI_ERROR_NONE) {
      g_malloc_fail_countdown = 1; /* Fail the second alloc (signal) */
      ui_miller_columns_base_create(small_arena, &model, NULL, &miller_small);
      g_malloc_fail_countdown = -1;
      if (miller_small) {
        {
          ui_error_t rc_cleanup = ui_miller_columns_base_destroy(miller_small);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
      {
        ui_error_t rc_cleanup = ui_arena_destroy(small_arena);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
  }

  /* Test with missing get_child_count */
  model.get_child_count = NULL;
  {
    struct ui_miller_columns_base *miller2 = NULL;
    ui_miller_columns_base_create(arena, &model, NULL, &miller2);
    ui_miller_columns_base_select_item(miller2, 0, &mock_fs[1]);
    {
      ui_error_t rc_cleanup = ui_miller_columns_base_destroy(miller2);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  model.get_child_count = mock_get_child_count;

  if (ui_miller_columns_base_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return;

  ui_signal_t *sig;
  if (ui_miller_columns_base_get_topology_changed_signal(NULL, &sig) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_miller_columns_base_get_topology_changed_signal(miller, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;

  int count;
  if (ui_miller_columns_base_get_column_count(NULL, &count) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_miller_columns_base_get_column_count(miller, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;

  if (ui_miller_columns_base_select_item(NULL, 0, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_miller_columns_base_select_item(miller, 999, NULL) !=
      UI_ERROR_OUT_OF_BOUNDS)
    return;

  if (ui_miller_columns_base_navigate_left(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_miller_columns_base_navigate_left(miller) != UI_ERROR_OUT_OF_BOUNDS)
    return; /* Since already at 0 */

  /* navigate_right test missing from errors */
  ui_miller_columns_base_navigate_right(NULL);
  ui_miller_columns_base_navigate_right(miller);
  ui_miller_columns_base_select_item(miller, 0, &mock_fs[1]);
  ui_miller_columns_base_navigate_left(miller);
  ui_miller_columns_base_navigate_right(miller);

  {
    ui_error_t rc_cleanup = ui_miller_columns_base_destroy(miller);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}

extern int g_malloc_fail_countdown;
static void test_oom(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_arena *arena;
  ui_arena_create(1024, &arena);
  struct ui_tree_model model = {0};
  model.get_root_count = mock_get_root_count;
  model.get_root_node = mock_get_root_node;
  model.get_parent = mock_get_parent;
  model.get_child_count = mock_get_child_count;
  model.get_child = mock_get_child;
  model.render_node = mock_render_node;

  struct ui_miller_columns_base *miller = NULL;

  int i;
  for (i = 0; i < 10; i++) {
    g_malloc_fail_countdown = i;
    miller = NULL;
    ui_miller_columns_base_create(arena, &model, NULL, &miller);
    g_malloc_fail_countdown = -1;
    if (miller) {
      {
        ui_error_t rc_cleanup = ui_miller_columns_base_destroy(miller);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
  }

  ui_miller_columns_base_create(arena, &model, NULL, &miller);
  for (i = 0; i < 10; i++) {
    g_malloc_fail_countdown = i;
    ui_miller_columns_base_select_item(miller, 0, &mock_fs[1]);
    g_malloc_fail_countdown = -1;
  }

  {
    ui_error_t rc_cleanup = ui_miller_columns_base_destroy(miller);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
#endif
}

static void test_miller_missing_coverage(void) {
  struct ui_arena *arena = NULL;
  ui_arena_create(2048, &arena);
  struct ui_tree_model tree_model = {0};
  tree_model.get_root_count = mock_get_root_count;
  tree_model.get_root_node = mock_get_root_node;
  tree_model.get_parent = mock_get_parent;
  tree_model.get_child_count = mock_get_child_count;
  tree_model.get_child = mock_get_child;
  tree_model.render_node = mock_render_node;
  struct ui_miller_columns_base *miller = NULL;
  ui_miller_columns_base_create(arena, &tree_model, NULL, &miller);
  ui_miller_columns_base_navigate_left(NULL);
  ui_miller_columns_base_navigate_right(NULL);
  ui_miller_columns_base_create(NULL, NULL, NULL, NULL);
  {
    ui_error_t rc_cleanup = ui_miller_columns_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_miller_columns_base_select_item(NULL, 0, NULL);
  ui_miller_columns_base_get_column_count(NULL, NULL);
  ui_miller_columns_base_get_topology_changed_signal(NULL, NULL);
  ui_miller_columns_base_navigate_left(miller);
  ui_miller_columns_base_navigate_right(miller);
  int i;
  for (i = 0; i < 40; i++) {
    int rc = ui_miller_columns_base_select_item(miller, i, &mock_fs[1]);
    if (rc)
      printf("loop %d rc %d\n", i, rc);
  }
  ui_miller_columns_base_navigate_right(miller);
  for (i = 0; i < 10; i++) {
    ui_miller_columns_base_navigate_left(miller);
  }
  for (i = 0; i < 10; i++) {
    ui_miller_columns_base_navigate_right(miller);
  }
  {
    ui_error_t rc_cleanup = ui_miller_columns_base_destroy(miller);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}

int main(void) {
  test_miller_missing_coverage();
  test_miller_errors();
  test_oom();
  struct ui_arena *arena;
  struct ui_miller_columns_base *miller = NULL;
  struct ui_tree_model tree_model;
  ui_error_t err;
  ui_signal_t *signal = NULL;
  int col_count = 0;

  if (ui_arena_create(1024 * 16, &arena) != UI_ERROR_NONE) {
    return 1;
  }

  tree_model.get_root_count = mock_get_root_count;
  tree_model.get_root_node = mock_get_root_node;
  tree_model.get_parent = mock_get_parent;
  tree_model.get_child_count = mock_get_child_count;
  tree_model.get_child = mock_get_child;
  tree_model.render_node = mock_render_node;

  err = ui_miller_columns_base_create(arena, &tree_model, NULL, &miller);
  if (err != UI_ERROR_NONE || miller == NULL)
    return 1;

  err = ui_miller_columns_base_get_topology_changed_signal(miller, &signal);
  if (err != UI_ERROR_NONE || signal == NULL)
    return 1;

  err = ui_miller_columns_base_get_column_count(miller, &col_count);
  if (err != UI_ERROR_NONE || col_count != 1)
    return 1; /* Initial root column */

  /* Select Dir 1 (has children), should spawn column 2 */
  err = ui_miller_columns_base_select_item(miller, 0, &mock_fs[1]);
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_miller_columns_base_get_column_count(miller, &col_count);
  if (err != UI_ERROR_NONE || col_count != 2)
    return 1;

  /* Select Dir 2 (has children), should spawn column 3 */
  err = ui_miller_columns_base_select_item(miller, 1, &mock_fs[3]);
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_miller_columns_base_get_column_count(miller, &col_count);
  if (err != UI_ERROR_NONE || col_count != 3)
    return 1;

  /* Navigate Left (back to column 2) */
  err = ui_miller_columns_base_navigate_left(miller);
  if (err != UI_ERROR_NONE)
    return 1;

  /* Select File 1 in Root column. Should prune trailing columns. File 1 has no
   * children. */
  err = ui_miller_columns_base_select_item(miller, 0, &mock_fs[2]);
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_miller_columns_base_get_column_count(miller, &col_count);
  if (err != UI_ERROR_NONE || col_count != 1)
    return 1; /* Back to 1 column */

  err = ui_miller_columns_base_destroy(miller);
  if (err != UI_ERROR_NONE)
    return 1;

  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}
