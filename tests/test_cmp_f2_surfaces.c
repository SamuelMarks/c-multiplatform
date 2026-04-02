#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_f2_surfaces.h"
/* clang-format on */

TEST test_f2_card(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_card_t *data = NULL;
  int res;

  res = cmp_f2_card_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);

  data = (cmp_f2_card_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_CARD_VARIANT_FILLED, data->variant);
  ASSERT_EQ(CMP_F2_CARD_ORIENTATION_VERTICAL, data->orientation);
  ASSERT_EQ(0, data->is_clickable);

  res = cmp_f2_card_set_variant(node, CMP_F2_CARD_VARIANT_OUTLINE);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_CARD_VARIANT_OUTLINE, data->variant);

  res = cmp_f2_card_set_orientation(node, CMP_F2_CARD_ORIENTATION_HORIZONTAL);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_CARD_ORIENTATION_HORIZONTAL, data->orientation);
  ASSERT_EQ(CMP_FLEX_ROW, node->layout->direction);

  res = cmp_f2_card_set_clickable(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_clickable);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_accordion(void) {
  cmp_ui_node_t *node = NULL;
  cmp_ui_node_t *panel = NULL;
  cmp_f2_accordion_t *data = NULL;
  cmp_f2_accordion_panel_t *panel_data = NULL;
  int res;

  res = cmp_f2_accordion_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);
  data = (cmp_f2_accordion_t *)node->properties;
  ASSERT_EQ(CMP_F2_ACCORDION_VARIANT_FLUSH, data->variant);
  ASSERT_EQ(0, data->multi_expand);

  res = cmp_f2_accordion_set_variant(node, CMP_F2_ACCORDION_VARIANT_CONTAINED);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_ACCORDION_VARIANT_CONTAINED, data->variant);

  res = cmp_f2_accordion_set_multi_expand(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->multi_expand);

  res = cmp_f2_accordion_panel_create(&panel, "Details");
  ASSERT_EQ(CMP_SUCCESS, res);
  panel_data = (cmp_f2_accordion_panel_t *)panel->properties;
  ASSERT_EQ(0, panel_data->is_expanded);

  res = cmp_f2_accordion_panel_set_expanded(panel, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, panel_data->is_expanded);

  cmp_ui_node_add_child(node, panel);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_divider(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_divider_t *data = NULL;
  int res;

  res = cmp_f2_divider_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);
  data = (cmp_f2_divider_t *)node->properties;
  ASSERT_EQ(CMP_F2_DIVIDER_HORIZONTAL, data->orientation);
  ASSERT_EQ(CMP_F2_DIVIDER_APPEARANCE_DEFAULT, data->appearance);
  ASSERT_EQ(0, data->is_inset);
  ASSERT_EQ(1.0f, node->layout->height);

  res = cmp_f2_divider_set_orientation(node, CMP_F2_DIVIDER_VERTICAL);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_DIVIDER_VERTICAL, data->orientation);
  ASSERT_EQ(1.0f, node->layout->width);

  res = cmp_f2_divider_set_appearance(node, CMP_F2_DIVIDER_APPEARANCE_BRAND);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_DIVIDER_APPEARANCE_BRAND, data->appearance);

  res = cmp_f2_divider_set_inset(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_inset);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_split_view(void) {
  cmp_ui_node_t *node = NULL;
  int res;

  res = cmp_f2_split_view_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_FLEX_ROW, node->layout->direction);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_scroll_view(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_scroll_view_t *data = NULL;
  int res;

  res = cmp_f2_scroll_view_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);
  data = (cmp_f2_scroll_view_t *)node->properties;
  ASSERT_EQ(CMP_F2_SCROLLBAR_BEHAVIOR_AUTO_HIDE, data->behavior);
  ASSERT_EQ(1, data->is_overlay);

  res = cmp_f2_scroll_view_set_behavior(node,
                                        CMP_F2_SCROLLBAR_BEHAVIOR_ALWAYS_ON);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_SCROLLBAR_BEHAVIOR_ALWAYS_ON, data->behavior);

  res = cmp_f2_scroll_view_set_overlay_mode(node, 0);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, data->is_overlay);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_surfaces_invalid_args(void) {
  int res;

  res = cmp_f2_card_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_accordion_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_accordion_panel_create(NULL, "Fail");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_divider_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_split_view_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_scroll_view_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

SUITE(cmp_f2_surfaces_suite) {
  RUN_TEST(test_f2_card);
  RUN_TEST(test_f2_accordion);
  RUN_TEST(test_f2_divider);
  RUN_TEST(test_f2_split_view);
  RUN_TEST(test_f2_scroll_view);
  RUN_TEST(test_f2_surfaces_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_f2_surfaces_suite);
  GREATEST_MAIN_END();
}
