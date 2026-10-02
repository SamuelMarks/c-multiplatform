/**
 * @file test_cupertino_scrollbar.c
 * @brief Unit tests for Cupertino Scrollbar component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_scrollbar.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_scrollbar_suite);

TEST test_scrollbar_invalid_args(void) {
  struct cupertino_scrollbar_descriptor desc;
  struct cupertino_scrollbar *scrollbar = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float val1 = 0.0f;
  float val2 = 0.0f;
  int dragging = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Create invalid */
  rc = cupertino_scrollbar_create(NULL, &desc, &scrollbar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrollbar_create(dummy_engine, NULL, &scrollbar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrollbar_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy invalid */
  rc = cupertino_scrollbar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Handle scroll invalid */
  rc = cupertino_scrollbar_handle_scroll(NULL, 10.0f, 1000.0f, 500.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Tick invalid */
  rc = cupertino_scrollbar_tick(NULL, 16.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Dragging invalid */
  rc = cupertino_scrollbar_set_dragging(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrollbar_is_dragging(NULL, &dragging);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Thickness / opacity / geometry */
  rc = cupertino_scrollbar_get_thickness(NULL, &val1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrollbar_get_opacity(NULL, &val1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrollbar_get_thumb_geometry(NULL, &val1, &val2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Safe area insets invalid */
  rc = cupertino_scrollbar_set_safe_area_insets(NULL, 10.0f, 20.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid scrollbar to test getters with NULL outputs and geometry edge cases
   */
  rc = cupertino_scrollbar_create(dummy_engine, &desc, &scrollbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_scrollbar_handle_scroll(scrollbar, 10.0f, 1000.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrollbar_tick(scrollbar, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrollbar_is_dragging(scrollbar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrollbar_get_thickness(scrollbar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrollbar_get_opacity(scrollbar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrollbar_get_thumb_geometry(scrollbar, NULL, &val2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrollbar_get_thumb_geometry(scrollbar, &val1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Safe area insets negative top or bottom */
  rc = cupertino_scrollbar_set_safe_area_insets(scrollbar, -5.0f, 20.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrollbar_set_safe_area_insets(scrollbar, 10.0f, -5.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid safe area insets call */
  rc = cupertino_scrollbar_set_safe_area_insets(scrollbar, 20.0f, 20.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Edge case: track_h <= 0 (safe area insets larger than viewport) */
  rc = cupertino_scrollbar_set_safe_area_insets(scrollbar, 500.0f, 500.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_scrollbar_handle_scroll(scrollbar, 0.0f, 2000.0f, 400.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Edge case: content_length <= viewport_length */
  rc = cupertino_scrollbar_handle_scroll(scrollbar, 0.0f, 200.0f, 400.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(400.0f, scrollbar->thumb_length);

  /* Edge case: thumb_length < CUPERTINO_SCROLLBAR_MIN_THUMB_LENGTH */
  rc = cupertino_scrollbar_set_safe_area_insets(scrollbar, 0.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_scrollbar_handle_scroll(scrollbar, 0.0f, 100000.0f, 100.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SCROLLBAR_MIN_THUMB_LENGTH, scrollbar->thumb_length);

  /* Edge case: scroll offset < 0.0f (overscroll top) */
  rc = cupertino_scrollbar_handle_scroll(scrollbar, -50.0f, 1000.0f, 200.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, scrollbar->thumb_offset);

  /* Edge case: scroll offset > max_scroll (overscroll bottom) */
  rc = cupertino_scrollbar_handle_scroll(scrollbar, 1500.0f, 1000.0f, 200.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Edge case: max_travel < 0.0f (track_h < thumb_length) */
  scrollbar->viewport_length = 10.0f;
  scrollbar->content_length = 100.0f;
  scrollbar->safe_area_top = 0.0f;
  scrollbar->safe_area_bottom = 0.0f;
  rc = cupertino_scrollbar_handle_scroll(scrollbar, 10.0f, 100.0f, 10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_scrollbar_destroy(scrollbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_scrollbar_lifecycle_and_scroll(void) {
  struct cupertino_scrollbar_descriptor desc;
  struct cupertino_scrollbar *scrollbar = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float thickness = 0.0f;
  float opacity = 0.0f;
  float offset = 0.0f;
  float size = 0.0f;
  int dragging = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.safe_area_top = 44.0f;
  desc.safe_area_bottom = 34.0f;

  rc = cupertino_scrollbar_create(dummy_engine, &desc, &scrollbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scrollbar != NULL);

  /* Initial thickness = resting 3.0pt, opacity = 0.0 (hidden before scroll) */
  rc = cupertino_scrollbar_get_thickness(scrollbar, &thickness);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(thickness > 2.99f && thickness < 3.01f);

  rc = cupertino_scrollbar_get_opacity(scrollbar, &opacity);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(opacity < 0.001f);

  /* Scroll event: viewport = 800, content = 1600 (ratio = 0.5), offset = 0 */
  rc = cupertino_scrollbar_handle_scroll(scrollbar, 0.0f, 1600.0f, 800.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Opacity becomes 1.0f on scroll */
  rc = cupertino_scrollbar_get_opacity(scrollbar, &opacity);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(opacity > 0.999f);

  /* Geometry check: track_h = 800 - 44 - 34 = 722. Thumb size = 722 * 0.5 = 361
   */
  rc = cupertino_scrollbar_get_thumb_geometry(scrollbar, &offset, &size);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(offset > 43.9f && offset < 44.1f); /* Starts at safe_area_top */
  ASSERT(size > 360.9f && size < 361.1f);

  /* Scroll to middle: offset = 400 (halfway) */
  rc = cupertino_scrollbar_handle_scroll(scrollbar, 400.0f, 1600.0f, 800.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_scrollbar_get_thumb_geometry(scrollbar, &offset, &size);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* travel = 722 - 361 = 361. offset = 44 + 0.5 * 361 = 224.5 */
  ASSERT(offset > 224.4f && offset < 224.6f);

  /* Test dragging state expands thickness to 6pt */
  rc = cupertino_scrollbar_set_dragging(scrollbar, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_scrollbar_is_dragging(scrollbar, &dragging);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, dragging);

  rc = cupertino_scrollbar_get_thickness(scrollbar, &thickness);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(thickness > 5.99f && thickness < 6.01f);

  /* When dragging, ticks do not fade out scrollbar */
  rc = cupertino_scrollbar_tick(scrollbar, 2000.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_scrollbar_get_opacity(scrollbar, &opacity);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(opacity > 0.999f);

  /* Release dragging: thickness returns to 3pt */
  rc = cupertino_scrollbar_set_dragging(scrollbar, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_scrollbar_get_thickness(scrollbar, &thickness);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(thickness > 2.99f && thickness < 3.01f);

  /* Advance 1000ms: still within 1500ms inactivity window -> opacity
   * remains 1.0 */
  rc = cupertino_scrollbar_tick(scrollbar, 1000.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_scrollbar_get_opacity(scrollbar, &opacity);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(opacity > 0.999f);

  /* Advance another 650ms (total 1650ms): fade elapsed = 150ms / 300ms ->
   * opacity = 0.5 */
  rc = cupertino_scrollbar_tick(scrollbar, 650.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_scrollbar_get_opacity(scrollbar, &opacity);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(opacity > 0.49f && opacity < 0.51f);

  /* Advance another 200ms (total 1850ms >= 1800ms) -> opacity = 0.0 */
  rc = cupertino_scrollbar_tick(scrollbar, 200.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_scrollbar_get_opacity(scrollbar, &opacity);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(opacity < 0.001f);

  /* Destroy */
  rc = cupertino_scrollbar_destroy(scrollbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_scrollbar_oom_mock(void) {
  struct cupertino_scrollbar_descriptor desc;
  struct cupertino_scrollbar *scrollbar = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_scrollbar_create(dummy_engine, &desc, &scrollbar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(scrollbar == NULL);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_scrollbar_create(NULL, &desc, &scrollbar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_scrollbar_suite) {
  RUN_TEST(test_scrollbar_invalid_args);
  RUN_TEST(test_scrollbar_lifecycle_and_scroll);
  RUN_TEST(test_scrollbar_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_scrollbar_suite);
  GREATEST_MAIN_END();
}
