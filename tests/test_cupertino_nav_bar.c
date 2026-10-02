/**
 * @file test_cupertino_nav_bar.c
 * @brief Unit tests for Cupertino Navigation Bar component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_nav_bar.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_nav_bar_suite);

TEST test_nav_bar_invalid_args(void) {
  struct cupertino_nav_bar_descriptor desc;
  struct cupertino_nav_bar *bar = NULL;
  struct ui_top_app_bar_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *str = NULL;
  char buf[64];
  float val = 0.0f;
  int enabled = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Create invalid args */
  rc = cupertino_nav_bar_create(NULL, &desc, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_create(dummy_engine, NULL, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy invalid args */
  rc = cupertino_nav_bar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set / get title */
  rc = cupertino_nav_bar_set_title(NULL, "Title");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_title(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set / get prompt */
  rc = cupertino_nav_bar_set_prompt(NULL, "Prompt");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_prompt(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set / get previous title */
  rc = cupertino_nav_bar_set_previous_title(NULL, "Back");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_previous_title(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Large title mode */
  rc = cupertino_nav_bar_set_large_title_enabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_is_large_title_enabled(NULL, &enabled);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Handle scroll */
  rc = cupertino_nav_bar_handle_scroll(NULL, 10.0f, 1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Get height / progress */
  rc = cupertino_nav_bar_get_height(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_collapse_progress(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Effective back title */
  rc = cupertino_nav_bar_get_effective_back_title(NULL, 100.0f, buf,
                                                  sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Get base */
  rc = cupertino_nav_bar_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_nav_bar_lifecycle_and_scroll(void) {
  struct cupertino_nav_bar_descriptor desc;
  struct cupertino_nav_bar *bar = NULL;
  struct ui_top_app_bar_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *str = NULL;
  char buf[64];
  float h = 0.0f;
  float progress = 0.0f;
  int enabled = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.title = "Settings";
  desc.prompt = "Connecting...";
  desc.previous_title = "General";
  desc.is_large_title_enabled = 1;

  rc = cupertino_nav_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bar != NULL);

  /* Check getters */
  rc = cupertino_nav_bar_get_title(bar, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Settings", str);

  rc = cupertino_nav_bar_get_prompt(bar, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Connecting...", str);

  rc = cupertino_nav_bar_get_previous_title(bar, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("General", str);

  rc = cupertino_nav_bar_is_large_title_enabled(bar, &enabled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, enabled);

  /* Check base retrieval */
  rc = cupertino_nav_bar_get_base(bar, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Expanded height with prompt: 44 (collapsed) + 24 (prompt) + 52 (large
   * title) = 120 */
  rc = cupertino_nav_bar_get_height(bar, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(h > 119.9f && h < 120.1f);

  rc = cupertino_nav_bar_get_collapse_progress(bar, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(progress < 0.001f);

  /* Scroll pull-down (overscroll) */
  rc = cupertino_nav_bar_handle_scroll(bar, -20.0f, -5.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_get_height(bar, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(h > 139.9f && h < 140.1f);

  /* Scroll intermediate (collapse 26pt = 50%) */
  rc = cupertino_nav_bar_handle_scroll(bar, 26.0f, 10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_get_collapse_progress(bar, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(progress > 0.49f && progress < 0.51f);
  ASSERT(bar->large_title_opacity > 0.49f && bar->large_title_opacity < 0.51f);

  /* Scroll beyond threshold -> fully collapsed (height = 44 + 24 = 68) */
  rc = cupertino_nav_bar_handle_scroll(bar, 100.0f, 20.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_get_height(bar, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(h > 67.9f && h < 68.1f);
  rc = cupertino_nav_bar_get_collapse_progress(bar, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(progress > 0.999f);
  ASSERT(bar->inline_title_opacity > 0.999f);
  ASSERT(bar->large_title_opacity < 0.001f);

  /* Clear prompt: collapsed height becomes 44 */
  rc = cupertino_nav_bar_set_prompt(bar, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_get_prompt(bar, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(str == NULL);
  rc = cupertino_nav_bar_get_height(bar, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(h > 43.9f && h < 44.1f);

  /* Change title */
  rc = cupertino_nav_bar_set_title(bar, "Wi-Fi");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_get_title(bar, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Wi-Fi", str);

  /* Disable large title */
  rc = cupertino_nav_bar_set_large_title_enabled(bar, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_is_large_title_enabled(bar, &enabled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, enabled);
  rc = cupertino_nav_bar_get_height(bar, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(h > 43.9f && h < 44.1f);

  /* Test back title resolution */
  /* 1. Too constrained (< 40pt) -> empty */
  rc = cupertino_nav_bar_get_effective_back_title(bar, 30.0f, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", buf);

  /* 2. Fits previous title ("General" ~ 7*9 + 20 = 83pt) */
  rc =
      cupertino_nav_bar_get_effective_back_title(bar, 120.0f, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("General", buf);

  /* 3. Doesn't fit previous title (needed 83pt, available 70pt), but fits
   * "Back" (>=60pt) */
  rc = cupertino_nav_bar_get_effective_back_title(bar, 70.0f, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Back", buf);

  /* 4. Doesn't fit "Back" (<60pt, e.g. 50pt) */
  rc = cupertino_nav_bar_get_effective_back_title(bar, 50.0f, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", buf);

  /* 5. Clear previous title, test fallback to "Back" */
  rc = cupertino_nav_bar_set_previous_title(bar, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_get_previous_title(bar, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(str == NULL);
  rc = cupertino_nav_bar_get_effective_back_title(bar, 80.0f, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Back", buf);

  rc = cupertino_nav_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_nav_bar_oom_mock(void) {
  struct cupertino_nav_bar_descriptor desc;
  struct cupertino_nav_bar *bar = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.title = "OOM Test";

#ifdef UI_TEST_MOCK_ALLOC
  /* Fail malloc for struct cupertino_nav_bar */
  g_malloc_fail_countdown = 0;
  rc = cupertino_nav_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(bar == NULL);

  /* Fail malloc inside arena creation */
  g_malloc_fail_countdown = 1;
  rc = cupertino_nav_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(bar == NULL);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_nav_bar_create(NULL, &desc, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_nav_bar_search_and_scale(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  struct cupertino_nav_bar_descriptor desc;
  struct cupertino_nav_bar *bar = NULL;
  struct cupertino_search_bar *dummy_search =
      (struct cupertino_search_bar *)0x2000;
  struct cupertino_search_bar *ret_search = NULL;
  int hides = 0;
  float prog = 0.0f;
  float opac = 0.0f;
  float scale = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.title = "A Very Long Navigation Bar Header Title That Needs Scaling";
  desc.is_large_title_enabled = 1;

  rc = cupertino_nav_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Title scale factor on narrow width */
  rc = cupertino_nav_bar_compute_title_scale(bar, 200.0f, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.7f, scale, 0.01f);

  /* Title scale factor on wide width */
  rc = cupertino_nav_bar_compute_title_scale(bar, 3000.0f, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(1.0f, scale, 0.01f);

  /* Attach search controller with hides_when_scrolling = 1 */
  rc = cupertino_nav_bar_set_search_controller(bar, dummy_search, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_nav_bar_get_search_controller(bar, &ret_search, &hides);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(dummy_search, ret_search);
  ASSERT_EQ(1, hides);

  /* Scroll 0: revealed */
  rc = cupertino_nav_bar_handle_scroll(bar, 0.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_get_search_collapse_progress(bar, &prog, &opac);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.0f, prog, 0.01f);
  ASSERT_IN_RANGE(1.0f, opac, 0.01f);

  /* Scroll 26: 50% collapsed */
  rc = cupertino_nav_bar_handle_scroll(bar, 26.0f, 10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_get_search_collapse_progress(bar, &prog, &opac);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.5f, prog, 0.01f);
  ASSERT_IN_RANGE(0.5f, opac, 0.01f);

  /* Scroll 60: 100% collapsed */
  rc = cupertino_nav_bar_handle_scroll(bar, 60.0f, 10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_get_search_collapse_progress(bar, &prog, &opac);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(1.0f, prog, 0.01f);
  ASSERT_IN_RANGE(0.0f, opac, 0.01f);

  /* Attach with hides_when_scrolling = 0 (always pinned) */
  rc = cupertino_nav_bar_set_search_controller(bar, dummy_search, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_handle_scroll(bar, 60.0f, 10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_get_search_collapse_progress(bar, &prog, &opac);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.0f, prog, 0.01f);
  ASSERT_IN_RANGE(1.0f, opac, 0.01f);

  /* Detach */
  rc = cupertino_nav_bar_set_search_controller(bar, NULL, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_get_search_controller(bar, &ret_search, &hides);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(NULL, ret_search);

  /* Null checks */
  rc = cupertino_nav_bar_set_search_controller(NULL, dummy_search, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_search_controller(NULL, &ret_search, &hides);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_search_controller(bar, NULL, &hides);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_search_controller(bar, &ret_search, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_search_collapse_progress(NULL, &prog, &opac);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_search_collapse_progress(bar, NULL, &opac);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_search_collapse_progress(bar, &prog, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_compute_title_scale(NULL, 200.0f, &scale);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_compute_title_scale(bar, 0.0f, &scale);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_compute_title_scale(bar, 200.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_nav_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
extern int g_cupertino_nav_bar_mock_base_create_fail;
extern int g_cupertino_nav_bar_mock_base_destroy_fail;
extern int g_cupertino_nav_bar_mock_base_handle_scroll_fail;
extern int g_cupertino_nav_bar_mock_arena_destroy_fail;
#endif

TEST test_nav_bar_coverage_branches(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_nav_bar_descriptor desc;
  struct cupertino_nav_bar *bar = NULL;
  struct ui_top_app_bar_base *base = NULL;
  float scale = 0.0f;
  const char *str = NULL;
  char buf[64];
  ui_error_t rc;

  /* Create with NULL title */
  memset(&desc, 0, sizeof(desc));
  desc.title = NULL;
  desc.is_large_title_enabled = 0;
  rc = cupertino_nav_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", bar->title);

  /* Title scale with empty title -> scale = 1.0 */
  rc = cupertino_nav_bar_compute_title_scale(bar, 100.0f, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.0f, scale);

  /* Title scale with scale between 0.7 and 1.0 (e.g. available_width = 160, len
   * = 10, measured_w = 200, scale = 0.8) */
  rc = cupertino_nav_bar_set_title(bar, "0123456789");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_compute_title_scale(bar, 160.0f, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scale > 0.79f && scale < 0.81f);

  /* Set title NULL check */
  rc = cupertino_nav_bar_set_title(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set very long title to trigger scale < 0.7f clamped to 0.7f */
  rc = cupertino_nav_bar_set_title(
      bar, "This is an extremely long title that exceeds the available space");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_compute_title_scale(bar, 50.0f, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.7f, scale);

  /* Set prompt and previous_title non-null, then null */
  rc = cupertino_nav_bar_set_prompt(bar, "A prompt");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_get_prompt(bar, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("A prompt", str);
  rc = cupertino_nav_bar_set_prompt(bar, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_nav_bar_set_previous_title(bar, "Back");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_get_previous_title(bar, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Back", str);
  rc = cupertino_nav_bar_set_previous_title(bar, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_get_previous_title(bar, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(NULL, str);

  /* Null checks */
  rc = cupertino_nav_bar_get_title(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_title(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_nav_bar_get_prompt(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_prompt(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_nav_bar_get_previous_title(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_previous_title(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_nav_bar_is_large_title_enabled(NULL, (int *)buf);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_is_large_title_enabled(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_nav_bar_get_height(NULL, &scale);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_height(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_nav_bar_get_collapse_progress(NULL, &scale);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_collapse_progress(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_nav_bar_get_effective_back_title(NULL, 100.0f, buf,
                                                  sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_effective_back_title(bar, 100.0f, NULL,
                                                  sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_effective_back_title(bar, 100.0f, buf, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_nav_bar_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_nav_bar_get_base(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Scroll intermediate with p > 0.6f (e.g. 75% collapse) */
  rc = cupertino_nav_bar_set_large_title_enabled(bar, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_handle_scroll(bar, 39.0f, 10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bar->inline_title_opacity > 0.0f);

  /* Search controller with hides_search_when_scrolling enabled */
  rc = cupertino_nav_bar_set_search_controller(
      bar, (struct cupertino_search_bar *)0x5678, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_handle_scroll(bar, 10.0f, 5.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Search controller with large title disabled (collapsed search height
   * adjustment) */
  rc = cupertino_nav_bar_set_large_title_enabled(bar, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_get_height(bar, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Search controller with large title disabled and hides_search_when_scrolling
   * == 0 */
  rc = cupertino_nav_bar_set_search_controller(
      bar, (struct cupertino_search_bar *)0x5678, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_nav_bar_handle_scroll(bar, 10.0f, 5.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Handle scroll when bar->base is NULL */
  ui_top_app_bar_base_destroy(bar->base);
  bar->base = NULL;
  rc = cupertino_nav_bar_handle_scroll(bar, 10.0f, 5.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy when bar->arena is NULL */
  ui_arena_destroy(bar->arena);
  bar->arena = NULL;
  rc = cupertino_nav_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  bar = NULL;

  /* Error paths with mocks */
#ifdef UI_TEST_MOCK_ALLOC
  rc = cupertino_nav_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* on_scroll failure */
  g_cupertino_nav_bar_mock_base_handle_scroll_fail = 1;
  rc = cupertino_nav_bar_handle_scroll(bar, 5.0f, 5.0f);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_nav_bar_mock_base_handle_scroll_fail = 0;

  /* base destroy failure in nav_bar_destroy */
  g_cupertino_nav_bar_mock_base_destroy_fail = 1;
  rc = cupertino_nav_bar_destroy(bar);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_nav_bar_mock_base_destroy_fail = 0;

  /* arena destroy failure in nav_bar_destroy */
  g_cupertino_nav_bar_mock_arena_destroy_fail = 1;
  rc = cupertino_nav_bar_destroy(bar);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_nav_bar_mock_arena_destroy_fail = 0;

  /* clean up bar */
  rc = cupertino_nav_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  bar = NULL;

  /* top_app_bar_base_create failure during create */
  g_cupertino_nav_bar_mock_base_create_fail = 1;
  rc = cupertino_nav_bar_create(dummy_engine, &desc, &bar);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_nav_bar_mock_base_create_fail = 0;

  /* top_app_bar_base_create failure with arena destroy failure */
  g_cupertino_nav_bar_mock_base_create_fail = 1;
  g_cupertino_nav_bar_mock_arena_destroy_fail = 1;
  rc = cupertino_nav_bar_create(dummy_engine, &desc, &bar);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_nav_bar_mock_base_create_fail = 0;
  g_cupertino_nav_bar_mock_arena_destroy_fail = 0;
#else
  rc = cupertino_nav_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#endif

  PASS();
}

SUITE(cupertino_nav_bar_suite) {
  RUN_TEST(test_nav_bar_invalid_args);
  RUN_TEST(test_nav_bar_lifecycle_and_scroll);
  RUN_TEST(test_nav_bar_search_and_scale);
  RUN_TEST(test_nav_bar_oom_mock);
  RUN_TEST(test_nav_bar_coverage_branches);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_nav_bar_suite);
  GREATEST_MAIN_END();
}
