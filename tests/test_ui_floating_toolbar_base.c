/**
 * @file test_ui_floating_toolbar_base.c
 * @brief Unit tests and OOM mocks for ui_floating_toolbar_base CDK primitive.
 */

/* clang-format off */
#include "ui_floating_toolbar_base.h"
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

SUITE(ui_floating_toolbar_base_suite);

TEST test_ui_floating_toolbar_create_destroy(void) {
  struct ui_floating_toolbar_base *tb = NULL;
  struct ui_component *comp = NULL;
  const char *role_val = NULL;
  ui_error_t rc;

  /* Null pointer check */
  rc = ui_floating_toolbar_base_create(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_floating_toolbar_base_destroy(NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_floating_toolbar_base_get_component(NULL, &comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Normal creation */
  rc = ui_floating_toolbar_base_create(&tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tb != NULL);

  rc = ui_floating_toolbar_base_get_component(tb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = ui_floating_toolbar_base_get_component(tb, &comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(comp != NULL);
  ASSERT(comp->shadow_root != NULL);

  rc = ui_dom_node_get_attribute(comp->shadow_root, "role", &role_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("toolbar", role_val);

  rc = ui_floating_toolbar_base_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_ui_floating_toolbar_orientation_and_state(void) {
  struct ui_floating_toolbar_base *tb = NULL;
  enum ui_floating_toolbar_orientation orient;
  enum ui_floating_toolbar_state state;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = ui_floating_toolbar_base_create(&tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = ui_floating_toolbar_base_set_orientation(NULL,
                                                UI_FLOATING_TOOLBAR_VERTICAL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_floating_toolbar_base_get_orientation(NULL, &orient);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_floating_toolbar_base_get_orientation(tb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = ui_floating_toolbar_base_set_state(NULL, UI_FLOATING_TOOLBAR_COLLAPSED);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_floating_toolbar_base_get_state(NULL, &state);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_floating_toolbar_base_get_state(tb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Defaults */
  rc = ui_floating_toolbar_base_get_orientation(tb, &orient);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_FLOATING_TOOLBAR_HORIZONTAL, orient);

  rc = ui_floating_toolbar_base_get_state(tb, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_FLOATING_TOOLBAR_EXPANDED, state);

  /* Set Vertical */
  rc = ui_floating_toolbar_base_set_orientation(tb,
                                                UI_FLOATING_TOOLBAR_VERTICAL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_floating_toolbar_base_get_orientation(tb, &orient);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_FLOATING_TOOLBAR_VERTICAL, orient);

  /* Set Collapsed */
  rc = ui_floating_toolbar_base_set_state(tb, UI_FLOATING_TOOLBAR_COLLAPSED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_floating_toolbar_base_get_state(tb, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_FLOATING_TOOLBAR_COLLAPSED, state);

  rc = ui_floating_toolbar_base_get_component(tb, &comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(comp != NULL);

  rc = ui_floating_toolbar_base_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_ui_floating_toolbar_fab_slot_and_actions(void) {
  struct ui_floating_toolbar_base *tb = NULL;
  struct ui_component *fab_comp = NULL;
  struct ui_component *actions[6];
  size_t count = 0;
  size_t i;
  ui_error_t rc;

  rc = ui_floating_toolbar_base_create(&tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = ui_floating_toolbar_base_set_fab_slot(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_floating_toolbar_base_append_action(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_floating_toolbar_base_append_action(tb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_floating_toolbar_base_get_action_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_floating_toolbar_base_get_action_count(tb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Initial action count is 0 */
  rc = ui_floating_toolbar_base_get_action_count(tb, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, (int)count);

  /* Mount FAB slot */
  rc = ui_component_create(&fab_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_floating_toolbar_base_set_fab_slot(tb, fab_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Append actions beyond initial capacity (4) to trigger reallocation */
  for (i = 0; i < 6; ++i) {
    rc = ui_component_create(&actions[i]);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_floating_toolbar_base_append_action(tb, actions[i]);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = ui_floating_toolbar_base_get_action_count(tb, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(6, (int)count);

  /* Clear FAB slot */
  rc = ui_floating_toolbar_base_set_fab_slot(tb, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Cleanup */
  for (i = 0; i < 6; ++i) {
    rc = ui_component_destroy(actions[i]);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = ui_component_destroy(fab_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_floating_toolbar_base_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
TEST test_ui_floating_toolbar_oom(void) {
  struct ui_floating_toolbar_base *tb = NULL;
  ui_error_t rc;

  /* Fail on 1st malloc (struct allocation) */
  g_malloc_fail_countdown = 0;
  rc = ui_floating_toolbar_base_create(&tb);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(tb == NULL);
  g_malloc_fail_countdown = -1;

  PASS();
}
#endif

SUITE(ui_floating_toolbar_base_suite) {
  RUN_TEST(test_ui_floating_toolbar_create_destroy);
  RUN_TEST(test_ui_floating_toolbar_orientation_and_state);
  RUN_TEST(test_ui_floating_toolbar_fab_slot_and_actions);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_ui_floating_toolbar_oom);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_floating_toolbar_base_suite);
  GREATEST_MAIN_END();
}
