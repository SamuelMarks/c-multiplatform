/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

static void mock_event_callback(cmp_event_t *evt, cmp_ui_node_t *node,
                                void *user_data) {
  (void)evt;
  (void)node;
  if (user_data) {
    int *counter = (int *)user_data;
    (*counter)++;
  }
}

typedef struct {
  int call_order[10];
  int count;
} tracking_data_t;

static void ordered_mock_callback(cmp_event_t *evt, cmp_ui_node_t *node,
                                  void *user_data) {
  tracking_data_t *tracker = (tracking_data_t *)user_data;
  (void)evt;
  if (tracker && tracker->count < 10) {
    /* Use the node's type as an ID for this test */
    tracker->call_order[tracker->count++] = node->type;
  }
}

TEST test_event_dispatch_run_success(void) {
  cmp_ui_node_t *tree =
      (cmp_ui_node_t *)(size_t)0xCAFEBABE; /* Mock tree pointer */
  cmp_ui_node_t target_node = {0};
  cmp_event_t evt = {0};

  evt.type = 1; /* Mock MOUSE_DOWN */

  ASSERT_EQ(CMP_SUCCESS, cmp_event_dispatch_run(tree, &target_node, &evt));
  PASS();
}

TEST test_event_dispatch_edge_cases(void) {
  cmp_ui_node_t *tree = (cmp_ui_node_t *)(size_t)0xCAFEBABE;
  cmp_ui_node_t target_node = {0};
  cmp_event_t evt = {0};

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_event_dispatch_run(NULL, &target_node, &evt));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_event_dispatch_run(tree, NULL, &evt));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_event_dispatch_run(tree, &target_node, NULL));
  PASS();
}

TEST test_add_event_listener_success(void) {
  cmp_ui_node_t node = {0};
  int trigger_count = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_node_add_event_listener(
                             &node, 1, 0, mock_event_callback, &trigger_count));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_node_add_event_listener(
                             &node, 1, 1, mock_event_callback, &trigger_count));
  PASS();
}

TEST test_add_event_listener_edge_cases(void) {
  cmp_ui_node_t node = {0};

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_node_add_event_listener(
                                       NULL, 1, 0, mock_event_callback, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_node_add_event_listener(&node, 1, 0, NULL, NULL));
  PASS();
}

TEST test_event_dispatch_phases(void) {
  cmp_ui_node_t root = {0};
  cmp_ui_node_t child = {0};
  cmp_ui_node_t target = {0};
  cmp_event_t evt = {0};
  tracking_data_t tracker = {{0}, 0};

  evt.type = 1;

  root.type = 1;
  child.type = 2;
  target.type = 3;

  target.parent = &child;
  child.parent = &root;

  /* Root capturing */
  cmp_ui_node_add_event_listener(&root, 1, 1, ordered_mock_callback, &tracker);
  /* Root bubbling */
  cmp_ui_node_add_event_listener(&root, 1, 0, ordered_mock_callback, &tracker);
  /* Child capturing */
  cmp_ui_node_add_event_listener(&child, 1, 1, ordered_mock_callback, &tracker);
  /* Child bubbling */
  cmp_ui_node_add_event_listener(&child, 1, 0, ordered_mock_callback, &tracker);
  /* Target */
  cmp_ui_node_add_event_listener(&target, 1, 0, ordered_mock_callback,
                                 &tracker);

  ASSERT_EQ(CMP_SUCCESS, cmp_event_dispatch_run(&root, &target, &evt));

  /* Expected order:
     Capture phase: root (1) -> child (2)
     Target phase: target (3)
     Bubble phase: child (2) -> root (1)
  */
  ASSERT_EQ(5, tracker.count);
  ASSERT_EQ(1, tracker.call_order[0]);
  ASSERT_EQ(2, tracker.call_order[1]);
  ASSERT_EQ(3, tracker.call_order[2]);
  ASSERT_EQ(2, tracker.call_order[3]);
  ASSERT_EQ(1, tracker.call_order[4]);

  PASS();
}

TEST test_event_dispatch_deep_tree_realloc(void) {
  /* Tests the reallocation logic when ancestor_count > 16 */
  cmp_ui_node_t nodes[20];
  cmp_event_t evt = {0};
  int i;
  tracking_data_t tracker = {{0}, 0};

  evt.type = 1;

  for (i = 0; i < 20; i++) {
    memset(&nodes[i], 0, sizeof(cmp_ui_node_t));
    nodes[i].type = i + 1;
    if (i > 0) {
      nodes[i].parent = &nodes[i - 1];
    }
  }

  cmp_ui_node_add_event_listener(&nodes[19], 1, 0, ordered_mock_callback,
                                 &tracker);

  ASSERT_EQ(CMP_SUCCESS, cmp_event_dispatch_run(&nodes[0], &nodes[19], &evt));

  ASSERT_EQ(1, tracker.count);
  ASSERT_EQ(20, tracker.call_order[0]);

  PASS();
}

SUITE(cmp_event_bubbling_suite) {
  RUN_TEST(test_event_dispatch_run_success);
  RUN_TEST(test_event_dispatch_edge_cases);
  RUN_TEST(test_add_event_listener_success);
  RUN_TEST(test_add_event_listener_edge_cases);
  RUN_TEST(test_event_dispatch_phases);
  RUN_TEST(test_event_dispatch_deep_tree_realloc);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_event_bubbling_suite);
  GREATEST_MAIN_END();
}
