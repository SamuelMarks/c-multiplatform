/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_pointer_events_set_get(void) {
  cmp_ui_node_t node = {0};
  cmp_pointer_events_t out_events;

  /* Should default to AUTO/0 initially */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_node_get_pointer_events(&node, &out_events));
  ASSERT_EQ(CMP_POINTER_EVENTS_AUTO, out_events);

  /* Set to NONE */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_node_set_pointer_events(&node, CMP_POINTER_EVENTS_NONE));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_node_get_pointer_events(&node, &out_events));
  ASSERT_EQ(CMP_POINTER_EVENTS_NONE, out_events);

  /* Set back to AUTO */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_node_set_pointer_events(&node, CMP_POINTER_EVENTS_AUTO));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_node_get_pointer_events(&node, &out_events));
  ASSERT_EQ(CMP_POINTER_EVENTS_AUTO, out_events);

  PASS();
}

TEST test_pointer_events_edge_cases(void) {
  cmp_pointer_events_t out_events;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_node_set_pointer_events(NULL, CMP_POINTER_EVENTS_NONE));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_node_get_pointer_events(NULL, &out_events));
  PASS();
}

SUITE(cmp_pointer_events_suite) {
  RUN_TEST(test_pointer_events_set_get);
  RUN_TEST(test_pointer_events_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_pointer_events_suite);
  GREATEST_MAIN_END();
}
