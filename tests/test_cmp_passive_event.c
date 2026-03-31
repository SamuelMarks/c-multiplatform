/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

static void mock_passive_callback(cmp_event_t *evt, cmp_ui_node_t *node,
                                  void *user_data) {
  (void)evt;
  (void)node;
  (void)user_data;
}

TEST test_passive_listener_init(void) {
  ASSERT_EQ(CMP_SUCCESS, cmp_passive_event_init());
  PASS();
}

TEST test_add_passive_listener(void) {
  cmp_ui_node_t node = {0};

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_node_add_passive_listener(
                             &node, 1, mock_passive_callback, NULL));
  PASS();
}

TEST test_prevent_default(void) {
  cmp_event_t evt = {0};
  cmp_event_t passive_evt = {0};

  /* Normal prevent default */
  ASSERT_EQ(CMP_SUCCESS, cmp_event_prevent_default(&evt));
  ASSERT_EQ(1, cmp_event_is_default_prevented(&evt));

  /* Passive prevent default (failure) */
  passive_evt.modifiers = 0x80000000; /* Mock passive context */
  ASSERT_EQ(CMP_ERROR_BOUNDS, cmp_event_prevent_default(&passive_evt));
  ASSERT_EQ(0, cmp_event_is_default_prevented(&passive_evt));

  PASS();
}

TEST test_passive_event_edge_cases(void) {
  cmp_ui_node_t node = {0};

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_node_add_passive_listener(
                                       NULL, 1, mock_passive_callback, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_node_add_passive_listener(&node, 1, NULL, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_event_prevent_default(NULL));
  ASSERT_EQ(0, cmp_event_is_default_prevented(NULL));

  PASS();
}

SUITE(cmp_passive_event_suite) {
  RUN_TEST(test_passive_listener_init);
  RUN_TEST(test_add_passive_listener);
  RUN_TEST(test_prevent_default);
  RUN_TEST(test_passive_event_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_passive_event_suite);
  GREATEST_MAIN_END();
}
