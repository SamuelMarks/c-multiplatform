/**
 * @file test_cupertino_control_center.c
 * @brief Unit tests for iOS 18 Control Center modular controls conforming to
 * Apple HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_control_center.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

TEST test_control_center_module_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_control_center_module_descriptor desc;
  struct cupertino_control_center_module *mod = NULL;
  struct ui_button_base *btn = NULL;
  struct ui_slider_base *sld = NULL;
  float w = 0.0f;
  float h = 0.0f;
  float r = 0.0f;
  int is_act = 0;
  int is_exp = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.layout = CUPERTINO_CONTROL_CENTER_LAYOUT_1X1_CIRCULAR;
  desc.title = "Wi-Fi";
  desc.subtitle = "Home Network";
  desc.symbol_name = "wifi";
  desc.is_active = 1;
  desc.has_slider = 0;
  desc.active_tint_color = 0;

  /* Null checks */
  rc = cupertino_control_center_module_create(NULL, &desc, &mod);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_create(dummy_engine, NULL, &mod);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation 1x1 circular */
  rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, mod);

  rc = cupertino_control_center_module_is_active(mod, &is_act);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_act);

  rc = cupertino_control_center_module_get_bounds(mod, &w, &h, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(60.0f, w, 0.01f);
  ASSERT_IN_RANGE(60.0f, h, 0.01f);
  ASSERT_IN_RANGE(30.0f, r, 0.01f);

  rc = cupertino_control_center_module_get_button_base(mod, &btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, btn);

  rc = cupertino_control_center_module_get_slider_base(mod, &sld);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(NULL, sld);

  /* Toggle active */
  rc = cupertino_control_center_module_toggle(mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_control_center_module_is_active(mod, &is_act);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_act);

  /* Set same active state (no haptic/appearance change branch) */
  rc = cupertino_control_center_module_set_active(mod, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_control_center_module_set_active(mod, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_control_center_module_is_active(mod, &is_act);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_act);

  /* Test expansion on module without slider (is_expanded true, has_slider
   * false) */
  rc = cupertino_control_center_module_set_expanded(mod, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_control_center_module_set_expanded(mod, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null and invalid checks */
  rc = cupertino_control_center_module_set_active(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_is_active(NULL, &is_act);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_is_active(mod, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_toggle(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_set_expanded(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_is_expanded(NULL, &is_exp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_is_expanded(mod, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_get_bounds(NULL, &w, &h, &r);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_get_bounds(mod, NULL, &h, &r);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_get_bounds(mod, &w, NULL, &r);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_get_bounds(mod, &w, &h, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_get_button_base(NULL, &btn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_get_button_base(mod, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_get_slider_base(NULL, &sld);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_get_slider_base(mod, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy */
  rc = cupertino_control_center_module_destroy(mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_control_center_module_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_control_center_layouts_and_slider(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_control_center_module_descriptor desc;
  struct cupertino_control_center_module *mod = NULL;
  struct ui_slider_base *sld = NULL;
  float w = 0.0f;
  float h = 0.0f;
  float r = 0.0f;
  float val = 0.0f;
  int is_exp = 0;
  ui_error_t rc;

  /* 1x1 Squircle */
  memset(&desc, 0, sizeof(desc));
  desc.layout = CUPERTINO_CONTROL_CENTER_LAYOUT_1X1_SQUIRCLE;
  desc.title = "Torch";
  desc.symbol_name = "flashlight.on.fill";
  desc.has_slider = 1;
  desc.initial_slider_value = 0.75f;
  desc.active_tint_color = UI_COLOR_ARGB(255, 255, 149, 0);

  rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test out of range initial_slider_value branches (< 0.0f and > 1.0f) */
  {
    struct cupertino_control_center_module *mod_clamp = NULL;
    desc.initial_slider_value = -0.5f;
    rc =
        cupertino_control_center_module_create(dummy_engine, &desc, &mod_clamp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(0.0f, mod_clamp->slider_value);
    rc = cupertino_control_center_module_destroy(mod_clamp);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    desc.initial_slider_value = 1.5f;
    rc =
        cupertino_control_center_module_create(dummy_engine, &desc, &mod_clamp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(0.0f, mod_clamp->slider_value);
    rc = cupertino_control_center_module_destroy(mod_clamp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = cupertino_control_center_module_get_bounds(mod, &w, &h, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(60.0f, w, 0.01f);
  ASSERT_IN_RANGE(60.0f, h, 0.01f);
  ASSERT_IN_RANGE(18.0f, r, 0.01f);

  rc = cupertino_control_center_module_get_slider_base(mod, &sld);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, sld);

  rc = cupertino_control_center_module_get_slider_value(mod, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.75f, val, 0.01f);

  rc = cupertino_control_center_module_set_slider_value(mod, 0.5f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_control_center_module_get_slider_value(mod, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.5f, val, 0.01f);

  /* Slider value checks */
  rc = cupertino_control_center_module_set_slider_value(NULL, 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_set_slider_value(mod, -0.1f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_set_slider_value(mod, 1.1f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_get_slider_value(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_get_slider_value(mod, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Expansion */
  rc = cupertino_control_center_module_set_expanded(mod, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_control_center_module_is_expanded(mod, &is_exp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_exp);

  rc = cupertino_control_center_module_get_bounds(mod, &w, &h, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(60.0f, w, 0.01f);
  ASSERT_IN_RANGE(180.0f, h, 0.01f);
  ASSERT_IN_RANGE(28.0f, r, 0.01f);

  rc = cupertino_control_center_module_set_expanded(mod, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_control_center_module_destroy(mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 2x1 Pill */
  memset(&desc, 0, sizeof(desc));
  desc.layout = CUPERTINO_CONTROL_CENTER_LAYOUT_2X1_PILL;
  rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_control_center_module_get_bounds(mod, &w, &h, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(128.0f, w, 0.01f);
  ASSERT_IN_RANGE(60.0f, h, 0.01f);
  ASSERT_IN_RANGE(18.0f, r, 0.01f);
  rc = cupertino_control_center_module_destroy(mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 2x2 Macro */
  memset(&desc, 0, sizeof(desc));
  desc.layout = CUPERTINO_CONTROL_CENTER_LAYOUT_2X2_MACRO;
  rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_control_center_module_get_bounds(mod, &w, &h, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(128.0f, w, 0.01f);
  ASSERT_IN_RANGE(128.0f, h, 0.01f);
  ASSERT_IN_RANGE(20.0f, r, 0.01f);
  rc = cupertino_control_center_module_destroy(mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Unknown layout to hit default branch */
  memset(&desc, 0, sizeof(desc));
  desc.layout = (enum cupertino_control_center_layout)999;
  rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_control_center_module_destroy(mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_control_center_symbol_morph(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_control_center_module_descriptor desc;
  struct cupertino_control_center_module *mod = NULL;
  const char *from = NULL;
  const char *to = NULL;
  float progress = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.layout = CUPERTINO_CONTROL_CENTER_LAYOUT_1X1_CIRCULAR;
  desc.symbol_name = "play.fill";

  rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Invalid morph calls */
  rc = cupertino_control_center_module_set_symbol_morph(NULL, "play.fill",
                                                        "pause.fill", 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_set_symbol_morph(mod, NULL, "pause.fill",
                                                        0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_set_symbol_morph(mod, "play.fill", NULL,
                                                        0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_set_symbol_morph(mod, "play.fill",
                                                        "pause.fill", -0.1f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_module_set_symbol_morph(mod, "play.fill",
                                                        "pause.fill", 1.1f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid morph */
  rc = cupertino_control_center_module_set_symbol_morph(mod, "play.fill",
                                                        "pause.fill", 0.65f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_control_center_module_get_symbol_morph(mod, &from, &to,
                                                        &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("play.fill", from);
  ASSERT_STR_EQ("pause.fill", to);
  ASSERT_IN_RANGE(0.65f, progress, 0.01f);

  rc = cupertino_control_center_module_destroy(mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_control_center_grid(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_control_center_grid *grid = NULL;
  struct cupertino_control_center_module_descriptor desc;
  struct cupertino_control_center_module *mod1 = NULL;
  struct cupertino_control_center_module *mod2 = NULL;
  struct cupertino_control_center_module *ret_mod = NULL;
  size_t count = 0;
  float total_w = 0.0f;
  float total_h = 0.0f;
  int col = 0;
  int row = 0;
  ui_error_t rc;

  rc = cupertino_control_center_grid_create(NULL, &grid);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_grid_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_control_center_grid_create(dummy_engine, &grid);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, grid);

  /* Empty grid bounds */
  rc = cupertino_control_center_grid_get_bounds(grid, &total_w, &total_h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(32.0f, total_w, 0.01f);
  ASSERT_IN_RANGE(32.0f, total_h, 0.01f);

  memset(&desc, 0, sizeof(desc));
  desc.layout = CUPERTINO_CONTROL_CENTER_LAYOUT_1X1_CIRCULAR;
  rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Single 1x1 item in grid so max_col == 1 and max_row == 1 (false branches of
   * max_col > 1 and max_row > 1) */
  rc = cupertino_control_center_grid_add_module(grid, mod1, 0, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_control_center_grid_get_bounds(grid, &total_w, &total_h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  grid->item_count = 0; /* Reset for subsequent tests */

  desc.layout = CUPERTINO_CONTROL_CENTER_LAYOUT_2X1_PILL;
  rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_control_center_grid_add_module(NULL, mod1, 0, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_grid_add_module(grid, NULL, 0, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_grid_add_module(grid, mod1, -1, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_grid_add_module(grid, mod1, 0, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_control_center_grid_add_module(grid, mod1, 0, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_control_center_grid_add_module(grid, mod2, 1, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_control_center_grid_get_module_count(grid, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, count);

  rc = cupertino_control_center_grid_get_module_at(grid, 1, &ret_mod, &col,
                                                   &row);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(mod2, ret_mod);
  ASSERT_EQ(1, col);
  ASSERT_EQ(0, row);

  rc = cupertino_control_center_grid_get_bounds(grid, &total_w, &total_h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_GT(total_w, 100.0f);
  ASSERT_GT(total_h, 50.0f);

  /* Null checks on grid queries */
  rc = cupertino_control_center_grid_get_module_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_grid_get_module_count(grid, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_grid_get_module_at(NULL, 0, &ret_mod, NULL,
                                                   NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_grid_get_module_at(grid, 0, NULL, NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_grid_get_module_at(grid, 99, &ret_mod, NULL,
                                                   NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_grid_get_bounds(NULL, &total_w, &total_h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_grid_get_bounds(grid, NULL, &total_h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_grid_get_bounds(grid, &total_w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_control_center_grid_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Fill up grid to capacity */
  while (grid->item_count < CUPERTINO_CONTROL_CENTER_MAX_GRID_MODULES) {
    rc = cupertino_control_center_grid_add_module(grid, mod1, 0, 0);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  /* Exceed capacity */
  rc = cupertino_control_center_grid_add_module(grid, mod1, 0, 0);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  rc = cupertino_control_center_module_destroy(mod1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_control_center_module_destroy(mod2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_control_center_grid_destroy(grid);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_control_center_oom(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_control_center_module_descriptor desc;
  struct cupertino_control_center_module *mod = NULL;
  struct cupertino_control_center_grid *grid = NULL;
  ui_error_t rc;
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_control_center_mock_button_create_fail;
  extern int g_cupertino_control_center_mock_slider_create_fail;
  extern int g_cupertino_control_center_mock_button_destroy_fail;
  extern int g_cupertino_control_center_mock_slider_destroy_fail;
  extern int g_cupertino_control_center_mock_haptic_fail;
  extern int g_cupertino_control_center_mock_geometry_fail;
  extern int g_cupertino_control_center_mock_appearance_fail;
#endif

  memset(&desc, 0, sizeof(desc));
  desc.layout = CUPERTINO_CONTROL_CENTER_LAYOUT_1X1_CIRCULAR;

  g_malloc_fail_countdown = 0;
  rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod);
  g_malloc_fail_countdown = -1;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, mod);

  g_malloc_fail_countdown = 0;
  rc = cupertino_control_center_grid_create(dummy_engine, &grid);
  g_malloc_fail_countdown = -1;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, grid);

#ifdef UI_TEST_MOCK_ALLOC
  /* Test geometry fail in module_create */
  g_cupertino_control_center_mock_geometry_fail = 1;
  rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, mod);
  g_cupertino_control_center_mock_geometry_fail = 0;

  /* Test appearance fail in module_create */
  g_cupertino_control_center_mock_appearance_fail = 1;
  rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, mod);
  g_cupertino_control_center_mock_appearance_fail = 0;

  /* Test button create fail in module_create */
  g_cupertino_control_center_mock_button_create_fail = 1;
  rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, mod);
  g_cupertino_control_center_mock_button_create_fail = 0;

  /* Test slider create fail in module_create (with has_slider = 1) */
  desc.has_slider = 1;
  g_cupertino_control_center_mock_slider_create_fail = 1;
  rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, mod);
  g_cupertino_control_center_mock_slider_create_fail = 0;

  /* Create valid module for haptic / appearance failure in set_active */
  rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, mod);

  /* mod->is_active starts at 0 since desc.is_active is 0 */
  g_cupertino_control_center_mock_haptic_fail = 1;
  rc = cupertino_control_center_module_set_active(mod, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_control_center_mock_haptic_fail = 0;

  /* Reset is_active back to 0 so next call from 0 to 1 triggers
   * update_appearance */
  mod->is_active = 0;
  g_cupertino_control_center_mock_appearance_fail = 1;
  rc = cupertino_control_center_module_set_active(mod, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_control_center_mock_appearance_fail = 0;

  /* Destroy with failure branches in sub-components */
  g_cupertino_control_center_mock_button_destroy_fail = 1;
  g_cupertino_control_center_mock_slider_destroy_fail = 1;
  rc = cupertino_control_center_module_destroy(mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_control_center_mock_button_destroy_fail = 0;
  g_cupertino_control_center_mock_slider_destroy_fail = 0;
  mod = NULL;
#endif

  /* Test cupertino_control_center_module_get_symbol_morph NULL checks */
  rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  {
    const char *from_str, *to_str;
    float prog;
    rc = cupertino_control_center_module_get_symbol_morph(NULL, &from_str,
                                                          &to_str, &prog);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = cupertino_control_center_module_get_symbol_morph(mod, NULL, &to_str,
                                                          &prog);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = cupertino_control_center_module_get_symbol_morph(mod, &from_str, NULL,
                                                          &prog);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = cupertino_control_center_module_get_symbol_morph(mod, &from_str,
                                                          &to_str, NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  }
  rc = cupertino_control_center_module_destroy(mod);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  mod = NULL;

  /* Test 2x2 macro in grid bounds */
  {
    struct cupertino_control_center_module *mod_2x2 = NULL;
    float gw, gh;
    memset(&desc, 0, sizeof(desc));
    desc.layout = CUPERTINO_CONTROL_CENTER_LAYOUT_2X2_MACRO;
    /* Explicitly leave title, subtitle, symbol_name NULL and active_tint_color
     * non-zero */
    desc.title = NULL;
    desc.subtitle = NULL;
    desc.symbol_name = NULL;
    desc.active_tint_color = UI_COLOR_ARGB(255, 10, 20, 30);
    rc = cupertino_control_center_module_create(dummy_engine, &desc, &mod_2x2);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = cupertino_control_center_grid_create(dummy_engine, &grid);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    /* Multi-item grid where second item does not exceed max_col/max_row */
    rc = cupertino_control_center_grid_add_module(grid, mod_2x2, 5, 5);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_control_center_grid_add_module(grid, mod_2x2, 0, 0);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_control_center_grid_get_bounds(grid, &gw, &gh);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* Test grid item with module == NULL in get_bounds */
    grid->items[0].module = NULL;
    rc = cupertino_control_center_grid_get_bounds(grid, &gw, &gh);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    grid->items[0].module = mod_2x2;

    rc = cupertino_control_center_grid_get_bounds(grid, &gw, &gh);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_GT(gw, 100.0f);
    ASSERT_GT(gh, 100.0f);

    /* Test get_module_at with out_col and out_row NULL branches */
    {
      struct cupertino_control_center_module *qm = NULL;
      int qc = 0;
      int qr = 0;
      rc = cupertino_control_center_grid_get_module_at(grid, 0, &qm, NULL, &qr);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      rc = cupertino_control_center_grid_get_module_at(grid, 0, &qm, &qc, NULL);
      ASSERT_EQ(UI_ERROR_NONE, rc);
    }

    rc = cupertino_control_center_module_destroy(mod_2x2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_control_center_grid_destroy(grid);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Test destroy with module->button_base and slider_base already NULL */
  {
    struct cupertino_control_center_module *mod_null_base = NULL;
    memset(&desc, 0, sizeof(desc));
    desc.layout = CUPERTINO_CONTROL_CENTER_LAYOUT_1X1_CIRCULAR;
    rc = cupertino_control_center_module_create(dummy_engine, &desc,
                                                &mod_null_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_button_base_destroy(mod_null_base->button_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    mod_null_base->button_base = NULL;
    rc = cupertino_control_center_module_destroy(mod_null_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  PASS();
}

SUITE(cupertino_control_center_suite) {
  RUN_TEST(test_control_center_module_lifecycle);
  RUN_TEST(test_control_center_layouts_and_slider);
  RUN_TEST(test_control_center_symbol_morph);
  RUN_TEST(test_control_center_grid);
  RUN_TEST(test_control_center_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_control_center_suite);
  GREATEST_MAIN_END();
}
