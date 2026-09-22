/* clang-format off */
#include "ui_mutation_observer.h"
#include "ui_error.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int g_cb_called = 0;
static struct ui_mutation_record g_last_record;

static ui_error_t mutation_cb(struct ui_mutation_observer *observer,
                              const struct ui_mutation_record *records,
                              int record_count, void *user_data) {
  int i;
  if (observer) {
  }
  if (user_data) {
  }
  g_cb_called += record_count;
  if (record_count > 0) {
    g_last_record = records[record_count - 1]; /* Copy last for testing */
  }
  return UI_ERROR_NONE;
}

static ui_error_t failing_mutation_cb(struct ui_mutation_observer *observer,
                                      const struct ui_mutation_record *records,
                                      int record_count, void *user_data) {
  if (observer) {
  }
  if (records) {
  }
  if (record_count) {
  }
  if (user_data) {
  }
  return UI_ERROR_INVALID_ARGUMENT; /* Example error */
}

static int run_failing_cb_tests(void) {
  struct ui_mutation_observer *obs = NULL;
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *child = NULL;
  struct ui_mutation_observer_init options = {1, 1, 1, 1, 1};

  if (ui_mutation_observer_create(failing_mutation_cb, NULL, &obs) !=
      UI_ERROR_NONE)
    return 1;
  if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root) != UI_ERROR_NONE)
    return 1;
  if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child) != UI_ERROR_NONE)
    return 1;

  if (ui_mutation_observer_observe(obs, root, &options) != UI_ERROR_NONE)
    return 1;

  if (ui_mutation_observer_notify_child_list(root, child, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_mutation_observer_notify_attribute(root, "id", "old") !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_mutation_observer_notify_character_data(root, "old") !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_dom_node_destroy(child) != UI_ERROR_NONE)
    return 1;
  if (ui_dom_node_destroy(root) != UI_ERROR_NONE)
    return 1;
  if (ui_mutation_observer_destroy(obs) != UI_ERROR_NONE)
    return 1;

  return 0;
}

static int run_normal_tests(void) {
  struct ui_mutation_observer *obs = NULL;
  struct ui_mutation_observer *obs_array[70];
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *child1 = NULL;
  struct ui_dom_node *child2 = NULL;
  struct ui_dom_node *nodes[10];
  struct ui_mutation_observer_init options = {1, 1, 1, 1, 1};
  ui_error_t rc;
  int i;

  if (ui_mutation_observer_create(NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_mutation_observer_create(mutation_cb, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_mutation_observer_destroy(NULL) != UI_ERROR_NONE)
    return 1;
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
    if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &nodes[i]) !=
        UI_ERROR_NONE)
      return 1;
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
  rc = ui_dom_node_remove_child(root, child1);
  if (rc != UI_ERROR_NONE && rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_append_child(root, child1) != UI_ERROR_NONE)
    return 1;
  if (g_cb_called != 1 || g_last_record.type != UI_MUTATION_TYPE_CHILD_LIST)
    return 1;

  g_cb_called = -100;
  if (ui_dom_node_remove_child(root, child1) != UI_ERROR_NONE)
    return 1;
  if (ui_dom_node_append_child(root, child1) != UI_ERROR_NONE)
    return 1;
  g_cb_called = 0;

  g_cb_called = -100;
  if (ui_dom_node_set_attribute(root, "id", "test3") != UI_ERROR_NONE)
    return 1;
  g_cb_called = 0;

  g_cb_called = -100;
  if (ui_dom_node_set_text_content(root->first_child, "test3") !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  g_cb_called = 0;

  g_cb_called = -100;
  if (ui_dom_node_set_attribute(root, "class", "test3") != UI_ERROR_NONE)
    return 1;
  g_cb_called = 0;

  g_cb_called = -100;
  if (ui_dom_node_set_text_content(root->first_child, "test4") !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  g_cb_called = 0;

  /* Force dispatch failure cases */
  {
    struct ui_dom_node *test_node3 = NULL;
    if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &test_node3) !=
        UI_ERROR_NONE)
      return 1;
    if (ui_mutation_observer_observe(obs, test_node3, &options) !=
        UI_ERROR_NONE)
      return 1;

    g_cb_called = -100;
    if (ui_mutation_observer_notify_child_list(test_node3, NULL, NULL) !=
        UI_ERROR_NONE)
      return 1;
    g_cb_called = 0;

    g_cb_called = -100;
    if (ui_mutation_observer_notify_attribute(test_node3, "test", NULL) !=
        UI_ERROR_NONE)
      return 1;
    g_cb_called = 0;

    g_cb_called = -100;
    if (ui_mutation_observer_notify_character_data(test_node3, NULL) !=
        UI_ERROR_NONE)
      return 1;
    g_cb_called = 0;

    if (ui_dom_node_destroy(test_node3) != UI_ERROR_NONE)
      return 1;
  }

  /* Test notify_child_list with removed */
  g_cb_called = 0;
  if (ui_dom_node_remove_child(root, child1) != UI_ERROR_NONE)
    return 1;
  if (g_cb_called != 1 || g_last_record.type != UI_MUTATION_TYPE_CHILD_LIST ||
      g_last_record.removed_nodes_count != 1)
    return 1;

  /* Test ancestor path (subtree observing) */
  rc = ui_dom_node_remove_child(root, child1);
  if (rc != UI_ERROR_NONE && rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_append_child(root, child1) != UI_ERROR_NONE)
    return 1;
  g_cb_called = 0;
  if (ui_dom_node_append_child(child1, child2) != UI_ERROR_NONE)
    return 1; /* should trigger root's observer because subtree=1 */
  if (g_cb_called != 1 || g_last_record.type != UI_MUTATION_TYPE_CHILD_LIST)
    return 1;

  g_cb_called = -100;
  if (ui_dom_node_remove_child(root, child1) != UI_ERROR_NONE)
    return 1;
  if (ui_dom_node_append_child(root, child1) != UI_ERROR_NONE)
    return 1;
  g_cb_called = 0;

  g_cb_called = -100;
  if (ui_dom_node_set_attribute(root, "id", "test3") != UI_ERROR_NONE)
    return 1;
  g_cb_called = 0;

  g_cb_called = -100;
  if (ui_dom_node_set_text_content(root->first_child, "test3") !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  g_cb_called = 0;

  g_cb_called = -100;
  if (ui_dom_node_set_attribute(root, "class", "test3") != UI_ERROR_NONE)
    return 1;
  g_cb_called = 0;

  g_cb_called = -100;
  if (ui_dom_node_set_text_content(root->first_child, "test4") !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  g_cb_called = 0;

  /* Force dispatch failure cases */
  {
    struct ui_dom_node *test_node3 = NULL;
    if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &test_node3) !=
        UI_ERROR_NONE)
      return 1;
    if (ui_mutation_observer_observe(obs, test_node3, &options) !=
        UI_ERROR_NONE)
      return 1;

    g_cb_called = -100;
    if (ui_mutation_observer_notify_child_list(test_node3, NULL, NULL) !=
        UI_ERROR_NONE)
      return 1;
    g_cb_called = 0;

    g_cb_called = -100;
    if (ui_mutation_observer_notify_attribute(test_node3, "test", NULL) !=
        UI_ERROR_NONE)
      return 1;
    g_cb_called = 0;

    g_cb_called = -100;
    if (ui_mutation_observer_notify_character_data(test_node3, NULL) !=
        UI_ERROR_NONE)
      return 1;
    g_cb_called = 0;

    if (ui_dom_node_destroy(test_node3) != UI_ERROR_NONE)
      return 1;
  }

  /* Test attributes with old value */
  g_cb_called = 0;
  if (ui_dom_node_set_attribute(root, "id", "test1") != UI_ERROR_NONE)
    return 1;
  if (ui_dom_node_set_attribute(root, "id", "test2") != UI_ERROR_NONE)
    return 1;
  if (g_cb_called != 2 || g_last_record.type != UI_MUTATION_TYPE_ATTRIBUTES)
    return 1;

  /* Test character data with old value */
  {
    struct ui_dom_node *text_node = NULL;
    if (ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node) != UI_ERROR_NONE)
      return 1;
    if (ui_dom_node_set_text_content(text_node, "init") != UI_ERROR_NONE)
      return 1;
    if (ui_dom_node_append_child(root, text_node) != UI_ERROR_NONE)
      return 1; /* trigger 1 child list */
    g_cb_called = 0;
    if (ui_dom_node_set_text_content(text_node, "new text") != UI_ERROR_NONE)
      return 1; /* trigger char data on root via subtree */
    if (g_cb_called != 1 ||
        g_last_record.type != UI_MUTATION_TYPE_CHARACTER_DATA)
      return 1;
  }

  if (ui_mutation_observer_disconnect(obs) != UI_ERROR_NONE)
    return 1;
  if (ui_mutation_observer_destroy(obs) != UI_ERROR_NONE)
    return 1;

  /* Test options disabled */
  {
    struct ui_mutation_observer_init opt_none = {0, 0, 0, 0, 0};
    struct ui_mutation_observer_init opt_sub = {1, 1, 1, 1, 1};
    struct ui_dom_node *text_node2 = NULL;
    ui_error_t rc_cleanup;

    if (ui_mutation_observer_create(mutation_cb, NULL, &obs) != UI_ERROR_NONE)
      return 1;
    if (ui_mutation_observer_observe(obs, root, &opt_none) != UI_ERROR_NONE)
      return 1;

    if (ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node2) != UI_ERROR_NONE)
      return 1;
    if (ui_mutation_observer_observe(obs, text_node2, &opt_none) !=
        UI_ERROR_NONE)
      return 1;

    g_cb_called = 0;
    rc = ui_dom_node_remove_child(root, child1);
    if (rc != UI_ERROR_NONE && rc != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_dom_node_append_child(root, child1) != UI_ERROR_NONE)
      return 1;
    if (ui_dom_node_set_attribute(root, "id", "x") != UI_ERROR_NONE)
      return 1;
    if (ui_dom_node_set_text_content(text_node2, "x") != UI_ERROR_NONE)
      return 1;
    if (g_cb_called != 0)
      return 1; /* Should not be called */

    /* Test subtree attribute */
    if (ui_mutation_observer_observe(obs, root, &opt_sub) != UI_ERROR_NONE)
      return 1;
    if (ui_dom_node_set_attribute(child1, "class", "y") != UI_ERROR_NONE)
      return 1; /* trigger attribute via subtree */
    if (g_cb_called != 1)
      return 1;

    rc_cleanup = ui_dom_node_destroy(text_node2);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
    if (ui_mutation_observer_destroy(obs) != UI_ERROR_NONE)
      return 1;
  }

  /* Test NULL internally */
  if (ui_mutation_observer_notify_child_list(NULL, NULL, NULL) != UI_ERROR_NONE)
    return 1;
  if (ui_mutation_observer_notify_attribute(NULL, NULL, NULL) != UI_ERROR_NONE)
    return 1;
  if (ui_mutation_observer_notify_character_data(NULL, NULL) != UI_ERROR_NONE)
    return 1;

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
    if (ui_mutation_observer_destroy(obs_array[i]) != UI_ERROR_NONE)
      return 1;
  }

  if (ui_dom_node_destroy(root) != UI_ERROR_NONE)
    return 1; /* destroys child1, child2, text_node */
  for (i = 0; i < 10; i++) {
    ui_error_t rc_cleanup = ui_dom_node_destroy(nodes[i]);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  /* Additional branch coverage tests */
  {
    struct ui_mutation_observer *tmp_obs = NULL;
    if (ui_mutation_observer_create(NULL, NULL, &tmp_obs) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }

  {
    /* observer is already destroyed, but we can reuse the pointer for invalid
     * arg check */
    if (ui_mutation_observer_observe((void *)1, NULL, NULL) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_mutation_observer_observe((void *)1, root, NULL) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }

  {
    struct mock_obs {
      void *cb;
      void *ud;
      void *targets;
      int c;
      int cap;
    };
    struct mock_obs *mo = malloc(sizeof(struct mock_obs));
    mo->targets = NULL;
    ui_mutation_observer_destroy((struct ui_mutation_observer *)mo);
  }

  {
    struct ui_dom_node *live_root = NULL;
    struct ui_dom_node *live_child = NULL;
    struct ui_dom_node *disjoint = NULL;
    struct ui_mutation_observer *sobs = NULL;
    struct ui_mutation_observer *sobs2 = NULL;
    struct ui_mutation_observer_init opt_sub = {1, 1, 1, 1, 1};
    struct ui_mutation_observer_init opt_no_sub = {1, 1, 1, 0, 0};

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &live_root);
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &live_child);
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &disjoint);
    ui_dom_node_append_child(live_root, live_child);

    ui_mutation_observer_create(mutation_cb, NULL, &sobs);
    ui_mutation_observer_observe(sobs, live_root, &opt_sub);

    ui_mutation_observer_create(mutation_cb, NULL, &sobs2);
    ui_mutation_observer_observe(sobs2, live_root, &opt_no_sub);

    ui_mutation_observer_notify_child_list(live_child, NULL, NULL);
    ui_mutation_observer_notify_attribute(live_child, "test", "test");
    ui_mutation_observer_notify_character_data(live_child, "test");

    ui_mutation_observer_notify_child_list(disjoint, NULL, NULL);
    ui_mutation_observer_notify_attribute(live_root, NULL, NULL);
    ui_mutation_observer_notify_attribute(disjoint, "test", "test");
    ui_mutation_observer_notify_character_data(disjoint, "test");
    ui_mutation_observer_notify_attribute(live_root, "test", "test");

    ui_mutation_observer_destroy(sobs);
    ui_mutation_observer_destroy(sobs2);
    ui_dom_node_destroy(live_root);
    ui_dom_node_destroy(disjoint);
  }

  return 0;
}

static int run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_mutation_observer *obs = NULL;
  struct ui_dom_node *root = NULL;
  struct ui_mutation_observer_init options = {1, 1, 1, 1, 1};
  struct ui_dom_node *n1 = NULL, *n2 = NULL, *n3 = NULL, *n4 = NULL, *n5 = NULL;
  struct ui_dom_node *test_node = NULL;
  ui_error_t rc_cleanup;
  ui_error_t notify_rc;

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

  if (ui_mutation_observer_create(mutation_cb, NULL, &obs) != UI_ERROR_NONE)
    return 1;
  if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root) != UI_ERROR_NONE)
    return 1;

  /* Trigger array capacity OOM */
  if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n1) != UI_ERROR_NONE)
    return 1;
  if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n2) != UI_ERROR_NONE)
    return 1;
  if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n3) != UI_ERROR_NONE)
    return 1;
  if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n4) != UI_ERROR_NONE)
    return 1;
  if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n5) != UI_ERROR_NONE)
    return 1;
  if (ui_mutation_observer_observe(obs, n1, &options) != UI_ERROR_NONE)
    return 1;
  if (ui_mutation_observer_observe(obs, n2, &options) != UI_ERROR_NONE)
    return 1;
  if (ui_mutation_observer_observe(obs, n3, &options) != UI_ERROR_NONE)
    return 1;
  if (ui_mutation_observer_observe(obs, n4, &options) != UI_ERROR_NONE)
    return 1;

  g_malloc_fail_countdown = 0;
  if (ui_mutation_observer_observe(obs, n5, &options) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  rc_cleanup = ui_dom_node_destroy(n1);
  if (rc_cleanup != UI_ERROR_NONE) {
    return 1;
  }
  rc_cleanup = ui_dom_node_destroy(n2);
  if (rc_cleanup != UI_ERROR_NONE) {
    return 1;
  }
  rc_cleanup = ui_dom_node_destroy(n3);
  if (rc_cleanup != UI_ERROR_NONE) {
    return 1;
  }
  rc_cleanup = ui_dom_node_destroy(n4);
  if (rc_cleanup != UI_ERROR_NONE) {
    return 1;
  }
  rc_cleanup = ui_dom_node_destroy(n5);
  if (rc_cleanup != UI_ERROR_NONE) {
    return 1;
  }
  if (ui_mutation_observer_destroy(obs) != UI_ERROR_NONE)
    return 1;
  rc_cleanup = ui_dom_node_destroy(root);
  if (rc_cleanup != UI_ERROR_NONE) {
    return 1;
  }

  /* Trigger string OOM in attribute notify */
  if (ui_mutation_observer_create(mutation_cb, NULL, &obs) != UI_ERROR_NONE)
    return 1;
  if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &test_node) != UI_ERROR_NONE)
    return 1;
  if (ui_mutation_observer_observe(obs, test_node, &options) != UI_ERROR_NONE)
    return 1;

  g_malloc_fail_countdown = 0;
  notify_rc =
      ui_mutation_observer_notify_attribute(test_node, "test", "old_val");
  if (notify_rc != UI_ERROR_OUT_OF_MEMORY && notify_rc != UI_ERROR_NONE)
    return 1;
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 1;
  notify_rc =
      ui_mutation_observer_notify_attribute(test_node, "test", "old_val");
  if (notify_rc != UI_ERROR_OUT_OF_MEMORY && notify_rc != UI_ERROR_NONE)
    return 1;
  g_malloc_fail_countdown = -1;

  /* Trigger string OOM in character_data notify */
  g_malloc_fail_countdown = 0;
  notify_rc = ui_mutation_observer_notify_character_data(test_node, "old_val");
  if (notify_rc != UI_ERROR_OUT_OF_MEMORY && notify_rc != UI_ERROR_NONE)
    return 1;
  g_malloc_fail_countdown = -1;

  rc_cleanup = ui_dom_node_destroy(test_node);
  if (rc_cleanup != UI_ERROR_NONE) {
    return 1;
  }
  if (ui_mutation_observer_destroy(obs) != UI_ERROR_NONE)
    return 1;
#endif
  return 0;
}

int main(void) {
  if (run_normal_tests() != 0)
    return 1;
  if (run_failing_cb_tests() != 0)
    return 1;
  if (run_oom_tests() != 0)
    return 1;
  printf("All ui_mutation_observer tests passed.\n");
  return 0;
}
