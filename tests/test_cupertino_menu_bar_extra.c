/**
 * @file test_cupertino_menu_bar_extra.c
 * @brief Unit tests for macOS Menu Bar Extras conforming to Apple HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_menu_bar_extra.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

TEST test_menu_bar_extra_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_menu_bar_extra_descriptor desc;
  struct cupertino_menu_bar_extra *extra = NULL;
  const char *title = NULL;
  const char *icon = NULL;
  float w = 0.0f;
  float h = 0.0f;
  int is_open = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.title = "100%";
  desc.icon_symbol = "battery.100.bolt";
  desc.is_animating = 0;
  desc.animation_phase = 0.0f;
  desc.fixed_width = 0.0f;

  /* Null checks */
  rc = cupertino_menu_bar_extra_create(NULL, &desc, &extra);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_create(dummy_engine, NULL, &extra);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = cupertino_menu_bar_extra_create(dummy_engine, &desc, &extra);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, extra);

  rc = cupertino_menu_bar_extra_get_title(extra, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("100%", title);

  rc = cupertino_menu_bar_extra_get_icon(extra, &icon);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("battery.100.bolt", icon);

  rc = cupertino_menu_bar_extra_get_bounds(extra, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_GT(w, 20.0f);
  ASSERT_IN_RANGE(24.0f, h, 0.01f);

  /* Click toggle */
  rc = cupertino_menu_bar_extra_is_menu_open(extra, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  rc = cupertino_menu_bar_extra_click(extra);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_bar_extra_is_menu_open(extra, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_open);

  rc = cupertino_menu_bar_extra_set_menu_open(extra, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_bar_extra_is_menu_open(extra, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  rc = cupertino_menu_bar_extra_destroy(extra);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_menu_bar_extra_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_menu_bar_extra_updates_and_animation(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_menu_bar_extra_descriptor desc;
  struct cupertino_menu_bar_extra *extra = NULL;
  const char *title = NULL;
  const char *icon = NULL;
  int is_anim = 0;
  float phase = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.fixed_width = 50.0f;

  rc = cupertino_menu_bar_extra_create(dummy_engine, &desc, &extra);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_menu_bar_extra_get_bounds(extra, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(50.0f, w, 0.01f);

  /* Title mutations */
  rc = cupertino_menu_bar_extra_set_title(extra, "Wi-Fi: Connected");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_bar_extra_get_title(extra, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Wi-Fi: Connected", title);

  rc = cupertino_menu_bar_extra_set_title(extra, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_bar_extra_get_title(extra, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", title);

  /* Icon mutations */
  rc = cupertino_menu_bar_extra_set_icon(extra, "wifi");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_bar_extra_get_icon(extra, &icon);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("wifi", icon);

  rc = cupertino_menu_bar_extra_set_icon(extra, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_bar_extra_get_icon(extra, &icon);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", icon);

  /* Animation */
  rc = cupertino_menu_bar_extra_set_animating(extra, 1, 0.5f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_bar_extra_is_animating(extra, &is_anim, &phase);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_anim);
  ASSERT_IN_RANGE(0.5f, phase, 0.01f);

  /* Invalid phase */
  rc = cupertino_menu_bar_extra_set_animating(extra, 1, 1.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_set_animating(extra, 1, -0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Null queries */
  rc = cupertino_menu_bar_extra_get_title(NULL, &title);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_get_title(extra, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_get_icon(NULL, &icon);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_get_icon(extra, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_is_animating(NULL, &is_anim, &phase);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_is_animating(extra, NULL, &phase);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_set_menu_open(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_is_menu_open(NULL, &is_anim);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_is_menu_open(extra, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_click(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_get_bounds(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_set_title(NULL, "Test");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_set_icon(NULL, "Test");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_menu_bar_extra_destroy(extra);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_menu_bar_extra_oom(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_menu_bar_extra_descriptor desc;
  struct cupertino_menu_bar_extra *extra = NULL;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.title = "Clock";

  g_malloc_fail_countdown = 0;
  rc = cupertino_menu_bar_extra_create(dummy_engine, &desc, &extra);
  g_malloc_fail_countdown = -1;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, extra);

  PASS();
}

TEST test_menu_bar_extra_edge_cases(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_menu_bar_extra_descriptor desc;
  struct cupertino_menu_bar_extra *extra = NULL;
  float w = 0.0f;
  float h = 0.0f;
  float phase = 0.0f;
  int is_anim = 0;
  ui_error_t rc;

  /* Null extra for recompute_bounds */
  rc = cupertino_menu_bar_extra_recompute_bounds(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Case: title only (no icon) and fixed_width 0 (to hit content_w == 0 branch)
   */
  memset(&desc, 0, sizeof(desc));
  desc.title = "Volume";
  rc = cupertino_menu_bar_extra_create(dummy_engine, &desc, &extra);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_GT(extra->computed_width, 24.0f);
  rc = cupertino_menu_bar_extra_destroy(extra);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Case: both icon and title present (to test content_w > 0.0f gap branch) */
  memset(&desc, 0, sizeof(desc));
  desc.title = "Battery";
  desc.icon_symbol = "battery.100";
  desc.animation_phase = 0.5f;
  rc = cupertino_menu_bar_extra_create(dummy_engine, &desc, &extra);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_GT(extra->computed_width, 24.0f);

  /* Set animating with phase bounds */
  rc = cupertino_menu_bar_extra_set_animating(NULL, 1, 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Query is_animating with NULL out_phase (to test out_phase == NULL branch)
   */
  rc = cupertino_menu_bar_extra_is_animating(extra, &is_anim, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Bounds query with NULL out_w and NULL out_h */
  rc = cupertino_menu_bar_extra_get_bounds(extra, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_extra_get_bounds(extra, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_menu_bar_extra_destroy(extra);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Case: neither icon nor title -> content_w = 10.0f < 24.0f (clamp branch) */
  memset(&desc, 0, sizeof(desc));
  rc = cupertino_menu_bar_extra_create(dummy_engine, &desc, &extra);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(24.0f, extra->computed_width, 0.01f);
  rc = cupertino_menu_bar_extra_destroy(extra);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Case: invalid animation_phase on create (< 0.0f or > 1.0f fallback to 0.0f)
   */
  memset(&desc, 0, sizeof(desc));
  desc.animation_phase = -2.0f;
  rc = cupertino_menu_bar_extra_create(dummy_engine, &desc, &extra);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_bar_extra_is_animating(extra, &is_anim, &phase);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.0f, phase, 0.01f);
  rc = cupertino_menu_bar_extra_destroy(extra);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&desc, 0, sizeof(desc));
  desc.animation_phase = 2.0f;
  rc = cupertino_menu_bar_extra_create(dummy_engine, &desc, &extra);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_bar_extra_is_animating(extra, &is_anim, &phase);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.0f, phase, 0.01f);
  rc = cupertino_menu_bar_extra_destroy(extra);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_menu_bar_extra_mock_recompute_fail;
  g_cupertino_menu_bar_extra_mock_recompute_fail = 1;
  memset(&desc, 0, sizeof(desc));
  rc = cupertino_menu_bar_extra_create(dummy_engine, &desc, &extra);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, extra);
  g_cupertino_menu_bar_extra_mock_recompute_fail = 0;
#endif

  PASS();
}

SUITE(cupertino_menu_bar_extra_suite) {
  RUN_TEST(test_menu_bar_extra_lifecycle);
  RUN_TEST(test_menu_bar_extra_updates_and_animation);
  RUN_TEST(test_menu_bar_extra_oom);
  RUN_TEST(test_menu_bar_extra_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_menu_bar_extra_suite);
  GREATEST_MAIN_END();
}
