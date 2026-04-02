#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_f2_data_display.h"
/* clang-format on */

TEST test_f2_avatar(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_avatar_t *data = NULL;
  int res;

  res = cmp_f2_avatar_create(&node, CMP_F2_AVATAR_SIZE_32,
                             CMP_F2_AVATAR_SHAPE_CIRCULAR);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);

  data = (cmp_f2_avatar_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_AVATAR_SIZE_32, data->size);
  ASSERT_EQ(CMP_F2_AVATAR_SHAPE_CIRCULAR, data->shape);
  ASSERT_EQ(32.0f, node->layout->width);
  ASSERT_EQ(32.0f, node->layout->height);

  res = cmp_f2_avatar_set_initials(node, "JD");
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(data->initials_node != NULL);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_avatar_group(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_avatar_group_t *data = NULL;
  int res;

  res =
      cmp_f2_avatar_group_create(&node, CMP_F2_AVATAR_GROUP_LAYOUT_STACKED, 5);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);

  data = (cmp_f2_avatar_group_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_AVATAR_GROUP_LAYOUT_STACKED, data->layout);
  ASSERT_EQ(5, data->max_count);
  ASSERT_EQ(CMP_FLEX_ROW, node->layout->direction);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_presence_badge(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_presence_badge_t *data = NULL;
  int res;

  res = cmp_f2_presence_badge_create(&node, CMP_F2_PRESENCE_AVAILABLE);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);

  data = (cmp_f2_presence_badge_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_PRESENCE_AVAILABLE, data->status);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_badge(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_badge_t *data = NULL;
  int res;

  res = cmp_f2_badge_create(&node, "99+");
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);

  data = (cmp_f2_badge_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_BADGE_VARIANT_FILLED, data->variant);
  ASSERT_EQ(CMP_F2_BADGE_SHAPE_ROUNDED, data->shape);
  ASSERT(data->text_node != NULL);

  res = cmp_f2_badge_set_variant(node, CMP_F2_BADGE_VARIANT_GHOST);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_BADGE_VARIANT_GHOST, data->variant);

  res = cmp_f2_badge_set_shape(node, CMP_F2_BADGE_SHAPE_CIRCULAR);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_BADGE_SHAPE_CIRCULAR, data->shape);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_tag(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_tag_t *data = NULL;
  int res;

  res = cmp_f2_tag_create(&node, "React", 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);

  data = (cmp_f2_tag_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(1, data->is_dismissible);
  ASSERT_EQ(0, data->is_selected);
  ASSERT(data->text_node != NULL);

  res = cmp_f2_tag_set_selected(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_selected);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_datagrid(void) {
  cmp_ui_node_t *grid = NULL;
  cmp_ui_node_t *header_row = NULL;
  cmp_ui_node_t *data_row = NULL;
  cmp_f2_datagrid_t *data = NULL;
  int res;

  res = cmp_f2_datagrid_create(&grid);
  ASSERT_EQ(CMP_SUCCESS, res);
  data = (cmp_f2_datagrid_t *)grid->properties;
  ASSERT(data != NULL);

  res = cmp_f2_datagrid_row_create(&header_row, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_FLEX_ROW, header_row->layout->direction);

  res = cmp_f2_datagrid_row_create(&data_row, 0);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_FLEX_ROW, data_row->layout->direction);

  cmp_ui_node_add_child(grid, header_row);
  cmp_ui_node_add_child(grid, data_row);

  cmp_ui_node_destroy(grid);
  PASS();
}

TEST test_f2_tree(void) {
  cmp_ui_node_t *tree = NULL;
  cmp_ui_node_t *item = NULL;
  cmp_f2_tree_item_t *data = NULL;
  int res;

  res = cmp_f2_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_FLEX_COLUMN, tree->layout->direction);

  res = cmp_f2_tree_item_create(&item, "Documents");
  ASSERT_EQ(CMP_SUCCESS, res);
  data = (cmp_f2_tree_item_t *)item->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(0, data->is_expanded);
  ASSERT_EQ(0, data->is_selected);
  ASSERT(data->content_node != NULL);

  res = cmp_f2_tree_item_set_expanded(item, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_expanded);

  res = cmp_f2_tree_item_set_selected(item, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_selected);

  cmp_ui_node_add_child(tree, item);
  cmp_ui_node_destroy(tree);
  PASS();
}

TEST test_f2_data_display_invalid_args(void) {
  int res;

  res = cmp_f2_avatar_create(NULL, CMP_F2_AVATAR_SIZE_32,
                             CMP_F2_AVATAR_SHAPE_CIRCULAR);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_avatar_group_create(NULL, CMP_F2_AVATAR_GROUP_LAYOUT_STACKED, 5);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_presence_badge_create(NULL, CMP_F2_PRESENCE_AVAILABLE);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_badge_create(NULL, "New");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_tag_create(NULL, "Tag", 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_datagrid_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_datagrid_row_create(NULL, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_tree_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_tree_item_create(NULL, "Node");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

SUITE(cmp_f2_data_display_suite) {
  RUN_TEST(test_f2_avatar);
  RUN_TEST(test_f2_avatar_group);
  RUN_TEST(test_f2_presence_badge);
  RUN_TEST(test_f2_badge);
  RUN_TEST(test_f2_tag);
  RUN_TEST(test_f2_datagrid);
  RUN_TEST(test_f2_tree);
  RUN_TEST(test_f2_data_display_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_f2_data_display_suite);
  GREATEST_MAIN_END();
}
