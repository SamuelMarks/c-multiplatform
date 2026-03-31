/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

static void mock_context_menu_cb(cmp_context_menu_t *menu, cmp_ui_node_t *node,
                                 float x, float y, void *user_data) {
  (void)menu;
  (void)node;
  (void)x;
  (void)y;
  if (user_data) {
    int *trigger_count = (int *)user_data;
    (*trigger_count)++;
  }
}

TEST test_context_menu_create_destroy(void) {
  cmp_context_menu_t *menu = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_context_menu_create(&menu));
  ASSERT_NEQ(NULL, menu);
  ASSERT_EQ(CMP_SUCCESS, cmp_context_menu_destroy(menu));

  PASS();
}

TEST test_context_menu_trigger_logic(void) {
  cmp_context_menu_t *menu = NULL;
  cmp_event_t event_normal = {0};
  cmp_event_t event_right_click = {0};
  cmp_event_t event_long_press = {0};
  int trigger_count = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_context_menu_create(&menu));
  ASSERT_EQ(CMP_SUCCESS, cmp_context_menu_set_callback(
                             menu, mock_context_menu_cb, &trigger_count));

  event_normal.action = CMP_ACTION_DOWN;
  event_normal.modifiers = 0x0; /* No right click / long press */

  event_right_click.action = CMP_ACTION_DOWN;
  event_right_click.modifiers = 0x1; /* Simulated right click */

  event_long_press.action = CMP_ACTION_DOWN;
  event_long_press.modifiers = 0x2; /* Simulated long press */

  /* Normal event should not trigger the context menu */
  ASSERT_EQ(CMP_SUCCESS, cmp_context_menu_process_event(menu, &event_normal));

  /* Mock handlers return 1 on successful trigger */
  ASSERT_EQ(1, cmp_context_menu_process_event(menu, &event_right_click));
  ASSERT_EQ(1, cmp_context_menu_process_event(menu, &event_long_press));

  ASSERT_EQ(CMP_SUCCESS, cmp_context_menu_destroy(menu));
  PASS();
}

TEST test_context_menu_edge_cases(void) {
  cmp_context_menu_t *menu = NULL;
  cmp_event_t event = {0};

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_context_menu_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_context_menu_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_context_menu_create(&menu));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_context_menu_set_callback(NULL, mock_context_menu_cb, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_context_menu_set_callback(menu, NULL, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_context_menu_process_event(NULL, &event));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_context_menu_process_event(menu, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_context_menu_destroy(menu));
  PASS();
}

SUITE(cmp_context_menu_suite) {
  RUN_TEST(test_context_menu_create_destroy);
  RUN_TEST(test_context_menu_trigger_logic);
  RUN_TEST(test_context_menu_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_context_menu_suite);
  GREATEST_MAIN_END();
}
