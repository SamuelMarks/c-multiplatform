/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_a11y_action_lifecycle(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_a11y_action_t *action = NULL;
  int res;

  cmp_a11y_tree_create(&tree);

  res = cmp_a11y_action_create(tree, &action);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, action);

  res = cmp_a11y_action_destroy(action);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_a11y_action_null_args(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_a11y_action_t *action = NULL;
  int res;

  cmp_a11y_tree_create(&tree);

  res = cmp_a11y_action_create(NULL, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_action_create(tree, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_action_create(NULL, &action);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_action_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_a11y_action_create(tree, &action);

  res = cmp_a11y_action_execute(NULL, 1, CMP_A11Y_ACTION_CLICK);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_action_execute(action, -1, CMP_A11Y_ACTION_CLICK);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_action_execute(action, 1, 999);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_a11y_action_destroy(action);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_a11y_action_execute_events(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_a11y_action_t *action = NULL;
  cmp_event_t evt;
  int res;

  cmp_event_system_init();
  cmp_a11y_tree_create(&tree);
  cmp_a11y_action_create(tree, &action);

  /* Test Click routes into event queue */
  res = cmp_a11y_action_execute(action, 10, CMP_A11Y_ACTION_CLICK);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_event_pop(&evt);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_ACTION_DOWN, evt.action);
  ASSERT_EQ(10, evt.source_id);

  res = cmp_event_pop(&evt);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_ACTION_UP, evt.action);
  ASSERT_EQ(10, evt.source_id);

  /* Test Scroll Forward */
  res = cmp_a11y_action_execute(action, 20, CMP_A11Y_ACTION_SCROLL_FORWARD);
  ASSERT_EQ(CMP_SUCCESS, res);
  res = cmp_event_pop(&evt);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_ACTION_MOVE, evt.action);
  ASSERT_EQ(-50, evt.y);

  /* Test Focus */
  res = cmp_a11y_action_execute(action, 30, CMP_A11Y_ACTION_FOCUS);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(30, cmp_event_get_focus());

  /* Test Blur */
  res = cmp_a11y_action_execute(action, 30, CMP_A11Y_ACTION_BLUR);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(-1, cmp_event_get_focus());

  cmp_a11y_action_destroy(action);
  cmp_a11y_tree_destroy(tree);
  cmp_event_system_shutdown();
  PASS();
}

SUITE(a11y_action_suite) {
  RUN_TEST(test_a11y_action_lifecycle);
  RUN_TEST(test_a11y_action_null_args);
  RUN_TEST(test_a11y_action_execute_events);
}

#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(a11y_action_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
