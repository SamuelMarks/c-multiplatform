/**
 * @file test_cupertino_desktop_text_selection_toolbar.c
 * @brief Unit tests for macOS Desktop Text Selection Toolbar conforming to
 * Apple HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_desktop_text_selection_toolbar.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

TEST test_desktop_text_toolbar_lifecycle_and_items(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_desktop_text_selection_toolbar_descriptor desc;
  struct cupertino_desktop_text_selection_toolbar *tb = NULL;
  const char *title = NULL;
  const char *shortcut = NULL;
  int action_id = 0;
  int is_disabled = 0;
  size_t count = 0;
  float x = 0.0f;
  float y = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  int is_vis = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.can_cut = 1;
  desc.can_copy = 1;
  desc.can_paste = 0;
  desc.can_select_all = 1;

  /* Null checks */
  rc = cupertino_desktop_text_selection_toolbar_create(NULL, &desc, &tb);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_desktop_text_selection_toolbar_create(dummy_engine, NULL, &tb);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_desktop_text_selection_toolbar_create(dummy_engine, &desc,
                                                       NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_cupertino_desktop_text_mock_test_helpers;
    int k;
    for (k = 1; k <= 5; k++) {
      g_cupertino_desktop_text_mock_test_helpers = k;
      rc = cupertino_desktop_text_selection_toolbar_create(dummy_engine, &desc,
                                                           &tb);
      if (k == 4) {
        ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
      } else {
        ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
      }
      ASSERT_EQ(NULL, tb);
    }
    g_cupertino_desktop_text_mock_test_helpers = 0;
  }
#endif

  rc =
      cupertino_desktop_text_selection_toolbar_create(dummy_engine, &desc, &tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, tb);

  rc = cupertino_desktop_text_selection_toolbar_get_item_count(tb, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(6, count);

  /* Check Cut */
  rc = cupertino_desktop_text_selection_toolbar_get_item_at(
      tb, 0, &title, &shortcut, &action_id, &is_disabled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Cut", title);
  ASSERT_STR_EQ("Cmd+X", shortcut);
  ASSERT_EQ(0, is_disabled);

  /* Check Paste (disabled in desc) */
  rc = cupertino_desktop_text_selection_toolbar_get_item_at(
      tb, 2, &title, &shortcut, &action_id, &is_disabled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Paste", title);
  ASSERT_EQ(1, is_disabled);

  /* Show / Hide */
  rc = cupertino_desktop_text_selection_toolbar_is_visible(tb, &is_vis);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_vis);

  rc = cupertino_desktop_text_selection_toolbar_show(tb, 150.0f, 250.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_desktop_text_selection_toolbar_is_visible(tb, &is_vis);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_vis);

  rc = cupertino_desktop_text_selection_toolbar_get_bounds(tb, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(150.0f, x, 0.01f);
  ASSERT_IN_RANGE(250.0f, y, 0.01f);
  ASSERT_IN_RANGE(CUPERTINO_DESKTOP_TEXT_TOOLBAR_WIDTH, w, 0.01f);
  ASSERT_GT(h, 100.0f);

  rc = cupertino_desktop_text_selection_toolbar_hide(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_desktop_text_selection_toolbar_is_visible(tb, &is_vis);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_vis);

  rc = cupertino_desktop_text_selection_toolbar_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_desktop_text_selection_toolbar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_desktop_text_toolbar_hover_and_actions(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_desktop_text_selection_toolbar_descriptor desc;
  struct cupertino_desktop_text_selection_toolbar *tb = NULL;
  int h_idx = 0;
  int handled = 0;
  int is_vis = 0;
  size_t count = 0;
  float x = 0.0f;
  float y = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.can_cut = 1;
  desc.can_copy = 1;
  desc.can_paste = 0; /* Disabled */
  desc.can_select_all = 1;

  rc =
      cupertino_desktop_text_selection_toolbar_create(dummy_engine, &desc, &tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Hover */
  rc = cupertino_desktop_text_selection_toolbar_set_hover_index(tb, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_desktop_text_selection_toolbar_get_hover_index(tb, &h_idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, h_idx);

  /* Invalid hover */
  rc = cupertino_desktop_text_selection_toolbar_set_hover_index(tb, 99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_desktop_text_selection_toolbar_set_hover_index(tb, -2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Clear hover */
  rc = cupertino_desktop_text_selection_toolbar_set_hover_index(tb, -1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Show and trigger active action */
  rc = cupertino_desktop_text_selection_toolbar_show(tb, 10.0f, 20.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_desktop_text_selection_toolbar_trigger_action(tb, 1, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, handled);
  /* Should auto-dismiss */
  rc = cupertino_desktop_text_selection_toolbar_is_visible(tb, &is_vis);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_vis);

  /* Trigger disabled action (Paste = 2) */
  rc = cupertino_desktop_text_selection_toolbar_show(tb, 10.0f, 20.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_desktop_text_selection_toolbar_trigger_action(tb, 2, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, handled);
  /* Stays visible */
  rc = cupertino_desktop_text_selection_toolbar_is_visible(tb, &is_vis);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_vis);

  /* Null checks */
  rc = cupertino_desktop_text_selection_toolbar_show(NULL, 10.0f, 20.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_desktop_text_selection_toolbar_hide(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_desktop_text_selection_toolbar_is_visible(NULL, &is_vis);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_desktop_text_selection_toolbar_is_visible(tb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_desktop_text_selection_toolbar_set_hover_index(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_desktop_text_selection_toolbar_get_hover_index(NULL, &h_idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_desktop_text_selection_toolbar_get_hover_index(tb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_desktop_text_selection_toolbar_trigger_action(NULL, 1,
                                                               &handled);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_desktop_text_selection_toolbar_trigger_action(tb, 1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_desktop_text_selection_toolbar_get_item_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_desktop_text_selection_toolbar_get_item_count(tb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_desktop_text_selection_toolbar_get_item_at(NULL, 0, NULL, NULL,
                                                            NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_desktop_text_selection_toolbar_get_item_at(tb, 99, NULL, NULL,
                                                            NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      cupertino_desktop_text_selection_toolbar_get_bounds(NULL, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_desktop_text_selection_toolbar_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_desktop_text_toolbar_oom(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_desktop_text_selection_toolbar_descriptor desc;
  struct cupertino_desktop_text_selection_toolbar *tb = NULL;
  float x, y, w, h;
  int out_act, out_dis;
  const char *out_t, *out_s;
  ui_error_t rc;
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_desktop_text_mock_add_item_fail_at;
  extern int g_cupertino_desktop_text_mock_recompute_fail;
  int i;
#endif

  memset(&desc, 0, sizeof(desc));
  /* Test opposite boolean branches for all can_* flags */
  desc.can_cut = 0;
  desc.can_copy = 0;
  desc.can_paste = 1;
  desc.can_select_all = 0;
  rc =
      cupertino_desktop_text_selection_toolbar_create(dummy_engine, &desc, &tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, tb);

  /* Test get_item_at partial output pointer branches */
  rc = cupertino_desktop_text_selection_toolbar_get_item_at(tb, 0, NULL, NULL,
                                                            NULL, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_desktop_text_selection_toolbar_get_item_at(
      tb, 0, &out_t, &out_s, &out_act, &out_dis);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test get_bounds null arg branches */
  rc =
      cupertino_desktop_text_selection_toolbar_get_bounds(tb, NULL, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      cupertino_desktop_text_selection_toolbar_get_bounds(tb, &x, NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      cupertino_desktop_text_selection_toolbar_get_bounds(tb, &x, &y, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      cupertino_desktop_text_selection_toolbar_get_bounds(tb, &x, &y, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Trigger unhandled action */
  {
    int handled = -1;
    rc = cupertino_desktop_text_selection_toolbar_trigger_action(tb, 999,
                                                                 &handled);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(0, handled);
  }

  rc = cupertino_desktop_text_selection_toolbar_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  tb = NULL;

  g_malloc_fail_countdown = 0;
  rc =
      cupertino_desktop_text_selection_toolbar_create(dummy_engine, &desc, &tb);
  g_malloc_fail_countdown = -1;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, tb);

#ifdef UI_TEST_MOCK_ALLOC
  /* Test failures for each of the 6 item additions */
  for (i = 0; i < 6; i++) {
    g_cupertino_desktop_text_mock_add_item_fail_at = i;
    rc = cupertino_desktop_text_selection_toolbar_create(dummy_engine, &desc,
                                                         &tb);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    ASSERT_EQ(NULL, tb);
  }
  g_cupertino_desktop_text_mock_add_item_fail_at = -1;

  /* Test failure for recompute_bounds */
  g_cupertino_desktop_text_mock_recompute_fail = 1;
  rc =
      cupertino_desktop_text_selection_toolbar_create(dummy_engine, &desc, &tb);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, tb);
  g_cupertino_desktop_text_mock_recompute_fail = 0;
#endif

  PASS();
}

SUITE(cupertino_desktop_text_selection_toolbar_suite) {
  RUN_TEST(test_desktop_text_toolbar_lifecycle_and_items);
  RUN_TEST(test_desktop_text_toolbar_hover_and_actions);
  RUN_TEST(test_desktop_text_toolbar_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_desktop_text_selection_toolbar_suite);
  GREATEST_MAIN_END();
}
