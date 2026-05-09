/* clang-format off */
#include "greatest.h"
#include "cmp_ui_app_bar.h"
#include "cmp_ui_navigation_rail.h"
#include "cmp_ui_modal_drawer.h"
#include "cmp_ui_tabs.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_app_bar(void) {
  cmp_ui_app_bar_t *bar;
  cmp_ui_node_t *node;
  cmp_ui_node_t *action_node;
  cmp_a11y_tree_t *tree;

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_ui_app_bar_create(NULL, CMP_UI_APP_BAR_PLACEMENT_TOP),
                "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_app_bar_create(&bar, CMP_UI_APP_BAR_PLACEMENT_TOP),
                "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_app_bar_get_node(NULL, &node),
                "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_app_bar_get_node(bar, NULL),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_app_bar_get_node(bar, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_ui_app_bar_set_title(NULL, "My Title"), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_app_bar_set_title(bar, "My Title"), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_app_bar_set_title(bar, NULL), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_box_create(&action_node), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_ui_app_bar_add_action(NULL, action_node), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_app_bar_add_action(bar, NULL),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_app_bar_add_action(bar, action_node), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_a11y_tree_create(&tree), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_app_bar_bind_a11y(NULL, tree),
                "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_app_bar_bind_a11y(bar, NULL),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_app_bar_bind_a11y(bar, tree), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_a11y_tree_destroy(tree), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_app_bar_destroy(NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_app_bar_destroy(bar), "%d");
  PASS();
}

TEST test_navigation_rail(void) {
  cmp_ui_navigation_rail_t *rail;
  cmp_ui_node_t *node;
  int idx1, idx2;

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_navigation_rail_create(&rail), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_navigation_rail_get_node(rail, &node),
                "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(
      CMP_SUCCESS,
      cmp_ui_navigation_rail_add_destination(rail, "home", "Home", &idx1),
      "%d");
  ASSERT_EQ_FMT(
      CMP_SUCCESS,
      cmp_ui_navigation_rail_add_destination(rail, "settings", NULL, &idx2),
      "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_navigation_rail_set_selected(rail, idx1),
                "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_navigation_rail_destroy(rail), "%d");
  PASS();
}

TEST test_modal_drawer(void) {
  cmp_ui_modal_drawer_t *drawer;
  cmp_ui_node_t *node;
  int idx1, idx2;

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_modal_drawer_create(&drawer), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_modal_drawer_get_node(drawer, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_modal_drawer_set_open(drawer, 1), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_modal_drawer_set_open(drawer, 0), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_modal_drawer_add_item(drawer, "inbox", "Inbox", &idx1),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_modal_drawer_add_item(drawer, "outbox", "Outbox", &idx2),
                "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_modal_drawer_set_selected(drawer, idx1),
                "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_modal_drawer_destroy(drawer), "%d");
  PASS();
}

TEST test_tabs(void) {
  cmp_ui_tabs_t *tabs;
  cmp_ui_node_t *node;
  int idx1, idx2;

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_tabs_create(&tabs), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_tabs_get_node(tabs, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_tabs_add_tab(tabs, "Tab 1", &idx1), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_tabs_add_tab(tabs, "Tab 2", &idx2), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_tabs_set_selected(tabs, idx1), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_tabs_destroy(tabs), "%d");
  PASS();
}

SUITE(m3_navigation_suite) {
  RUN_TEST(test_app_bar);
  RUN_TEST(test_navigation_rail);
  RUN_TEST(test_modal_drawer);
  RUN_TEST(test_tabs);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(m3_navigation_suite);
  GREATEST_MAIN_END();
}
