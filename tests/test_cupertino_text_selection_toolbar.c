/**
 * @file test_cupertino_text_selection_toolbar.c
 * @brief Unit tests for Cupertino iOS Text Selection Toolbar and Grabber
 * Handles.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_text_selection_toolbar.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_text_selection_toolbar_suite);

TEST test_text_toolbar_invalid_arguments(void) {
  struct cupertino_text_selection_toolbar_descriptor desc;
  struct cupertino_text_selection_toolbar *tb = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  int val = 0;
  int total = 0;
  float x, y, w, h;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Creation invalid */
  rc = cupertino_text_selection_toolbar_create(NULL, &desc, &tb);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_create(dummy_engine, NULL, &tb);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_text_selection_toolbar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Show / hide / visible / paginate / action / bounds invalid */
  rc = cupertino_text_selection_toolbar_show(NULL, 10.0f, 100.0f, 50.0f, 20.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_show(
      (struct cupertino_text_selection_toolbar *)0x123, 10.0f, 100.0f, -1.0f,
      20.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_show(
      (struct cupertino_text_selection_toolbar *)0x123, 10.0f, 100.0f, 50.0f,
      -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_selection_toolbar_hide(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_is_visible(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_is_visible(
      (const struct cupertino_text_selection_toolbar *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_selection_toolbar_get_page(NULL, &val, &total);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_get_page(
      (const struct cupertino_text_selection_toolbar *)0x123, NULL, &total);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_get_page(
      (const struct cupertino_text_selection_toolbar *)0x123, &val, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_selection_toolbar_paginate(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_selection_toolbar_trigger_action(
      NULL, CUPERTINO_TEXT_ACTION_COPY, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_trigger_action(
      (struct cupertino_text_selection_toolbar *)0x123,
      CUPERTINO_TEXT_ACTION_COPY, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_trigger_action(
      (struct cupertino_text_selection_toolbar *)0x123,
      (enum cupertino_text_action) - 1, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_trigger_action(
      (struct cupertino_text_selection_toolbar *)0x123,
      CUPERTINO_TEXT_ACTION_COUNT, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_selection_toolbar_get_bounds(NULL, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_get_bounds(
      (const struct cupertino_text_selection_toolbar *)0x123, NULL, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_get_bounds(
      (const struct cupertino_text_selection_toolbar *)0x123, &x, NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_get_bounds(
      (const struct cupertino_text_selection_toolbar *)0x123, &x, &y, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_get_bounds(
      (const struct cupertino_text_selection_toolbar *)0x123, &x, &y, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_selection_toolbar_get_handle_bounds(NULL, 1, &x, &y, &w,
                                                          &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_get_handle_bounds(
      (const struct cupertino_text_selection_toolbar *)0x123, 1, NULL, &y, &w,
      &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_get_handle_bounds(
      (const struct cupertino_text_selection_toolbar *)0x123, 1, &x, NULL, &w,
      &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_get_handle_bounds(
      (const struct cupertino_text_selection_toolbar *)0x123, 1, &x, &y, NULL,
      &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_selection_toolbar_get_handle_bounds(
      (const struct cupertino_text_selection_toolbar *)0x123, 1, &x, &y, &w,
      NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_text_toolbar_lifecycle_and_actions(void) {
  struct cupertino_text_selection_toolbar_descriptor desc;
  struct cupertino_text_selection_toolbar *tb = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  int is_visible = 0;
  int page = 0;
  int total = 0;
  int handled = 0;
  float x, y, w, h;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  /* Allow all actions */
  desc.allowed_actions_mask = 0;

  rc = cupertino_text_selection_toolbar_create(dummy_engine, &desc, &tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tb != NULL);

  rc = cupertino_text_selection_toolbar_is_visible(tb, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_visible);

  /* Show anchored above text selection at (100, 200) with size 80x20 */
  rc = cupertino_text_selection_toolbar_show(tb, 100.0f, 200.0f, 80.0f, 20.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_text_selection_toolbar_is_visible(tb, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_visible);

  /* Bubble bounds check */
  rc = cupertino_text_selection_toolbar_get_bounds(tb, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(w > 0.0f);
  ASSERT(h > 0.0f);
  ASSERT_EQ(1, tb->arrow_on_bottom);
  ASSERT(y < 200.0f);

  /* Start handle bounds */
  rc =
      cupertino_text_selection_toolbar_get_handle_bounds(tb, 1, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(100.0f - CUPERTINO_TEXT_HANDLE_RADIUS, x);
  ASSERT_EQ(200.0f, y);

  /* End handle bounds */
  rc =
      cupertino_text_selection_toolbar_get_handle_bounds(tb, 0, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(180.0f - CUPERTINO_TEXT_HANDLE_RADIUS, x);
  ASSERT_EQ(200.0f, y);

  /* Pagination check */
  rc = cupertino_text_selection_toolbar_get_page(tb, &page, &total);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, page);
  ASSERT(total >= 2);

  rc = cupertino_text_selection_toolbar_paginate(tb, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_text_selection_toolbar_get_page(tb, &page, &total);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, page);

  rc = cupertino_text_selection_toolbar_paginate(tb, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_text_selection_toolbar_get_page(tb, &page, &total);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, page);

  /* Paginate backwards while at page 0 -> no-op */
  rc = cupertino_text_selection_toolbar_paginate(tb, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, tb->current_page);

  /* Paginate forward beyond last page */
  tb->current_page = tb->total_pages - 1;
  rc = cupertino_text_selection_toolbar_paginate(tb, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(tb->total_pages - 1, tb->current_page);
  tb->current_page = 0;

  /* Trigger actions */
  rc = cupertino_text_selection_toolbar_trigger_action(
      tb, CUPERTINO_TEXT_ACTION_COPY, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, handled);

  /* Show near top of screen (y = 20) -> arrow flips to top */
  rc = cupertino_text_selection_toolbar_show(tb, 100.0f, 20.0f, 80.0f, 20.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, tb->arrow_on_bottom);

  /* Show with anchor_x = 0 to trigger toolbar->bubble_x < 10.0f clamping */
  rc = cupertino_text_selection_toolbar_show(tb, 0.0f, 100.0f, 10.0f, 20.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10.0f, tb->bubble_x);

  /* Trigger unhandled action */
  tb->actions[0] = CUPERTINO_TEXT_ACTION_CUT;
  tb->action_count = 1;
  rc = cupertino_text_selection_toolbar_trigger_action(
      tb, CUPERTINO_TEXT_ACTION_PASTE, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, handled);

  /* Hide */
  rc = cupertino_text_selection_toolbar_hide(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_text_selection_toolbar_is_visible(tb, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_visible);

  rc = cupertino_text_selection_toolbar_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test creation with custom action mask with zero matching actions */
  memset(&desc, 0, sizeof(desc));
  desc.allowed_actions_mask = 0x8000;
  rc = cupertino_text_selection_toolbar_create(dummy_engine, &desc, &tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, tb->action_count);
  ASSERT_EQ(1, tb->total_pages);
  rc = cupertino_text_selection_toolbar_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test creation with custom action mask (e.g. only 1 action) */
  memset(&desc, 0, sizeof(desc));
  desc.allowed_actions_mask = (1 << CUPERTINO_TEXT_ACTION_COPY);
  rc = cupertino_text_selection_toolbar_create(dummy_engine, &desc, &tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, tb->action_count);
  ASSERT_EQ(1, tb->total_pages);

  /* Test show with 0 actions */
  tb->action_count = 0;
  rc = cupertino_text_selection_toolbar_show(tb, 50.0f, 100.0f, 20.0f, 20.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_text_selection_toolbar_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_text_toolbar_oom_mock(void) {
  struct cupertino_text_selection_toolbar_descriptor desc;
  struct cupertino_text_selection_toolbar *tb = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_text_selection_toolbar_create(dummy_engine, &desc, &tb);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, tb);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_text_selection_toolbar_create(NULL, &desc, &tb);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_text_selection_toolbar_suite) {
  RUN_TEST(test_text_toolbar_invalid_arguments);
  RUN_TEST(test_text_toolbar_lifecycle_and_actions);
  RUN_TEST(test_text_toolbar_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_text_selection_toolbar_suite);
  GREATEST_MAIN_END();
}
