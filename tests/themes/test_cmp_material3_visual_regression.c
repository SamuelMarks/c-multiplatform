#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "cmp_ui_action_button.h"
#include "../test_visual_regression_utils.h"
/* clang-format on */

TEST test_material3_visual_golden_baseline(void) {
  cmp_window_config_t config;
  cmp_window_t *win = NULL;
  cmp_ui_action_button_t *btn_obj = NULL;
  cmp_ui_node_t *btn = NULL;
  void *pixels = NULL;
  int width = 0, height = 0;
  int res;

  cmp_event_system_init();
  cmp_window_system_init();

  config.title = "Material 3 Visual Regression Test";
  config.width = 400;
  config.height = 300;
  config.x = -1;
  config.y = -1;
  config.hidden = 1;
  config.frameless = 1;
  config.use_legacy_backend = 1;

  res = cmp_window_create(&config, &win);
  if (res != CMP_SUCCESS) {
    cmp_window_system_shutdown();
    cmp_event_system_shutdown();
    SKIP();
  }

  res = cmp_ui_action_button_create(&btn_obj, "Golden Button",
                                    CMP_UI_ACTION_BUTTON_STYLE_FILLED);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_ui_action_button_get_node(btn_obj, &btn);
  btn->design_language_override = 1; /* CMP_THEME_MATERIAL3_OVERRIDE */

  cmp_layout_calculate(btn->layout, 400.0f, 300.0f);
  cmp_window_set_ui_tree(win, btn);

  cmp_window_poll_events(win);

  res = cmp_test_capture_snapshot(win, &pixels, &width, &height);
  if (res == CMP_SUCCESS && pixels != NULL) {
    ASSERT(width > 0);
    ASSERT(height > 0);
    res = cmp_test_compare_golden("material3_golden_button.bmp",
                                  (const unsigned char *)pixels, width, height);
    CMP_FREE(pixels);
    ASSERT_EQ(0, res);
  }
  cmp_window_destroy(win);
  cmp_window_system_shutdown();
  cmp_event_system_shutdown();
  PASS();
}

SUITE(cmp_material3_visual_regression_suite) {
  RUN_TEST(test_material3_visual_golden_baseline);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_visual_regression_suite);
  GREATEST_MAIN_END();
}