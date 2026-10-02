/**
 * @file test_cupertino_status_bar.c
 * @brief Unit tests for iOS Safe Area & Translucent Status Bar conforming to
 * Apple HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_status_bar.h"
#include "ui_arena.h"
#include "ui_safe_area_manager.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

TEST test_status_bar_lifecycle_and_presets(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_status_bar_descriptor desc;
  struct cupertino_status_bar *bar = NULL;
  struct ui_safe_area_insets insets;
  enum cupertino_status_bar_style style;
  const char *time_str = NULL;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_STATUS_BAR_STYLE_DEFAULT;
  desc.preset = CUPERTINO_DEVICE_PRESET_DYNAMIC_ISLAND;
  desc.initial_time_text = "9:41";
  desc.initial_battery_level = 0.85f;
  desc.is_charging = 1;
  desc.cellular_bars = 4;
  desc.wifi_bars = 3;

  /* Null checks */
  rc = cupertino_status_bar_create(NULL, &desc, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_create(dummy_engine, NULL, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation: Dynamic Island preset */
  rc = cupertino_status_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, bar);

  rc = cupertino_status_bar_get_insets(bar, &insets);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(54.0f, insets.top, 0.01f);
  ASSERT_IN_RANGE(34.0f, insets.bottom, 0.01f);

  rc = cupertino_status_bar_get_bounds(bar, 393.0f, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(393.0f, w, 0.01f);
  ASSERT_IN_RANGE(54.0f, h, 0.01f);

  rc = cupertino_status_bar_get_style(bar, &style);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_STATUS_BAR_STYLE_DEFAULT, style);

  rc = cupertino_status_bar_get_time_text(bar, &time_str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("9:41", time_str);

  rc = cupertino_status_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_status_bar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Preset: Notch */
  desc.preset = CUPERTINO_DEVICE_PRESET_NOTCH;
  rc = cupertino_status_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_status_bar_get_insets(bar, &insets);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(47.0f, insets.top, 0.01f);
  ASSERT_IN_RANGE(34.0f, insets.bottom, 0.01f);
  rc = cupertino_status_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Preset: Classic */
  desc.preset = CUPERTINO_DEVICE_PRESET_CLASSIC;
  rc = cupertino_status_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_status_bar_get_insets(bar, &insets);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(20.0f, insets.top, 0.01f);
  ASSERT_IN_RANGE(0.0f, insets.bottom, 0.01f);
  rc = cupertino_status_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Preset: iPad */
  desc.preset = CUPERTINO_DEVICE_PRESET_IPAD;
  rc = cupertino_status_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_status_bar_get_insets(bar, &insets);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(24.0f, insets.top, 0.01f);
  ASSERT_IN_RANGE(20.0f, insets.bottom, 0.01f);
  rc = cupertino_status_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Creation with invalid initial values to hit fallback branches */
  memset(&desc, 0, sizeof(desc));
  desc.preset =
      (enum cupertino_hardware_device_preset)99; /* default preset branch */
  desc.initial_time_text = "";                   /* empty time text fallback */
  desc.initial_battery_level = -0.5f;            /* fallback to 1.0f */
  desc.cellular_bars = -1;                       /* fallback to 4 */
  desc.wifi_bars = -1;                           /* fallback to 3 */
  rc = cupertino_status_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(1.0f, bar->battery_level, 0.001f);
  ASSERT_EQ(4, bar->cellular_bars);
  ASSERT_EQ(3, bar->wifi_bars);
  ASSERT_STR_EQ("9:41", bar->time_text);
  rc = cupertino_status_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Creation with battery > 1.0f, cellular > 4, wifi > 3, initial_time_text =
   * NULL */
  memset(&desc, 0, sizeof(desc));
  desc.initial_time_text = NULL;
  desc.initial_battery_level = 1.5f;
  desc.cellular_bars = 5;
  desc.wifi_bars = 4;
  rc = cupertino_status_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(1.0f, bar->battery_level, 0.001f);
  ASSERT_EQ(4, bar->cellular_bars);
  ASSERT_EQ(3, bar->wifi_bars);
  ASSERT_STR_EQ("9:41", bar->time_text);
  rc = cupertino_status_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_status_bar_readouts_and_styles(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_status_bar_descriptor desc;
  struct cupertino_status_bar *bar = NULL;
  struct ui_safe_area_insets custom_insets;
  struct ui_safe_area_insets out_insets;
  enum cupertino_status_bar_style style;
  const char *time_str = NULL;
  float batt = 0.0f;
  int charging = 0;
  int cell = 0;
  int wifi = 0;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.preset = CUPERTINO_DEVICE_PRESET_DYNAMIC_ISLAND;
  rc = cupertino_status_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Style toggle */
  rc = cupertino_status_bar_set_style(bar,
                                      CUPERTINO_STATUS_BAR_STYLE_LIGHT_CONTENT);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_status_bar_get_style(bar, &style);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_STATUS_BAR_STYLE_LIGHT_CONTENT, style);

  rc = cupertino_status_bar_set_style(bar,
                                      CUPERTINO_STATUS_BAR_STYLE_TRANSLUCENT);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_status_bar_get_style(bar, &style);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_STATUS_BAR_STYLE_TRANSLUCENT, style);

  /* Clock text */
  rc = cupertino_status_bar_set_time_text(bar, "10:30");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_status_bar_get_time_text(bar, &time_str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("10:30", time_str);

  rc = cupertino_status_bar_set_time_text(bar, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_status_bar_get_time_text(bar, &time_str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("9:41", time_str);

  /* Empty clock string fallback */
  rc = cupertino_status_bar_set_time_text(bar, "");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_status_bar_get_time_text(bar, &time_str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("9:41", time_str);

  /* Battery */
  rc = cupertino_status_bar_set_battery(bar, 0.42f, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_status_bar_get_battery(bar, &batt, &charging);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.42f, batt, 0.01f);
  ASSERT_EQ(0, charging);

  /* Battery bounds check */
  rc = cupertino_status_bar_set_battery(bar, -0.1f, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_set_battery(bar, 1.1f, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Signal bars */
  rc = cupertino_status_bar_set_signal(bar, 3, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_status_bar_get_signal(bar, &cell, &wifi);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, cell);
  ASSERT_EQ(2, wifi);

  /* Signal bounds check */
  rc = cupertino_status_bar_set_signal(bar, -1, 2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_set_signal(bar, 5, 2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_set_signal(bar, 3, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_set_signal(bar, 3, 4);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Bounds when insets.top == 0.0f */
  bar->insets.top = 0.0f;
  rc = cupertino_status_bar_get_bounds(bar, 393.0f, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(20.0f, h, 0.01f);

  /* Custom insets application */
  custom_insets.top = 60.0f;
  custom_insets.bottom = 40.0f;
  custom_insets.left = 12.0f;
  custom_insets.right = 12.0f;
  rc = cupertino_status_bar_apply_insets(bar, &custom_insets);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_status_bar_get_insets(bar, &out_insets);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(60.0f, out_insets.top, 0.01f);
  ASSERT_IN_RANGE(40.0f, out_insets.bottom, 0.01f);

  /* Safe area manager integration */
  {
    struct ui_arena *arena = NULL;
    struct ui_safe_area_manager *mgr = NULL;
    struct ui_safe_area_insets mgr_insets;

    rc = ui_arena_create(1024 * 64, &arena);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_safe_area_manager_create(arena, &mgr);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    mgr_insets.top = 59.0f;
    mgr_insets.bottom = 34.0f;
    mgr_insets.left = 0.0f;
    mgr_insets.right = 0.0f;
    rc = ui_safe_area_manager_set_insets(mgr, &mgr_insets);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* Attach manager */
    rc = cupertino_status_bar_set_safe_area_manager(bar, mgr);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_IN_RANGE(59.0f, bar->insets.top, 0.01f);

#ifdef UI_TEST_MOCK_ALLOC
    {
      extern int g_cupertino_status_bar_mock_get_insets_fail;
      g_cupertino_status_bar_mock_get_insets_fail = 1;
      rc = cupertino_status_bar_set_safe_area_manager(bar, mgr);
      ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
      g_cupertino_status_bar_mock_get_insets_fail = 0;
    }
#endif

    /* Detach manager */
    rc = cupertino_status_bar_set_safe_area_manager(bar, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(NULL, bar->manager);

    rc = ui_safe_area_manager_destroy(mgr);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_arena_destroy(arena);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Null checks */
  rc = cupertino_status_bar_set_style(NULL, CUPERTINO_STATUS_BAR_STYLE_DEFAULT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_style(NULL, &style);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_style(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_apply_insets(NULL, &custom_insets);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_apply_insets(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_insets(NULL, &out_insets);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_insets(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_set_safe_area_manager(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_set_time_text(NULL, "9:41");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_time_text(NULL, &time_str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_time_text(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_set_battery(NULL, 0.5f, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_battery(NULL, &batt, &charging);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_battery(bar, NULL, &charging);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_battery(bar, &batt, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_set_signal(NULL, 2, 2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_signal(NULL, &cell, &wifi);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_signal(bar, NULL, &wifi);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_signal(bar, &cell, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_bounds(NULL, 393.0f, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_bounds(bar, 0.0f, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_bounds(bar, 393.0f, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_status_bar_get_bounds(bar, 393.0f, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_status_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_status_bar_oom(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_status_bar_descriptor desc;
  struct cupertino_status_bar *bar = NULL;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  g_malloc_fail_countdown = 0;
  rc = cupertino_status_bar_create(dummy_engine, &desc, &bar);
  g_malloc_fail_countdown = -1;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, bar);

  PASS();
}

SUITE(cupertino_status_bar_suite) {
  RUN_TEST(test_status_bar_lifecycle_and_presets);
  RUN_TEST(test_status_bar_readouts_and_styles);
  RUN_TEST(test_status_bar_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_status_bar_suite);
  GREATEST_MAIN_END();
}
