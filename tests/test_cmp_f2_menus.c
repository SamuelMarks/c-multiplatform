#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_f2_menus.h"
/* clang-format on */

TEST test_f2_menu(void) {
  cmp_ui_node_t *node = NULL;
  int res;

  res = cmp_f2_menu_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);
  ASSERT_EQ(CMP_FLEX_COLUMN, node->layout->direction);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_menu_item(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_menu_item_t *data = NULL;
  int res;

  res = cmp_f2_menu_item_create(&node, "Copy");
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);

  data = (cmp_f2_menu_item_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(0, data->is_disabled);
  ASSERT(data->text_node != NULL);

  res = cmp_f2_menu_item_set_shortcut(node, "Ctrl+C");
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(data->shortcut_text_node != NULL);

  res = cmp_f2_menu_item_set_disabled(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_disabled);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_nav_view(void) {
  cmp_ui_node_t *view = NULL;
  cmp_ui_node_t *item = NULL;
  cmp_f2_nav_view_t *view_data = NULL;
  cmp_f2_nav_item_t *item_data = NULL;
  int res;

  res = cmp_f2_nav_view_create(&view);
  ASSERT_EQ(CMP_SUCCESS, res);
  view_data = (cmp_f2_nav_view_t *)view->properties;
  ASSERT_EQ(CMP_F2_NAV_STATE_EXPANDED, view_data->state);

  res = cmp_f2_nav_view_set_state(view, CMP_F2_NAV_STATE_COLLAPSED);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_NAV_STATE_COLLAPSED, view_data->state);

  res = cmp_f2_nav_item_create(&item, "Home");
  ASSERT_EQ(CMP_SUCCESS, res);
  item_data = (cmp_f2_nav_item_t *)item->properties;
  ASSERT_EQ(0, item_data->is_active);

  res = cmp_f2_nav_item_set_active(item, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, item_data->is_active);

  cmp_ui_node_destroy(item);
  cmp_ui_node_destroy(view);
  PASS();
}

TEST test_f2_tabs(void) {
  cmp_ui_node_t *container = NULL;
  cmp_ui_node_t *item = NULL;
  cmp_f2_tab_t *data = NULL;
  int res;

  res = cmp_f2_tab_container_create(&container, 0); /* Horizontal */
  ASSERT_EQ(CMP_SUCCESS, res);
  data = (cmp_f2_tab_t *)container->properties;
  ASSERT_EQ(CMP_F2_TAB_VARIANT_TRANSPARENT, data->variant);
  ASSERT_EQ(CMP_F2_TAB_SIZE_MEDIUM, data->size);
  ASSERT_EQ(0, data->is_vertical);

  res = cmp_f2_tab_container_set_variant(container, CMP_F2_TAB_VARIANT_SOLID);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_TAB_VARIANT_SOLID, data->variant);

  res = cmp_f2_tab_container_set_size(container, CMP_F2_TAB_SIZE_LARGE);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_TAB_SIZE_LARGE, data->size);

  res = cmp_f2_tab_item_create(&item, "Tab 1");
  ASSERT_EQ(CMP_SUCCESS, res);
  cmp_ui_node_add_child(container, item);

  cmp_ui_node_destroy(container);
  PASS();
}

TEST test_f2_breadcrumb(void) {
  cmp_ui_node_t *node = NULL;
  cmp_ui_node_t *item1 = NULL;
  cmp_ui_node_t *item2 = NULL;
  int res;

  res = cmp_f2_breadcrumb_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_FLEX_ROW, node->layout->direction);

  res = cmp_f2_breadcrumb_item_create(&item1, "Home", 0);
  ASSERT_EQ(CMP_SUCCESS, res);
  cmp_ui_node_add_child(node, item1);

  res = cmp_f2_breadcrumb_item_create(&item2, "Settings", 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  cmp_ui_node_add_child(node, item2);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_toolbar(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_toolbar_t *data = NULL;
  int res;

  res = cmp_f2_toolbar_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);

  data = (cmp_f2_toolbar_t *)node->properties;
  ASSERT(data != NULL);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_link(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_link_t *data = NULL;
  int res;

  res = cmp_f2_link_create(&node, "Visit Google", CMP_F2_LINK_VARIANT_INLINE);
  ASSERT_EQ(CMP_SUCCESS, res);

  data = (cmp_f2_link_t *)node->properties;
  ASSERT_EQ(CMP_F2_LINK_VARIANT_INLINE, data->variant);
  ASSERT_EQ(0, data->is_disabled);

  res = cmp_f2_link_set_disabled(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_disabled);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_stepper(void) {
  cmp_ui_node_t *node = NULL;
  cmp_ui_node_t *item = NULL;
  cmp_f2_stepper_item_t *data = NULL;
  int res;

  res = cmp_f2_stepper_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_FLEX_ROW, node->layout->direction);

  res = cmp_f2_stepper_item_create(&item, "Step 1");
  ASSERT_EQ(CMP_SUCCESS, res);

  data = (cmp_f2_stepper_item_t *)item->properties;
  ASSERT_EQ(CMP_F2_STEPPER_STATE_UPCOMING, data->state);

  res = cmp_f2_stepper_item_set_state(item, CMP_F2_STEPPER_STATE_CURRENT);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_STEPPER_STATE_CURRENT, data->state);

  cmp_ui_node_add_child(node, item);
  cmp_ui_node_destroy(node);

  PASS();
}

TEST test_f2_menus_invalid_args(void) {
  int res;

  res = cmp_f2_menu_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_nav_view_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_tab_container_create(NULL, 0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_breadcrumb_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_toolbar_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_link_create(NULL, "Fail", CMP_F2_LINK_VARIANT_INLINE);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_stepper_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

SUITE(cmp_f2_menus_suite) {
  RUN_TEST(test_f2_menu);
  RUN_TEST(test_f2_menu_item);
  RUN_TEST(test_f2_nav_view);
  RUN_TEST(test_f2_tabs);
  RUN_TEST(test_f2_breadcrumb);
  RUN_TEST(test_f2_toolbar);
  RUN_TEST(test_f2_link);
  RUN_TEST(test_f2_stepper);
  RUN_TEST(test_f2_menus_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_f2_menus_suite);
  GREATEST_MAIN_END();
}
