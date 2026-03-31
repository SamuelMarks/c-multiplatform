/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_pointer_capture_init_shutdown(void) {
  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_capture_init());
  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_capture_shutdown());
  PASS();
}

TEST test_set_and_has_capture(void) {
  cmp_ui_node_t node = {0};
  int pointer_id = 42;

  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_capture_init());

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_node_set_pointer_capture(&node, pointer_id));
  ASSERT_EQ(1, cmp_ui_node_has_pointer_capture(&node, pointer_id));

  /* Should fail for different ID */
  ASSERT_EQ(0, cmp_ui_node_has_pointer_capture(&node, 99));

  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_capture_shutdown());
  PASS();
}

TEST test_release_capture(void) {
  cmp_ui_node_t node = {0};
  int pointer_id = 15;

  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_capture_init());

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_node_set_pointer_capture(&node, pointer_id));
  ASSERT_EQ(1, cmp_ui_node_has_pointer_capture(&node, pointer_id));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_node_release_pointer_capture(&node, pointer_id));
  ASSERT_EQ(0, cmp_ui_node_has_pointer_capture(&node, pointer_id));

  /* Releasing already released or unknown pointer */
  ASSERT_EQ(CMP_ERROR_NOT_FOUND,
            cmp_ui_node_release_pointer_capture(&node, pointer_id));

  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_capture_shutdown());
  PASS();
}

TEST test_capture_edge_cases(void) {
  cmp_ui_node_t node = {0};

  /* Operations before initialization */
  ASSERT_EQ(CMP_ERROR_IO, cmp_ui_node_set_pointer_capture(&node, 1));
  ASSERT_EQ(CMP_ERROR_IO, cmp_ui_node_release_pointer_capture(&node, 1));
  ASSERT_EQ(0, cmp_ui_node_has_pointer_capture(&node, 1));

  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_capture_init());

  /* Null/Invalid arguments */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_node_set_pointer_capture(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_node_set_pointer_capture(&node, -1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_node_release_pointer_capture(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_node_release_pointer_capture(&node, -1));

  ASSERT_EQ(0, cmp_ui_node_has_pointer_capture(NULL, 1));
  ASSERT_EQ(0, cmp_ui_node_has_pointer_capture(&node, -1));

  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_capture_shutdown());
  PASS();
}

SUITE(cmp_pointer_capture_suite) {
  RUN_TEST(test_pointer_capture_init_shutdown);
  RUN_TEST(test_set_and_has_capture);
  RUN_TEST(test_release_capture);
  RUN_TEST(test_capture_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_pointer_capture_suite);
  GREATEST_MAIN_END();
}
