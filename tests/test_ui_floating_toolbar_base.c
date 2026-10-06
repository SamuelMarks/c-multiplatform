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
extern int g_floating_toolbar_mock_fail;
extern int g_floating_toolbar_destroy_mock_fail;
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

  /* Destroy with comp set to NULL branch */
  {
    struct ui_floating_toolbar_base *tb_no_comp = NULL;
    struct ui_component *saved_comp = NULL;
    rc = ui_floating_toolbar_base_create(&tb_no_comp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_floating_toolbar_base_get_component(tb_no_comp, &saved_comp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_component_destroy(saved_comp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    /* Manually zero component in struct for branch coverage */
    *(struct ui_component **)tb_no_comp = NULL;
    rc = ui_floating_toolbar_base_destroy(tb_no_comp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

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

  /* Set Horizontal */
  rc = ui_floating_toolbar_base_set_orientation(tb,
                                                UI_FLOATING_TOOLBAR_HORIZONTAL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_floating_toolbar_base_get_orientation(tb, &orient);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_FLOATING_TOOLBAR_HORIZONTAL, orient);

  /* Set Collapsed */
  rc = ui_floating_toolbar_base_set_state(tb, UI_FLOATING_TOOLBAR_COLLAPSED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_floating_toolbar_base_get_state(tb, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_FLOATING_TOOLBAR_COLLAPSED, state);

  /* Set Expanded */
  rc = ui_floating_toolbar_base_set_state(tb, UI_FLOATING_TOOLBAR_EXPANDED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_floating_toolbar_base_get_state(tb, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_FLOATING_TOOLBAR_EXPANDED, state);

  rc = ui_floating_toolbar_base_get_component(tb, &comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(comp != NULL);

#ifdef UI_TEST_MOCK_ALLOC
  /* Test set_orientation failure via OOM */
  g_malloc_fail_countdown = 0;
  rc = ui_floating_toolbar_base_set_orientation(tb,
                                                UI_FLOATING_TOOLBAR_VERTICAL);
  ASSERT(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  /* Test set_state failure via OOM */
  g_malloc_fail_countdown = 0;
  rc = ui_floating_toolbar_base_set_state(tb, UI_FLOATING_TOOLBAR_COLLAPSED);
  ASSERT(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;
#endif

  rc = ui_floating_toolbar_base_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_ui_floating_toolbar_fab_slot_and_actions(void) {
  struct ui_floating_toolbar_base *tb = NULL;
  struct ui_component *fab_comp = NULL;
  struct ui_component *actions[8];
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

#ifdef UI_TEST_MOCK_ALLOC
  /* Mock mount failure in set_fab_slot */
  g_floating_toolbar_mock_fail = 3;
  rc = ui_floating_toolbar_base_set_fab_slot(tb, fab_comp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_floating_toolbar_mock_fail = 0;
#endif

  /* Append actions beyond initial capacity (4) to trigger reallocation up to
   * capacity 8 */
  for (i = 0; i < 8; ++i) {
    rc = ui_component_create(&actions[i]);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_floating_toolbar_base_append_action(tb, actions[i]);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = ui_floating_toolbar_base_get_action_count(tb, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(8, (int)count);

#ifdef UI_TEST_MOCK_ALLOC
  {
    /* Test realloc failure branch in append_action:
       Currently count is 8, capacity is 8. The next append will attempt
       realloc. */
    struct ui_component *extra_comp = NULL;
    rc = ui_component_create(&extra_comp);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    g_malloc_fail_countdown = 0;
    rc = ui_floating_toolbar_base_append_action(tb, extra_comp);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = -1;

    /* Test mount failure in append_action */
    g_floating_toolbar_mock_fail = 4;
    rc = ui_floating_toolbar_base_append_action(tb, extra_comp);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_floating_toolbar_mock_fail = 0;

    rc = ui_component_destroy(extra_comp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
#endif

  /* Clear FAB slot */
  rc = ui_floating_toolbar_base_set_fab_slot(tb, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Cleanup */
  for (i = 0; i < 8; ++i) {
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
  int i;
  ui_error_t rc;

  /* Test create under OOM countdown loops */
  for (i = 0; i < 40; ++i) {
    struct ui_floating_toolbar_base *tb = NULL;
    g_malloc_fail_countdown = i;
    rc = ui_floating_toolbar_base_create(&tb);
    if (rc == UI_ERROR_NONE) {
      ui_error_t rc_cleanup = ui_floating_toolbar_base_destroy(tb);
      ASSERT_EQ(UI_ERROR_NONE, rc_cleanup);
      break;
    } else {
      ASSERT(rc != UI_ERROR_NONE);
      ASSERT(tb == NULL);
    }
  }
  g_malloc_fail_countdown = -1;

  /* Test mock append_child failure for fab_slot_node (mock 1) */
  {
    struct ui_floating_toolbar_base *tb = NULL;
    g_floating_toolbar_mock_fail = 1;
    rc = ui_floating_toolbar_base_create(&tb);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    ASSERT(tb == NULL);
    g_floating_toolbar_mock_fail = 0;
  }

  /* Test mock append_child failure for actions_container_node (mock 2) */
  {
    struct ui_floating_toolbar_base *tb = NULL;
    g_floating_toolbar_mock_fail = 2;
    rc = ui_floating_toolbar_base_create(&tb);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    ASSERT(tb == NULL);
    g_floating_toolbar_mock_fail = 0;
  }

  /* Test mock destroy failures during destroy */
  {
    struct ui_floating_toolbar_base *tb = NULL;
    rc = ui_floating_toolbar_base_create(&tb);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_floating_toolbar_destroy_mock_fail = 2;
    rc = ui_floating_toolbar_base_destroy(tb);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_floating_toolbar_destroy_mock_fail = 0;
  }

  /* Test mock destroy failures during create cleanup */
  {
    struct ui_floating_toolbar_base *tb = NULL;
    g_floating_toolbar_mock_fail = 1;
    g_floating_toolbar_destroy_mock_fail = 1;
    rc = ui_floating_toolbar_base_create(&tb);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_floating_toolbar_mock_fail = 0;
    g_floating_toolbar_destroy_mock_fail = 0;
  }

  {
    struct ui_floating_toolbar_base *tb = NULL;
    g_floating_toolbar_mock_fail = 2;
    g_floating_toolbar_destroy_mock_fail = 1;
    rc = ui_floating_toolbar_base_create(&tb);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_floating_toolbar_mock_fail = 0;
    g_floating_toolbar_destroy_mock_fail = 0;
  }

  {
    struct ui_floating_toolbar_base *tb = NULL;
    g_floating_toolbar_mock_fail = 1;
    g_floating_toolbar_destroy_mock_fail = 2;
    rc = ui_floating_toolbar_base_create(&tb);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_floating_toolbar_mock_fail = 0;
    g_floating_toolbar_destroy_mock_fail = 0;
  }

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
