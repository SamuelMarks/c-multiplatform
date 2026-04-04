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

SUITE(cmp_event_bubbling_suite) {
  RUN_TEST(test_event_dispatch_run_success);
  RUN_TEST(test_event_dispatch_edge_cases);
  RUN_TEST(test_add_event_listener_success);
  RUN_TEST(test_add_event_listener_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_event_bubbling_suite);
  GREATEST_MAIN_END();
}
