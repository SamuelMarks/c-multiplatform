/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_touch_action_set_get(void) {
  cmp_ui_node_t node = {0};
  int val;

  /* Validate default */
  val = cmp_ui_node_get_touch_action(&node);
  ASSERT_EQ((int)CMP_TOUCH_ACTION_AUTO, val);

  /* Validate single flags */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_node_set_touch_action(
                             &node, (uint32_t)CMP_TOUCH_ACTION_NONE));
  val = cmp_ui_node_get_touch_action(&node);
  ASSERT_EQ((int)CMP_TOUCH_ACTION_NONE, val);

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_node_set_touch_action(
                             &node, (uint32_t)CMP_TOUCH_ACTION_PAN_X));
  val = cmp_ui_node_get_touch_action(&node);
  ASSERT_EQ((int)CMP_TOUCH_ACTION_PAN_X, val);

  /* Validate bitwise combinations */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_node_set_touch_action(
                             &node, (uint32_t)CMP_TOUCH_ACTION_PAN_X |
                                        (uint32_t)CMP_TOUCH_ACTION_PINCH_ZOOM));
  val = cmp_ui_node_get_touch_action(&node);
  ASSERT_EQ(((int)CMP_TOUCH_ACTION_PAN_X | (int)CMP_TOUCH_ACTION_PINCH_ZOOM),
            val);

  PASS();
}

TEST test_touch_action_edge_cases(void) {
  cmp_ui_node_t node = {0};

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_node_set_touch_action(
                                       NULL, (uint32_t)CMP_TOUCH_ACTION_PAN_Y));

  ASSERT_EQ((int)CMP_TOUCH_ACTION_AUTO, cmp_ui_node_get_touch_action(NULL));

  (void)node; /* Suppress unused variable warning */
  PASS();
}

SUITE(cmp_touch_action_suite) {
  RUN_TEST(test_touch_action_set_get);
  RUN_TEST(test_touch_action_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_touch_action_suite);
  GREATEST_MAIN_END();
}
