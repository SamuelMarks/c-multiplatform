/* clang-format off */
#include "ui_mutation_observer.h"
#include "ui_error.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int g_cb_called = 0;
static struct ui_mutation_record g_last_record;

static enum ui_error mutation_cb(struct ui_mutation_observer *observer,
                                 const struct ui_mutation_record *records,
                                 int record_count, void *user_data) {
  int i;
  (void)observer;
  (void)user_data;
  g_cb_called += record_count;
  if (record_count > 0) {
    g_last_record = records[record_count - 1]; /* Copy last for testing */
  }
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_mutation_observer *obs = NULL;
  struct ui_mutation_observer *obs_array[70];
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *child1 = NULL;
  struct ui_dom_node *child2 = NULL;
  struct ui_dom_node *nodes[10];
  struct ui_mutation_observer_init options = {1, 1, 1, 1, 1};
  enum ui_error rc;
  int i;

  if (ui_mutation_observer_create(NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_mutation_observer_create(mutation_cb, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_mutation_observer_destroy(NULL);
  if (ui_mutation_observer_observe(NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_mutation_observer_disconnect(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root) != UI_ERROR_NONE)
    return 1;
  if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1) != UI_ERROR_NONE)
    return 1;
  if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child2) != UI_ERROR_NONE)
    return 1;
  for (i = 0; i < 10; i++) {
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &nodes[i]);
  }

  if (ui_mutation_observer_create(mutation_cb, NULL, &obs) != UI_ERROR_NONE)
    return 1;

  /* Test re-observing the same target (should return NONE without adding) */
  if (ui_mutation_observer_observe(obs, root, &options) != UI_ERROR_NONE)
    return 1;
  if (ui_mutation_observer_observe(obs, root, &options) != UI_ERROR_NONE)
    return 1;

  /* Test capacity growing (requires observing > 4 targets) */
  for (i = 0; i < 10; i++) {
    if (ui_mutation_observer_observe(obs, nodes[i], &options) != UI_ERROR_NONE)
      return 1;
  }

  /* Test notify_child_list with added */
  g_cb_called = 0;
  ui_dom_node_remove_child(root, child1);
  ui_dom_node_append_child(root, child1);
  if (g_cb_called != 1 || g_last_record.type != UI_MUTATION_TYPE_CHILD_LIST)
    return 1;

  /* Test notify_child_list with removed */
  g_cb_called = 0;
  ui_dom_node_remove_child(root, child1);
  if (g_cb_called != 1 || g_last_record.type != UI_MUTATION_TYPE_CHILD_LIST ||
      g_last_record.removed_nodes_count != 1)
    return 1;

  /* Test ancestor path (subtree observing) */
  ui_dom_node_remove_child(root, child1);
  ui_dom_node_append_child(root, child1);
  g_cb_called = 0;
  ui_dom_node_append_child(
      child1, child2); /* should trigger root's observer because subtree=1 */
  if (g_cb_called != 1 || g_last_record.type != UI_MUTATION_TYPE_CHILD_LIST)
    return 1;

  /* Test attributes with old value */
  g_cb_called = 0;
  ui_dom_node_set_attribute(root, "id", "test1");
  ui_dom_node_set_attribute(root, "id", "test2");
  if (g_cb_called != 2 || g_last_record.type != UI_MUTATION_TYPE_ATTRIBUTES)
    return 1;

  /* Test character data with old value */
  struct ui_dom_node *text_node = NULL;
  ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
  ui_dom_node_set_text_content(text_node, "init");
  ui_dom_node_append_child(root, text_node); /* trigger 1 child list */
  g_cb_called = 0;
  ui_dom_node_set_text_content(
      text_node, "new text"); /* trigger char data on root via subtree */
  if (g_cb_called != 1 || g_last_record.type != UI_MUTATION_TYPE_CHARACTER_DATA)
    return 1;

  ui_mutation_observer_disconnect(obs);
  ui_mutation_observer_destroy(obs);

  /* Test options disabled */
  {
    struct ui_mutation_observer_init opt_none = {0, 0, 0, 0, 0};
    ui_mutation_observer_create(mutation_cb, NULL, &obs);
    ui_mutation_observer_observe(obs, root, &opt_none);

    struct ui_dom_node *text_node2 = NULL;
    ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node2);
    ui_mutation_observer_observe(obs, text_node2, &opt_none);

    g_cb_called = 0;
    ui_dom_node_remove_child(root, child1);
    ui_dom_node_append_child(root, child1);
    ui_dom_node_set_attribute(root, "id", "x");
    ui_dom_node_set_text_content(text_node2, "x");
    if (g_cb_called != 0)
      return 1; /* Should not be called */

    /* Test subtree attribute */
    struct ui_mutation_observer_init opt_sub = {1, 1, 1, 1, 1};
    ui_mutation_observer_observe(obs, root, &opt_sub);
    ui_dom_node_set_attribute(child1, "class",
                              "y"); /* trigger attribute via subtree */
    if (g_cb_called != 1)
      return 1;

    ui_dom_node_destroy(text_node2);
    ui_mutation_observer_destroy(obs);
  }

  /* Test NULL internally */
  ui_mutation_observer_notify_child_list(NULL, NULL, NULL);
  ui_mutation_observer_notify_attribute(NULL, NULL, NULL);
  ui_mutation_observer_notify_character_data(NULL, NULL);

  /* Test global registry full */
  for (i = 0; i < 70; i++) {
    obs_array[i] = NULL;
    rc = ui_mutation_observer_create(mutation_cb, NULL, &obs_array[i]);
    if (i >= 64) {
      if (rc != UI_ERROR_OUT_OF_BOUNDS)
        return 1;
    } else {
      if (rc != UI_ERROR_NONE)
        return 1;
    }
  }

  for (i = 0; i < 64; i++) {
    ui_mutation_observer_destroy(obs_array[i]);
  }

  ui_dom_node_destroy(root); /* destroys child1, child2, text_node */
  for (i = 0; i < 10; i++) {
    ui_dom_node_destroy(nodes[i]);
  }

  return 0;
}

static int run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_mutation_observer *obs = NULL;
  struct ui_dom_node *root = NULL;
  struct ui_mutation_observer_init options = {1, 1, 1, 1, 1};
  enum ui_error rc;

  g_malloc_fail_countdown = 0;
  if (ui_mutation_observer_create(mutation_cb, NULL, &obs) !=
      UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 1;
  if (ui_mutation_observer_create(mutation_cb, NULL, &obs) !=
      UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  ui_mutation_observer_create(mutation_cb, NULL, &obs);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);

  /* Trigger array capacity OOM */
  struct ui_dom_node *n1 = NULL, *n2 = NULL, *n3 = NULL, *n4 = NULL, *n5 = NULL;
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n1);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n2);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n3);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n4);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n5);
  ui_mutation_observer_observe(obs, n1, &options);
  ui_mutation_observer_observe(obs, n2, &options);
  ui_mutation_observer_observe(obs, n3, &options);
  ui_mutation_observer_observe(obs, n4, &options);

  g_malloc_fail_countdown = 0;
  if (ui_mutation_observer_observe(obs, n5, &options) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  ui_dom_node_destroy(n1);
  ui_dom_node_destroy(n2);
  ui_dom_node_destroy(n3);
  ui_dom_node_destroy(n4);
  ui_dom_node_destroy(n5);
  ui_mutation_observer_destroy(obs);
  ui_dom_node_destroy(root);

  /* Trigger string OOM in attribute notify */
  {
    ui_mutation_observer_create(mutation_cb, NULL, &obs);
    struct ui_dom_node *test_node = NULL;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &test_node);
    ui_mutation_observer_observe(obs, test_node, &options);

    g_malloc_fail_countdown = 0;
    ui_mutation_observer_notify_attribute(test_node, "test", "old_val");
    g_malloc_fail_countdown = -1;

    g_malloc_fail_countdown = 1;
    ui_mutation_observer_notify_attribute(test_node, "test", "old_val");
    g_malloc_fail_countdown = -1;

    /* Trigger string OOM in character_data notify */
    g_malloc_fail_countdown = 0;
    ui_mutation_observer_notify_character_data(test_node, "old_val");
    g_malloc_fail_countdown = -1;

    ui_dom_node_destroy(test_node);
    ui_mutation_observer_destroy(obs);
  }
#endif
  return 0;
}

int main(void) {
  if (run_normal_tests() != 0)
    return 1;
  if (run_oom_tests() != 0)
    return 1;
  printf("All ui_mutation_observer tests passed.\n");
  return 0;
}
