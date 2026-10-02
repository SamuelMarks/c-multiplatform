/**
 * @file test_cupertino_split_view.c
 * @brief Unit tests for Cupertino Navigation Split View (UISplitViewController
 * / NavigationSplitView).
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_split_view.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_split_view_suite);

TEST test_split_view_invalid_arguments(void) {
  struct cupertino_split_view_descriptor desc;
  struct cupertino_split_view *view = NULL;
  struct ui_split_pane_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct ui_event dummy_event;
  enum cupertino_split_view_style style;
  enum cupertino_split_view_display_mode mode;
  float val1, val2;
  int flag1, flag2, flag3;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  memset(&dummy_event, 0, sizeof(dummy_event));

  /* Creation invalid */
  rc = cupertino_split_view_create(NULL, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_create(dummy_engine, NULL, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Out of range style / display_mode */
  desc.style = (enum cupertino_split_view_style)99;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.style = CUPERTINO_SPLIT_VIEW_TWO_COLUMN;
  desc.display_mode = (enum cupertino_split_view_display_mode)99;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_split_view_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Setters / getters invalid */
  rc = cupertino_split_view_set_style(NULL, CUPERTINO_SPLIT_VIEW_TWO_COLUMN);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_style(NULL, &style);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_set_display_mode(NULL,
                                             CUPERTINO_SPLIT_VIEW_AUTOMATIC);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_display_mode(NULL, &mode);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_set_sidebar_width(NULL, 250.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_sidebar_width(NULL, &val1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_set_content_width(NULL, 250.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_content_width(NULL, &val1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_set_viewport_size(NULL, 1024.0f, 768.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_viewport_size(NULL, &val1, &val2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_set_collapsed(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_is_collapsed(NULL, &flag1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_get_column_visibility(NULL, &flag1, &flag2, &flag3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_process_event(NULL, &dummy_event);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_split_view_two_and_three_column_layout(void) {
  struct cupertino_split_view_descriptor desc;
  struct cupertino_split_view *view = NULL;
  struct ui_split_pane_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_split_view_style style;
  enum cupertino_split_view_display_mode mode;
  float sidebar_w, content_w, view_w, view_h;
  int is_col, show_side, show_content, show_detail;
  ui_error_t rc;

  /* Initialize two-column split view */
  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_SPLIT_VIEW_TWO_COLUMN;
  desc.display_mode = CUPERTINO_SPLIT_VIEW_AUTOMATIC;
  desc.sidebar_width = 250.0f;
  desc.viewport_width = 1024.0f;
  desc.viewport_height = 768.0f;

  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(view != NULL);

  rc = cupertino_split_view_get_base(view, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = cupertino_split_view_get_style(view, &style);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SPLIT_VIEW_TWO_COLUMN, style);

  rc = cupertino_split_view_get_display_mode(view, &mode);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SPLIT_VIEW_AUTOMATIC, mode);

  rc = cupertino_split_view_get_sidebar_width(view, &sidebar_w);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(250.0f, sidebar_w, "%f");

  rc = cupertino_split_view_get_column_visibility(view, &show_side,
                                                  &show_content, &show_detail);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, show_side);
  ASSERT_EQ(0, show_content);
  ASSERT_EQ(1, show_detail);

  /* Switch to three-column */
  rc = cupertino_split_view_set_style(view, CUPERTINO_SPLIT_VIEW_THREE_COLUMN);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_split_view_get_style(view, &style);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SPLIT_VIEW_THREE_COLUMN, style);

  /* When viewport width is 1024 (< 1100), automatic three-column shows content
   * + detail */
  rc = cupertino_split_view_get_column_visibility(view, &show_side,
                                                  &show_content, &show_detail);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, show_side);
  ASSERT_EQ(1, show_content);
  ASSERT_EQ(1, show_detail);

  /* Resize viewport to 1200 >= 1100, should show all 3 columns */
  rc = cupertino_split_view_set_viewport_size(view, 1200.0f, 800.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_split_view_get_viewport_size(view, &view_w, &view_h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(1200.0f, view_w, "%f");
  ASSERT_EQ_FMT(800.0f, view_h, "%f");

  rc = cupertino_split_view_get_column_visibility(view, &show_side,
                                                  &show_content, &show_detail);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, show_side);
  ASSERT_EQ(1, show_content);
  ASSERT_EQ(1, show_detail);

  /* Test display modes */
  rc = cupertino_split_view_set_display_mode(
      view, CUPERTINO_SPLIT_VIEW_SECONDARY_ONLY);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_split_view_get_column_visibility(view, &show_side,
                                                  &show_content, &show_detail);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, show_side);
  ASSERT_EQ(0, show_content);
  ASSERT_EQ(1, show_detail);

  rc = cupertino_split_view_set_display_mode(
      view, CUPERTINO_SPLIT_VIEW_ONE_BESIDE_SECONDARY);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_split_view_get_column_visibility(view, &show_side,
                                                  &show_content, &show_detail);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, show_side);
  ASSERT_EQ(1, show_content);
  ASSERT_EQ(1, show_detail);

  rc = cupertino_split_view_set_display_mode(
      view, CUPERTINO_SPLIT_VIEW_TWO_BESIDE_SECONDARY);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_split_view_get_column_visibility(view, &show_side,
                                                  &show_content, &show_detail);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, show_side);
  ASSERT_EQ(1, show_content);
  ASSERT_EQ(1, show_detail);

  /* Test compact size-class auto-collapse */
  rc = cupertino_split_view_set_viewport_size(view, 400.0f, 800.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_split_view_is_collapsed(view, &is_col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_col);

  rc = cupertino_split_view_get_column_visibility(view, &show_side,
                                                  &show_content, &show_detail);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, show_side);
  ASSERT_EQ(0, show_content);
  ASSERT_EQ(1, show_detail);

  /* Test manual uncollapse / collapse */
  rc = cupertino_split_view_set_collapsed(view, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_split_view_is_collapsed(view, &is_col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_col);

  /* Test setting widths with boundary clamping */
  rc = cupertino_split_view_set_sidebar_width(view, 300.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_split_view_get_sidebar_width(view, &sidebar_w);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(300.0f, sidebar_w, "%f");

  rc = cupertino_split_view_set_sidebar_width(view, 100.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_set_sidebar_width(view, 500.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_set_content_width(view, 280.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_split_view_get_content_width(view, &content_w);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(280.0f, content_w, "%f");

  rc = cupertino_split_view_set_content_width(view, 100.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_set_content_width(view, 500.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Process dummy input event */
  {
    struct ui_event evt;
    memset(&evt, 0, sizeof(evt));
    evt.type = UI_EVENT_MOUSE_MOVE;
    rc = cupertino_split_view_process_event(view, &evt);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = cupertino_split_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_split_view_oom_simulation(void) {
  struct cupertino_split_view_descriptor desc;
  struct cupertino_split_view *view = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_SPLIT_VIEW_TWO_COLUMN;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, view);
  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_split_view_create(NULL, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
extern int g_cupertino_split_view_mock_base_create_fail;
extern int g_cupertino_split_view_mock_base_destroy_fail;
extern int g_cupertino_split_view_mock_base_set_orient_fail;
extern int g_cupertino_split_view_mock_base_set_bounds_fail;
extern int g_cupertino_split_view_mock_base_set_pos_fail;
extern int g_cupertino_split_view_mock_base_forward_fail;
extern int g_cupertino_split_view_mock_base_get_pos_fail;
#endif

TEST test_split_view_coverage_branches(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_split_view_descriptor desc;
  struct cupertino_split_view *view = NULL;
  struct ui_split_pane_base *base = NULL;
  struct ui_event evt;
  float w = 0.0f, h = 0.0f;
  int is_col = 0, s = 0, c = 0, d = 0;
  enum cupertino_split_view_style style;
  enum cupertino_split_view_display_mode mode;
  ui_error_t rc;

  /* Descriptor with valid sidebar_width within bounds and content_width within
   * bounds */
  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_SPLIT_VIEW_THREE_COLUMN;
  desc.display_mode = CUPERTINO_SPLIT_VIEW_ONE_BESIDE_SECONDARY;
  desc.viewport_width = 1200.0f;
  desc.viewport_height = 800.0f;
  desc.sidebar_width = 250.0f;
  desc.content_width = 300.0f;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(250.0f, view->sidebar_width, "%f");
  ASSERT_EQ_FMT(300.0f, view->content_width, "%f");

  /* Layout under ONE_BESIDE_SECONDARY with THREE_COLUMN: show_sidebar=0,
   * show_content=1, show_detail=1 */
  rc = cupertino_split_view_get_column_visibility(view, &s, &c, &d);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, s);
  ASSERT_EQ(1, c);
  ASSERT_EQ(1, d);

  /* Switch to TWO_BESIDE_SECONDARY with THREE_COLUMN: show_sidebar=1,
   * show_content=1, show_detail=1 */
  rc = cupertino_split_view_set_display_mode(
      view, CUPERTINO_SPLIT_VIEW_TWO_BESIDE_SECONDARY);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_split_view_get_column_visibility(view, &s, &c, &d);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, s);
  ASSERT_EQ(1, c);
  ASSERT_EQ(1, d);

  /* Switch to TWO_BESIDE_SECONDARY with TWO_COLUMN: show_sidebar=1,
   * show_content=0, show_detail=1 */
  rc = cupertino_split_view_set_style(view, CUPERTINO_SPLIT_VIEW_TWO_COLUMN);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_split_view_set_display_mode(
      view, CUPERTINO_SPLIT_VIEW_TWO_BESIDE_SECONDARY);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_split_view_get_column_visibility(view, &s, &c, &d);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, s);
  ASSERT_EQ(0, c);
  ASSERT_EQ(1, d);

  rc = cupertino_split_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  view = NULL;

  /* Descriptors testing sidebar_width and content_width boundaries */
  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_SPLIT_VIEW_TWO_COLUMN;
  desc.sidebar_width = 50.0f;  /* < MIN */
  desc.content_width = 600.0f; /* > MAX */
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(CUPERTINO_SPLIT_VIEW_DEFAULT_SIDEBAR_WIDTH, view->sidebar_width,
                "%f");
  ASSERT_EQ_FMT(CUPERTINO_SPLIT_VIEW_DEFAULT_CONTENT_WIDTH, view->content_width,
                "%f");
  rc = cupertino_split_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  view = NULL;

  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_SPLIT_VIEW_TWO_COLUMN;
  desc.sidebar_width = 600.0f; /* > MAX */
  desc.content_width = 50.0f;  /* < MIN */
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(CUPERTINO_SPLIT_VIEW_DEFAULT_SIDEBAR_WIDTH, view->sidebar_width,
                "%f");
  ASSERT_EQ_FMT(CUPERTINO_SPLIT_VIEW_DEFAULT_CONTENT_WIDTH, view->content_width,
                "%f");
  rc = cupertino_split_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  view = NULL;

  /* Descriptor with invalid style (> 1) and (< 0) */
  memset(&desc, 0, sizeof(desc));
  desc.style = (enum cupertino_split_view_style)2;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.style = (enum cupertino_split_view_style) - 1;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Descriptor with invalid display_mode (> 3) and (< 0) */
  desc.style = CUPERTINO_SPLIT_VIEW_TWO_COLUMN;
  desc.display_mode = (enum cupertino_split_view_display_mode)4;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.display_mode = (enum cupertino_split_view_display_mode) - 1;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Descriptor with negative viewport width and height (< 0.0f) to trigger
   * defaults */
  desc.display_mode = CUPERTINO_SPLIT_VIEW_AUTOMATIC;
  desc.viewport_width = -10.0f;
  desc.viewport_height = -10.0f;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(1024.0f, view->viewport_width, "%f");
  ASSERT_EQ_FMT(768.0f, view->viewport_height, "%f");
  rc = cupertino_split_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  view = NULL;

  /* Descriptor with viewport_width == 0 (covers branch viewport_width > 0.0f)
   */
  desc.viewport_width = 0.0f;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, view->is_collapsed);
  rc = cupertino_split_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  view = NULL;

  /* set_style with invalid style */
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_split_view_set_style(view, (enum cupertino_split_view_style)2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_set_style(view,
                                      (enum cupertino_split_view_style) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* set_display_mode with invalid mode */
  rc = cupertino_split_view_set_display_mode(
      view, (enum cupertino_split_view_display_mode)4);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_set_display_mode(
      view, (enum cupertino_split_view_display_mode) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* update_layout switch default branch via unknown display mode */
  view->display_mode = (enum cupertino_split_view_display_mode)99;
  rc = cupertino_split_view_set_style(view, CUPERTINO_SPLIT_VIEW_TWO_COLUMN);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* set_viewport_size with negative width or height -> invalid arg */
  rc = cupertino_split_view_set_viewport_size(view, -1.0f, 100.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_set_viewport_size(view, 100.0f, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* set_viewport_size with width == 0 (covers branch width > 0.0f) */
  rc = cupertino_split_view_set_viewport_size(view, 0.0f, 500.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, view->is_collapsed);

  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_split_view_set_style(view, CUPERTINO_SPLIT_VIEW_TWO_COLUMN);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_split_view_get_column_visibility(view, &s, &c, &d);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, s);
  ASSERT_EQ(0, c);
  ASSERT_EQ(1, d);

  rc = cupertino_split_view_set_display_mode(
      view, CUPERTINO_SPLIT_VIEW_ONE_BESIDE_SECONDARY);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_split_view_get_column_visibility(view, &s, &c, &d);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, s);
  ASSERT_EQ(0, c);
  ASSERT_EQ(1, d);

  /* Null checks */
  rc = cupertino_split_view_get_style(NULL, &style);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_style(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_get_display_mode(NULL, &mode);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_display_mode(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_get_sidebar_width(NULL, &w);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_sidebar_width(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_get_content_width(NULL, &w);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_content_width(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_get_viewport_size(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_viewport_size(view, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_viewport_size(view, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_is_collapsed(NULL, &is_col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_is_collapsed(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_get_column_visibility(NULL, &s, &c, &d);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_column_visibility(view, NULL, &c, &d);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_column_visibility(view, &s, NULL, &d);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_column_visibility(view, &s, &c, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_process_event(NULL, &evt);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_process_event(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_split_view_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_split_view_get_base(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Operations when view->base is NULL */
  ui_split_pane_base_destroy(view->base);
  view->base = NULL;

  rc = cupertino_split_view_set_sidebar_width(view, 260.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&evt, 0, sizeof(evt));
  evt.type = UI_EVENT_MOUSE_MOVE;
  rc = cupertino_split_view_process_event(view, &evt);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_split_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  view = NULL;

  /* Mock error paths */
#ifdef UI_TEST_MOCK_ALLOC
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* base destroy failure in cupertino_split_view_destroy */
  g_cupertino_split_view_mock_base_destroy_fail = 1;
  rc = cupertino_split_view_destroy(view);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_split_view_mock_base_destroy_fail = 0;

  /* set_sidebar_width base failure */
  g_cupertino_split_view_mock_base_set_pos_fail = 1;
  rc = cupertino_split_view_set_sidebar_width(view, 240.0f);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_split_view_mock_base_set_pos_fail = 0;

  /* forward event failure */
  g_cupertino_split_view_mock_base_forward_fail = 1;
  rc = cupertino_split_view_process_event(view, &evt);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_split_view_mock_base_forward_fail = 0;

  /* get_position failure during process_event */
  g_cupertino_split_view_mock_base_get_pos_fail = 1;
  rc = cupertino_split_view_process_event(view, &evt);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_split_view_mock_base_get_pos_fail = 0;

  rc = cupertino_split_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  view = NULL;

  /* base_create failure in create */
  g_cupertino_split_view_mock_base_create_fail = 1;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_split_view_mock_base_create_fail = 0;

  /* set_orientation failure in create */
  g_cupertino_split_view_mock_base_set_orient_fail = 1;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_split_view_mock_base_set_orient_fail = 0;

  /* set_orientation failure with destroy failure */
  g_cupertino_split_view_mock_base_set_orient_fail = 1;
  g_cupertino_split_view_mock_base_destroy_fail = 1;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_split_view_mock_base_set_orient_fail = 0;
  g_cupertino_split_view_mock_base_destroy_fail = 0;

  /* set_bounds failure in create */
  g_cupertino_split_view_mock_base_set_bounds_fail = 1;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_split_view_mock_base_set_bounds_fail = 0;

  /* set_bounds failure with destroy failure */
  g_cupertino_split_view_mock_base_set_bounds_fail = 1;
  g_cupertino_split_view_mock_base_destroy_fail = 1;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_split_view_mock_base_set_bounds_fail = 0;
  g_cupertino_split_view_mock_base_destroy_fail = 0;

  /* set_position failure in create */
  g_cupertino_split_view_mock_base_set_pos_fail = 1;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_split_view_mock_base_set_pos_fail = 0;

  /* set_position failure with destroy failure */
  g_cupertino_split_view_mock_base_set_pos_fail = 1;
  g_cupertino_split_view_mock_base_destroy_fail = 1;
  rc = cupertino_split_view_create(dummy_engine, &desc, &view);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_split_view_mock_base_set_pos_fail = 0;
  g_cupertino_split_view_mock_base_destroy_fail = 0;
#endif

  PASS();
}

SUITE(cupertino_split_view_suite) {
  RUN_TEST(test_split_view_invalid_arguments);
  RUN_TEST(test_split_view_two_and_three_column_layout);
  RUN_TEST(test_split_view_oom_simulation);
  RUN_TEST(test_split_view_coverage_branches);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_split_view_suite);
  GREATEST_MAIN_END();
}
