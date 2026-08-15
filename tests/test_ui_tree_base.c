/* clang-format off */
#include "ui_tree_base.h"
#include "ui_dom_node.h"
#include "ui_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_computed;
extern ui_error_t ui_computed_destroy(struct ui_computed *node);

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

struct mock_node {
  int id;
  struct mock_node *parent;
  struct mock_node **children;
  size_t child_count;
};

static struct mock_node root1;
static struct mock_node root2;
static struct mock_node child1_1;
static struct mock_node child1_2;

static struct mock_node child2_1;

static struct mock_node *roots[] = {&root1, &root2};

static ui_error_t setup_mock_tree(void) {
  root1.id = 1;
  root1.parent = NULL;
  root1.child_count = 2;
  root1.children = (struct mock_node **)malloc(2 * sizeof(struct mock_node *));
  root1.children[0] = &child1_1;
  root1.children[1] = &child1_2;

  root2.id = 2;
  root2.parent = NULL;
  root2.child_count = 1;
  root2.children = (struct mock_node **)malloc(1 * sizeof(struct mock_node *));
  root2.children[0] = &child2_1;

  child1_1.id = 11;
  child1_1.parent = &root1;
  child1_1.child_count = 0;
  child1_1.children = NULL;

  child1_2.id = 12;
  child1_2.parent = &root1;
  child1_2.child_count = 0;
  child1_2.children = NULL;

  child2_1.id = 21;
  child2_1.parent = &root2;
  child2_1.child_count = 0;
  child2_1.children = NULL;
  return UI_ERROR_NONE;
}

static ui_error_t teardown_mock_tree(void) {
  if (root1.children)
    free(root1.children);
  if (root2.children)
    free(root2.children);
  return UI_ERROR_NONE;
}

static size_t mock_get_root_count(void *user_data) {
  (void)user_data;
  return 2;
}

static size_t mock_get_root_count_empty(void *user_data) {
  (void)user_data;
  return 0;
}

static void *mock_get_root_node(size_t index, void *user_data) {
  (void)user_data;
  return roots[index];
}

static void *mock_get_parent(void *node_id, void *user_data) {
  struct mock_node *n = (struct mock_node *)node_id;
  (void)user_data;
  return n ? n->parent : NULL;
}

static size_t mock_get_child_count(void *node_id, void *user_data) {
  struct mock_node *n = (struct mock_node *)node_id;
  (void)user_data;
  return n ? n->child_count : 0;
}

static void *mock_get_child(void *node_id, size_t index, void *user_data) {
  struct mock_node *n = (struct mock_node *)node_id;
  (void)user_data;
  return n ? n->children[index] : NULL;
}

static ui_error_t mock_render_node(void *node_id, struct ui_dom_node *cell_node,
                                   void *user_data) {
  struct mock_node *n = (struct mock_node *)node_id;
  char buf[32];
  (void)user_data;
  if (!n)
    return UI_ERROR_NONE;
#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "Node %d", n->id);
#else
  sprintf(buf, "Node %d", n->id);
#endif
  return ui_dom_node_set_attribute(cell_node, "data-name", buf);
}

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

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

static int test_tree_render(void) {
  struct ui_tree_model model;
  struct ui_tree_base *tree;
  struct ui_dom_node *container;
  ui_error_t rc;
  int failed = 0;

  model.get_root_count = mock_get_root_count;
  model.get_root_node = mock_get_root_node;
  model.get_parent = mock_get_parent;
  model.get_child_count = mock_get_child_count;
  model.get_child = mock_get_child;
  model.render_node = mock_render_node;
  model.user_data = NULL;

  /* NULL checks */
  EXPECT_EQ(ui_tree_base_create(NULL, &model), UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_base_create(&tree, NULL), UI_ERROR_INVALID_ARGUMENT);

  {
    struct ui_tree_model bad_model = model;
    bad_model.get_root_count = NULL;
    EXPECT_EQ(ui_tree_base_create(&tree, &bad_model),
              UI_ERROR_INVALID_ARGUMENT);
    bad_model.get_root_count = model.get_root_count;

    bad_model.get_root_node = NULL;
    EXPECT_EQ(ui_tree_base_create(&tree, &bad_model),
              UI_ERROR_INVALID_ARGUMENT);
    bad_model.get_root_node = model.get_root_node;

    bad_model.get_parent = NULL;
    EXPECT_EQ(ui_tree_base_create(&tree, &bad_model),
              UI_ERROR_INVALID_ARGUMENT);
    bad_model.get_parent = model.get_parent;

    bad_model.get_child_count = NULL;
    EXPECT_EQ(ui_tree_base_create(&tree, &bad_model),
              UI_ERROR_INVALID_ARGUMENT);
    bad_model.get_child_count = model.get_child_count;

    bad_model.get_child = NULL;
    EXPECT_EQ(ui_tree_base_create(&tree, &bad_model),
              UI_ERROR_INVALID_ARGUMENT);
    bad_model.get_child = model.get_child;

    bad_model.render_node = NULL;
    EXPECT_EQ(ui_tree_base_create(&tree, &bad_model),
              UI_ERROR_INVALID_ARGUMENT);
  }

  EXPECT_EQ(ui_tree_base_destroy(NULL), UI_ERROR_NONE);
  EXPECT_EQ(ui_tree_base_get_selection_model(NULL, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_base_bind_data(NULL, NULL), UI_ERROR_INVALID_ARGUMENT);

  {
    /* Redeclare ui_tree_base to mock missing destroy coverage */
    struct ui_tree_base_mock {
      struct ui_tree_model model;
      void **expanded_nodes;
      size_t num_expanded;
      size_t expanded_cap;
      void *active_node;
      struct ui_selection_model *selection_model;
      struct ui_computed *data_signal;
    };
    struct ui_tree_base *mock_tree = NULL;
    if (ui_tree_base_create(&mock_tree, &model) == UI_ERROR_NONE) {
      struct ui_tree_base_mock *m = (struct ui_tree_base_mock *)mock_tree;
      if (m->selection_model) {
        ui_selection_model_destroy(m->selection_model);
        m->selection_model = NULL;
      }
      if (m->expanded_nodes) {
        free(m->expanded_nodes);
        m->expanded_nodes = NULL;
      }
      if (m->data_signal) {
        ui_computed_destroy(m->data_signal);
        m->data_signal = NULL;
      }
      ui_tree_base_destroy(mock_tree);
    }
  }

#ifdef UI_TEST_MOCK_ALLOC
  int i;
  for (i = 0; i < 4; i++) {
    g_malloc_fail_countdown = i;
    if (ui_tree_base_create(&tree, &model) == UI_ERROR_NONE) {
      (void)ui_tree_base_destroy(tree);
    }
  }
  g_malloc_fail_countdown = -1;
#endif

  rc = ui_tree_base_create(&tree, &model);
  EXPECT_EQ(rc, UI_ERROR_NONE);

  /* Set expanded OOM */
#ifdef UI_TEST_MOCK_ALLOC
  for (i = 0; i < 3; i++) {
    g_malloc_fail_countdown = i;
    ui_tree_base_set_expanded(tree, &root2, 1);
  }
  g_malloc_fail_countdown = -1;
#endif

  ui_tree_base_set_expanded(tree, &root1, 1);
  ui_tree_base_set_expanded(tree, &root2, 1);
  ui_tree_base_set_expanded(tree, &root2, 0);

  /* NULL checks for coverage */
  {
    int is_exp;
    struct ui_selection_model *sm;
    EXPECT_EQ(ui_tree_base_get_selection_model(tree, NULL),
              UI_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ui_tree_base_is_expanded(NULL, &root1, &is_exp),
              UI_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ui_tree_base_is_expanded(tree, NULL, &is_exp),
              UI_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ui_tree_base_is_expanded(tree, &root1, NULL),
              UI_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ui_tree_base_set_expanded(NULL, &root1, 1),
              UI_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ui_tree_base_set_expanded(tree, NULL, 1),
              UI_ERROR_INVALID_ARGUMENT);
  }

  /* Trigger reallocation of expanded_nodes (cap > 8) */
  {
    size_t k;
    for (k = 0; k < 20; k++) {
      ui_tree_base_set_expanded(tree, (void *)((size_t)(100 + k)), 1);
    }
  }

  /* Select a node */
  {
    struct ui_selection_model *sm;
    ui_tree_base_get_selection_model(tree, &sm);
    ui_selection_model_toggle(sm, &root1);
  }

  /* test get_selection_model */
  {
    struct ui_selection_model *sm;
    EXPECT_EQ(ui_tree_base_get_selection_model(tree, &sm), UI_ERROR_NONE);
  }

  /* test bind_data */
  EXPECT_EQ(ui_tree_base_bind_data(tree, (struct ui_computed *)0x1),
            UI_ERROR_NONE);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);

  /* Render OOM */
#ifdef UI_TEST_MOCK_ALLOC
  for (i = 0; i < 100; i++) {
    g_malloc_fail_countdown = i;
    if (ui_tree_base_render(tree, container) == UI_ERROR_NONE) {
      if (container->first_child) {
        (void)ui_dom_node_destroy(container->first_child);
        container->first_child = NULL;
        container->last_child = NULL;
      }
    }
  }
  g_malloc_fail_countdown = -1;
#endif

  /* Unexpand root2 to test rendering of a collapsed node */
  ui_tree_base_set_expanded(tree, &root2, 0);

  rc = ui_tree_base_render(tree, container);
  EXPECT_EQ(rc, UI_ERROR_NONE);

  (void)ui_tree_base_destroy(tree);
  (void)ui_dom_node_destroy(container);
  return failed;
}

static int test_tree_navigation(void) {
  struct ui_tree_model model;
  struct ui_tree_base *tree;
  struct ui_keyboard_event ev;
  ui_error_t rc;
  void *tmp_node = NULL;
  int is_expanded = 0;
  int failed = 0;

  model.get_root_count = mock_get_root_count;
  model.get_root_node = mock_get_root_node;
  model.get_parent = mock_get_parent;
  model.get_child_count = mock_get_child_count;
  model.get_child = mock_get_child;
  model.render_node = mock_render_node;
  model.user_data = NULL;

  rc = ui_tree_base_create(&tree, &model);
  EXPECT_EQ(rc, UI_ERROR_NONE);

  /* NULL check for key handle */
  EXPECT_EQ(ui_tree_base_handle_key_event(NULL, &ev),
            UI_ERROR_INVALID_ARGUMENT);

  ev.key_code = UI_KEY_DOWN;
  ui_tree_base_handle_key_event(tree, &ev);
  {
    EXPECT_EQ(ui_tree_base_get_active_node(tree, &tmp_node), UI_ERROR_NONE);
    EXPECT_EQ(tmp_node, &root1);
  }

  /* Key down from an expanded node to hit line 178-179 */
  ui_tree_base_set_expanded(tree, &root1, 1);
  ui_tree_base_handle_key_event(tree, &ev);
  EXPECT_EQ(ui_tree_base_get_active_node(tree, &tmp_node), UI_ERROR_NONE);
  EXPECT_EQ(tmp_node, &child1_1);

  /* Set active node back to root1 for next key right test */
  ui_tree_base_set_active_node(tree, &root1);
  ui_tree_base_set_expanded(tree, &root1, 1);
  ui_tree_base_set_expanded(tree, &root1, 0);

  ev.key_code = UI_KEY_RIGHT;
  ui_tree_base_handle_key_event(tree, &ev);
  {
    is_expanded = 0;
    ui_tree_base_is_expanded(tree, &root1, &is_expanded);
    EXPECT_TRUE(is_expanded);
  }
  {
    EXPECT_EQ(ui_tree_base_get_active_node(tree, &tmp_node), UI_ERROR_NONE);
    EXPECT_EQ(tmp_node, &root1);
  }

  ui_tree_base_handle_key_event(tree, &ev);
  {
    EXPECT_EQ(ui_tree_base_get_active_node(tree, &tmp_node), UI_ERROR_NONE);
    EXPECT_EQ(tmp_node, &child1_1);
  }

  /* Test RIGHT on leaf node */
  ui_tree_base_set_active_node(tree, &child1_1);
  ev.key_code = UI_KEY_RIGHT;
  ui_tree_base_handle_key_event(tree, &ev);

  /* Test Enter key */
  ev.key_code = UI_KEY_ENTER;
  ui_tree_base_handle_key_event(tree, &ev);

  /* Test Space key */
  ev.key_code = UI_KEY_SPACE;
  ui_tree_base_handle_key_event(tree, &ev);

  ev.key_code = UI_KEY_DOWN;
  ui_tree_base_handle_key_event(tree, &ev);
  {
    EXPECT_EQ(ui_tree_base_get_active_node(tree, &tmp_node), UI_ERROR_NONE);
    EXPECT_EQ(tmp_node, &child1_2);
  }

  ui_tree_base_handle_key_event(tree, &ev);
  {
    EXPECT_EQ(ui_tree_base_get_active_node(tree, &tmp_node), UI_ERROR_NONE);
    EXPECT_EQ(tmp_node, &root2);
  }

  /* Trigger line 196: keep pressing down at end of tree */
  ui_tree_base_handle_key_event(tree, &ev);
  {
    EXPECT_EQ(ui_tree_base_get_active_node(tree, &tmp_node), UI_ERROR_NONE);
    EXPECT_EQ(tmp_node, &root2);
  }

  ev.key_code = UI_KEY_UP;
  ui_tree_base_handle_key_event(tree, &ev);
  {
    EXPECT_EQ(ui_tree_base_get_active_node(tree, &tmp_node), UI_ERROR_NONE);
    EXPECT_EQ(tmp_node, &child1_2);
  }

  ev.key_code = UI_KEY_LEFT;
  ui_tree_base_handle_key_event(tree, &ev);
  {
    EXPECT_EQ(ui_tree_base_get_active_node(tree, &tmp_node), UI_ERROR_NONE);
    EXPECT_EQ(tmp_node, &root1);
  }

  ui_tree_base_handle_key_event(tree, &ev);
  {
    is_expanded = 0;
    ui_tree_base_is_expanded(tree, &root1, &is_expanded);
    EXPECT_TRUE(!is_expanded);
  }

  /* Trigger line 223: press left on unexpanded root */
  ui_tree_base_handle_key_event(tree, &ev);

  /* Try UP from root2 to hit unexpanded root1 */
  ui_tree_base_set_active_node(tree, &root2);
  ev.key_code = UI_KEY_UP;
  ui_tree_base_handle_key_event(tree, &ev);

  /* Try UP from child1_2 to hit child1_1 which is expanded but has 0 children
   */
  ui_tree_base_set_expanded(tree, &child1_1, 1);
  ui_tree_base_set_active_node(tree, &child1_2);
  ev.key_code = UI_KEY_UP;
  ui_tree_base_handle_key_event(tree, &ev);
  {
    EXPECT_EQ(ui_tree_base_get_active_node(tree, &tmp_node), UI_ERROR_NONE);
    EXPECT_EQ(tmp_node, &child1_1);
  }

  /* Try DOWN from child1_1 to hit is_expanded && child_count > 0 == false */
  ui_tree_base_set_active_node(tree, &child1_1);
  ev.key_code = UI_KEY_DOWN;
  ui_tree_base_handle_key_event(tree, &ev);
  {
    EXPECT_EQ(ui_tree_base_get_active_node(tree, &tmp_node), UI_ERROR_NONE);
    EXPECT_EQ(tmp_node, &child1_2);
  }

  /* Trigger line 223: press UP on first root */
  ui_tree_base_set_active_node(tree, &root1);
  ev.key_code = UI_KEY_UP;
  ui_tree_base_handle_key_event(tree, &ev);

  /* Trigger line 167: search for non-existent node */
  static struct mock_node fake_node;
  fake_node.id = 999;
  fake_node.parent = &root1;
  fake_node.children = NULL;
  fake_node.child_count = 0;
  ui_tree_base_set_active_node(tree, &fake_node);
  ev.key_code = UI_KEY_UP;
  ui_tree_base_handle_key_event(tree, &ev);

  /* Test DOWN on empty tree */
  {
    struct ui_tree_base *empty_tree;
    struct ui_tree_model empty_model = model;
    empty_model.get_root_count = mock_get_root_count_empty;
    ui_tree_base_create(&empty_tree, &empty_model);
    ev.key_code = UI_KEY_DOWN;
    ui_tree_base_handle_key_event(empty_tree, &ev);
    ui_tree_base_destroy(empty_tree);
  }

  /* Test toggle with mock alloc */
  rc = ui_tree_base_toggle_node(tree, &root1);
  {
    is_expanded = 0;
    ui_tree_base_is_expanded(tree, &root1, &is_expanded);
    EXPECT_TRUE(is_expanded);
  }
  ui_tree_base_toggle_node(tree, &root1);
  {
    is_expanded = 0;
    ui_tree_base_is_expanded(tree, &root1, &is_expanded);
    EXPECT_TRUE(!is_expanded);
  }

  rc = ui_tree_base_set_active_node(tree, &child1_2);
  {
    EXPECT_EQ(rc, UI_ERROR_NONE);
    EXPECT_EQ(ui_tree_base_get_active_node(tree, &tmp_node), UI_ERROR_NONE);
    EXPECT_EQ(tmp_node, &child1_2);
  }

  /* Trigger line 291: unregistered key */
  ev.key_code = UI_KEY_TAB;
  ui_tree_base_handle_key_event(tree, &ev);

#ifdef UI_TEST_MOCK_ALLOC
  /* Test allocation failures in navigation handling */
  {
    int j;
    for (j = 0; j < 3; j++) {
      struct ui_tree_base *mock_tree;
      g_malloc_fail_countdown = -1; /* Reset for tree creation */
      (void)ui_tree_base_create(&mock_tree, &model);

      g_malloc_fail_countdown = j;
      ui_tree_base_set_active_node(mock_tree, &root1);
      ev.key_code = UI_KEY_RIGHT;
      (void)ui_tree_base_handle_key_event(mock_tree, &ev);

      g_malloc_fail_countdown = -1; /* Reset to allow cleanup */
      (void)ui_tree_base_destroy(mock_tree);

      (void)ui_tree_base_create(&mock_tree, &model);
      g_malloc_fail_countdown = j;
      ui_tree_base_set_active_node(mock_tree, &root1);
      ev.key_code = UI_KEY_ENTER;
      (void)ui_tree_base_handle_key_event(mock_tree, &ev);

      g_malloc_fail_countdown = -1;
      (void)ui_tree_base_destroy(mock_tree);
    }
  }
#endif

  /* Trigger line 167: search for non-existent node */
  ui_tree_base_set_active_node(tree, (void *)0xdeadbeef);

  /* Null checks */
  EXPECT_EQ(ui_tree_base_set_active_node(NULL, &child1_2),
            UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_base_get_active_node(NULL, &tmp_node),
            UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_base_is_expanded(NULL, &root1, &is_expanded),
            UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_base_set_expanded(NULL, &root1, 1),
            UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_base_toggle_node(NULL, &root1), UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_base_toggle_node(tree, NULL), UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_base_get_active_node(tree, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_base_handle_key_event(NULL, &ev),
            UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_base_handle_key_event(tree, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_base_render(NULL, NULL), UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ(ui_tree_base_render(tree, NULL), UI_ERROR_INVALID_ARGUMENT);

  /* Trigger mock callbacks via model structure for extra coverage */
  EXPECT_EQ(model.get_root_count(NULL), 2);
  EXPECT_EQ(model.get_root_node(0, NULL), &root1);
  EXPECT_EQ(model.get_parent(&child1_1, NULL), &root1);
  EXPECT_EQ(model.get_child_count(&root1, NULL), 2);
  EXPECT_EQ(model.get_child(&root1, 0, NULL), &child1_1);
  EXPECT_EQ(model.render_node(NULL, NULL, NULL), UI_ERROR_NONE);

  (void)ui_tree_base_destroy(tree);
  return failed;
}

int main(void) {
  int failed = 0;
  printf("Running ui_tree_base tests...\n");

  setup_mock_tree();

  failed |= test_tree_render();
  if (failed)
    printf("test_tree_render failed\n");
  int old_failed = failed;
  failed |= test_tree_navigation();
  if (failed > old_failed)
    printf("test_tree_navigation failed\n");

  teardown_mock_tree();

  if (!failed) {
    printf("All tree base tests PASSED\n");
  }

  return failed;
}
