#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_f2_icons.h"
/* clang-format on */

TEST test_f2_icon_create(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_icon_t *icon_data = NULL;
  int res;

  res = cmp_f2_icon_create(&node, 0x1F4BE, 0x1F4BF, 24.0f);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);
  ASSERT(node->properties != NULL);

  icon_data = (cmp_f2_icon_t *)node->properties;
  ASSERT_EQ(0x1F4BE, icon_data->codepoint_regular);
  ASSERT_EQ(0x1F4BF, icon_data->codepoint_filled);
  ASSERT_EQ(0, icon_data->is_filled);
  /* Verify layout bounds mapping */
  ASSERT_EQ(24.0f, node->layout->width);
  ASSERT_EQ(24.0f, node->layout->height);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_icon_state_swap(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_icon_t *icon_data = NULL;
  int res;

  res = cmp_f2_icon_create(&node, 'A', 'B', 20.0f);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_icon_set_filled(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  icon_data = (cmp_f2_icon_t *)node->properties;
  ASSERT_EQ(1, icon_data->is_filled);

  /* Test setting it back to regular */
  res = cmp_f2_icon_set_filled(node, 0);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, icon_data->is_filled);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_icon_invalid_args(void) {
  int res;
  cmp_ui_node_t *node = NULL;

  res = cmp_f2_icon_create(NULL, 'A', 'B', 16.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  /* Test node without valid structure mapping - handled implicitly now */
  res = cmp_ui_box_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);
  res = cmp_f2_icon_set_filled(node, 1);
  /* Should return INVALID_ARG since the box doesn't have the icon properties
   * structure mapped */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);
  cmp_ui_node_destroy(node);

  PASS();
}

SUITE(cmp_f2_icons_suite) {
  RUN_TEST(test_f2_icon_create);
  RUN_TEST(test_f2_icon_state_swap);
  RUN_TEST(test_f2_icon_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_f2_icons_suite);
  GREATEST_MAIN_END();
}
