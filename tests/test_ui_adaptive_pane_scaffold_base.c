/**
 * @file test_ui_adaptive_pane_scaffold_base.c
 * @brief Unit tests and OOM mocks for ui_adaptive_pane_scaffold_base CDK
 * primitive.
 */

/* clang-format off */
#include "ui_adaptive_pane_scaffold_base.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include "greatest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

SUITE(ui_adaptive_pane_scaffold_base_suite);

TEST test_ui_adaptive_pane_scaffold_create_destroy(void) {
  struct ui_adaptive_pane_scaffold_base *scaffold = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  /* Null checks */
  rc = ui_adaptive_pane_scaffold_base_create(UI_ADAPTIVE_SCAFFOLD_LIST_DETAIL,
                                             NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_adaptive_pane_scaffold_base_destroy(NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_adaptive_pane_scaffold_base_get_component(NULL, &comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* List detail create */
  rc = ui_adaptive_pane_scaffold_base_create(UI_ADAPTIVE_SCAFFOLD_LIST_DETAIL,
                                             &scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scaffold != NULL);

  rc = ui_adaptive_pane_scaffold_base_get_component(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = ui_adaptive_pane_scaffold_base_get_component(scaffold, &comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(comp != NULL);
  ASSERT(comp->shadow_root != NULL);

  rc = ui_adaptive_pane_scaffold_base_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Supporting pane create */
  rc = ui_adaptive_pane_scaffold_base_create(
      UI_ADAPTIVE_SCAFFOLD_SUPPORTING_PANE, &scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_adaptive_pane_scaffold_base_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_ui_adaptive_pane_scaffold_breakpoints(void) {
  struct ui_adaptive_pane_scaffold_base *scaffold = NULL;
  enum ui_adaptive_window_width_class wc;
  ui_error_t rc;

  rc = ui_adaptive_pane_scaffold_base_create(UI_ADAPTIVE_SCAFFOLD_LIST_DETAIL,
                                             &scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = ui_adaptive_pane_scaffold_base_set_window_width_class(
      NULL, UI_WINDOW_WIDTH_COMPACT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_adaptive_pane_scaffold_base_get_window_width_class(NULL, &wc);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_adaptive_pane_scaffold_base_get_window_width_class(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Default is expanded */
  rc = ui_adaptive_pane_scaffold_base_get_window_width_class(scaffold, &wc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_WINDOW_WIDTH_EXPANDED, wc);

  /* Set compact */
  rc = ui_adaptive_pane_scaffold_base_set_window_width_class(
      scaffold, UI_WINDOW_WIDTH_COMPACT);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_adaptive_pane_scaffold_base_get_window_width_class(scaffold, &wc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_WINDOW_WIDTH_COMPACT, wc);

  /* Set medium */
  rc = ui_adaptive_pane_scaffold_base_set_window_width_class(
      scaffold, UI_WINDOW_WIDTH_MEDIUM);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_adaptive_pane_scaffold_base_get_window_width_class(scaffold, &wc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_WINDOW_WIDTH_MEDIUM, wc);

  rc = ui_adaptive_pane_scaffold_base_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_ui_adaptive_pane_scaffold_panes(void) {
  struct ui_adaptive_pane_scaffold_base *scaffold = NULL;
  struct ui_component *p_comp = NULL;
  struct ui_component *s_comp = NULL;
  struct ui_component *sup_comp = NULL;
  struct ui_component *ret_comp = NULL;
  enum ui_adaptive_pane_role active_role;
  int levitated = 0;
  ui_error_t rc;

  rc = ui_adaptive_pane_scaffold_base_create(UI_ADAPTIVE_SCAFFOLD_LIST_DETAIL,
                                             &scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc =
      ui_adaptive_pane_scaffold_base_set_pane(NULL, UI_PANE_ROLE_PRIMARY, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_adaptive_pane_scaffold_base_get_pane(NULL, UI_PANE_ROLE_PRIMARY,
                                               &ret_comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_adaptive_pane_scaffold_base_get_pane(scaffold, UI_PANE_ROLE_PRIMARY,
                                               NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_adaptive_pane_scaffold_base_set_active_pane(NULL,
                                                      UI_PANE_ROLE_PRIMARY);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_adaptive_pane_scaffold_base_get_active_pane(NULL, &active_role);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_adaptive_pane_scaffold_base_get_active_pane(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_adaptive_pane_scaffold_base_set_dialog_levitation(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_adaptive_pane_scaffold_base_is_dialog_levitated(NULL, &levitated);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_adaptive_pane_scaffold_base_is_dialog_levitated(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create pane components */
  rc = ui_component_create(&p_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_create(&s_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_create(&sup_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Set panes */
  rc = ui_adaptive_pane_scaffold_base_set_pane(scaffold, UI_PANE_ROLE_PRIMARY,
                                               p_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_adaptive_pane_scaffold_base_set_pane(scaffold, UI_PANE_ROLE_SECONDARY,
                                               s_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_adaptive_pane_scaffold_base_set_pane(
      scaffold, UI_PANE_ROLE_SUPPORTING, sup_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Retrieve panes */
  rc = ui_adaptive_pane_scaffold_base_get_pane(scaffold, UI_PANE_ROLE_PRIMARY,
                                               &ret_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(p_comp, ret_comp);

  rc = ui_adaptive_pane_scaffold_base_get_pane(scaffold, UI_PANE_ROLE_SECONDARY,
                                               &ret_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(s_comp, ret_comp);

  rc = ui_adaptive_pane_scaffold_base_get_pane(
      scaffold, UI_PANE_ROLE_SUPPORTING, &ret_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(sup_comp, ret_comp);

  /* Active pane */
  rc = ui_adaptive_pane_scaffold_base_get_active_pane(scaffold, &active_role);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_PANE_ROLE_PRIMARY, active_role);

  rc = ui_adaptive_pane_scaffold_base_set_active_pane(scaffold,
                                                      UI_PANE_ROLE_SECONDARY);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_adaptive_pane_scaffold_base_get_active_pane(scaffold, &active_role);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_PANE_ROLE_SECONDARY, active_role);

  /* Dialog levitation */
  rc = ui_adaptive_pane_scaffold_base_is_dialog_levitated(scaffold, &levitated);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, levitated);

  rc = ui_adaptive_pane_scaffold_base_set_dialog_levitation(scaffold, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_adaptive_pane_scaffold_base_is_dialog_levitated(scaffold, &levitated);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, levitated);

  /* Cleanup */
  rc = ui_component_destroy(p_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(s_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(sup_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_adaptive_pane_scaffold_base_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
TEST test_ui_adaptive_pane_scaffold_oom(void) {
  struct ui_adaptive_pane_scaffold_base *scaffold = NULL;
  ui_error_t rc;

  g_malloc_fail_countdown = 0;
  rc = ui_adaptive_pane_scaffold_base_create(UI_ADAPTIVE_SCAFFOLD_LIST_DETAIL,
                                             &scaffold);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(scaffold == NULL);
  g_malloc_fail_countdown = -1;

  PASS();
}
#endif

SUITE(ui_adaptive_pane_scaffold_base_suite) {
  RUN_TEST(test_ui_adaptive_pane_scaffold_create_destroy);
  RUN_TEST(test_ui_adaptive_pane_scaffold_breakpoints);
  RUN_TEST(test_ui_adaptive_pane_scaffold_panes);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_ui_adaptive_pane_scaffold_oom);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_adaptive_pane_scaffold_base_suite);
  GREATEST_MAIN_END();
}
