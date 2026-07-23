/* clang-format off */
#include "ui_tree_base.h"
#include "ui_dom_node.h"
#include "ui_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

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

static struct mock_node *roots[] = {&root1, &root2};

static enum ui_error setup_mock_tree(void) {
  root1.id = 1;
  root1.parent = NULL;
  root1.child_count = 2;
  root1.children = (struct mock_node **)malloc(2 * sizeof(struct mock_node *));
  root1.children[0] = &child1_1;
  root1.children[1] = &child1_2;

  root2.id = 2;
  root2.parent = NULL;
  root2.child_count = 0;
  root2.children = NULL;

  child1_1.id = 11;
  child1_1.parent = &root1;
  child1_1.child_count = 0;
  child1_1.children = NULL;

  child1_2.id = 12;
  child1_2.parent = &root1;
  child1_2.child_count = 0;
  child1_2.children = NULL;
  return UI_ERROR_NONE;
}

static enum ui_error teardown_mock_tree(void) {
  if (root1.children)
    free(root1.children);
  return UI_ERROR_NONE;
}

static size_t mock_get_root_count(void *user_data) {
  (void)user_data;
  return 2;
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

static enum ui_error mock_render_node(void *node_id,
                                      struct ui_dom_node *cell_node,
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

static int test_tree_render(void) {
  struct ui_tree_model model;
  struct ui_tree_base *tree;
  struct ui_dom_node *container;
  enum ui_error rc;
  int failed = 0;

  model.get_root_count = mock_get_root_count;
  model.get_root_node = mock_get_root_node;
  model.get_parent = mock_get_parent;
  model.get_child_count = mock_get_child_count;
  model.get_child = mock_get_child;
  model.render_node = mock_render_node;
  model.user_data = NULL;

  rc = ui_tree_base_create(&tree, &model);
  failed |= (rc != UI_ERROR_NONE);

  ui_tree_base_set_expanded(tree, &root1, 1);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
  rc = ui_tree_base_render(tree, container);
  failed |= (rc != UI_ERROR_NONE);

  ui_tree_base_destroy(tree);
  ui_dom_node_destroy(container);
  return failed;
}

static int test_tree_navigation(void) {
  struct ui_tree_model model;
  struct ui_tree_base *tree;
  struct ui_keyboard_event ev;
  enum ui_error rc;
  int failed = 0;

  model.get_root_count = mock_get_root_count;
  model.get_root_node = mock_get_root_node;
  model.get_parent = mock_get_parent;
  model.get_child_count = mock_get_child_count;
  model.get_child = mock_get_child;
  model.render_node = mock_render_node;
  model.user_data = NULL;

  rc = ui_tree_base_create(&tree, &model);
  failed |= (rc != UI_ERROR_NONE);

  ev.key_code = UI_KEY_DOWN;
  ui_tree_base_handle_key_event(tree, &ev);
  {
    void *tmp_node;
    failed |= (ui_tree_base_get_active_node(tree, &tmp_node) != UI_ERROR_NONE ||
               tmp_node != &root1);
  }

  ev.key_code = UI_KEY_RIGHT;
  ui_tree_base_handle_key_event(tree, &ev);
  {
    int is_expanded = 0;
    ui_tree_base_is_expanded(tree, &root1, &is_expanded);
    failed |= (!is_expanded);
  }
  {
    void *tmp_node;
    failed |= (ui_tree_base_get_active_node(tree, &tmp_node) != UI_ERROR_NONE ||
               tmp_node != &root1);
  }

  ui_tree_base_handle_key_event(tree, &ev);
  {
    void *tmp_node;
    failed |= (ui_tree_base_get_active_node(tree, &tmp_node) != UI_ERROR_NONE ||
               tmp_node != &child1_1);
  }

  ev.key_code = UI_KEY_DOWN;
  ui_tree_base_handle_key_event(tree, &ev);
  {
    void *tmp_node;
    failed |= (ui_tree_base_get_active_node(tree, &tmp_node) != UI_ERROR_NONE ||
               tmp_node != &child1_2);
  }

  ui_tree_base_handle_key_event(tree, &ev);
  {
    void *tmp_node;
    failed |= (ui_tree_base_get_active_node(tree, &tmp_node) != UI_ERROR_NONE ||
               tmp_node != &root2);
  }

  ev.key_code = UI_KEY_UP;
  ui_tree_base_handle_key_event(tree, &ev);
  {
    void *tmp_node;
    failed |= (ui_tree_base_get_active_node(tree, &tmp_node) != UI_ERROR_NONE ||
               tmp_node != &child1_2);
  }

  ev.key_code = UI_KEY_LEFT;
  ui_tree_base_handle_key_event(tree, &ev);
  {
    void *tmp_node;
    failed |= (ui_tree_base_get_active_node(tree, &tmp_node) != UI_ERROR_NONE ||
               tmp_node != &root1);
  }

  ui_tree_base_handle_key_event(tree, &ev);
  {
    int is_expanded = 0;
    ui_tree_base_is_expanded(tree, &root1, &is_expanded);
    failed |= (is_expanded);
  }

  rc = ui_tree_base_toggle_node(tree, &root1);
  {
    int is_expanded = 0;
    ui_tree_base_is_expanded(tree, &root1, &is_expanded);
    failed |= (rc != UI_ERROR_NONE || !is_expanded);
  }
  ui_tree_base_toggle_node(tree, &root1);
  {
    int is_expanded = 0;
    ui_tree_base_is_expanded(tree, &root1, &is_expanded);
    failed |= (is_expanded);
  }

  rc = ui_tree_base_set_active_node(tree, &child1_2);
  {
    void *tmp_node;
    failed |= (rc != UI_ERROR_NONE ||
               ui_tree_base_get_active_node(tree, &tmp_node) != UI_ERROR_NONE ||
               tmp_node != &child1_2);
  }

  /* Trigger mock callbacks via model structure for extra coverage */
  failed |= (model.get_root_count(NULL) != 2);
  failed |= (model.get_root_node(0, NULL) != &root1);
  failed |= (model.get_parent(&child1_1, NULL) != &root1);
  failed |= (model.get_child_count(&root1, NULL) != 2);
  failed |= (model.get_child(&root1, 0, NULL) != &child1_1);
  failed |= (model.render_node(NULL, NULL, NULL) != UI_ERROR_NONE);

  ui_tree_base_destroy(tree);
  return failed;
}

int main(void) {
  int failed = 0;
  printf("Running ui_tree_base tests...\n");

  setup_mock_tree();

  failed |= test_tree_render();
  failed |= test_tree_navigation();

  teardown_mock_tree();

  if (!failed) {
    printf("All tree base tests PASSED\n");
  }

  return failed;
}
