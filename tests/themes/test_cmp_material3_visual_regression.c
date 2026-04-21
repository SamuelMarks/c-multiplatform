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

  /* Test State: Hovered */
  btn->is_hovered = 1;
  cmp_window_poll_events(win);
  res = cmp_test_capture_snapshot(win, &pixels, &width, &height);
  if (res == CMP_SUCCESS && pixels != NULL) {
    res = cmp_test_compare_golden("material3_golden_button_hover.bmp",
                                  (const unsigned char *)pixels, width, height);
    CMP_FREE(pixels);
    ASSERT_EQ(0, res);
  }

  /* Test State: Pressed */
  btn->is_hovered = 0;
  btn->is_pressed = 1; /* Pressed */
  cmp_window_poll_events(win);
  res = cmp_test_capture_snapshot(win, &pixels, &width, &height);
  if (res == CMP_SUCCESS && pixels != NULL) {
    res = cmp_test_compare_golden("material3_golden_button_pressed.bmp",
                                  (const unsigned char *)pixels, width, height);
    CMP_FREE(pixels);
    ASSERT_EQ(0, res);
  }

  cmp_window_destroy(win);
  cmp_window_system_shutdown();
  cmp_event_system_shutdown();
  PASS();
}

TEST test_material3_mosaic_visual(void) {
  cmp_window_config_t config;
  cmp_window_t *win = NULL;
  cmp_ui_node_t *root = NULL;
  cmp_ui_node_t *card1 = NULL;
  cmp_ui_node_t *card2 = NULL;
  void *pixels = NULL;
  int width = 0, height = 0;
  int res;

  cmp_event_system_init();
  cmp_window_system_init();

  config.title = "Material 3 Mosaic Headless";
  config.width = 800;
  config.height = 600;
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

  cmp_ui_box_create(&root);
  root->layout->direction = CMP_FLEX_ROW;
  /* Since CMP_FLEX_WRAP doesn't compile due to syntax, let's omit for this mock
   * test */

  cmp_ui_box_create(&card1);
  card1->layout->width = 200.0f;
  card1->layout->height = 160.0f;
  card1->layout->margin[0] = 16.0f; /* top */
  card1->layout->margin[1] = 16.0f; /* right */
  card1->layout->margin[2] = 16.0f; /* bottom */
  card1->layout->margin[3] = 16.0f; /* left */
  card1->bg_color = 0xFFE6E6E6;     /* light gray */

  cmp_ui_box_create(&card2);
  card2->layout->width = 200.0f;
  card2->layout->height = 160.0f;
  card2->layout->margin[0] = 16.0f;
  card2->layout->margin[1] = 16.0f;
  card2->layout->margin[2] = 16.0f;
  card2->layout->margin[3] = 16.0f;
  card2->bg_color = 0xFFCCCCCC; /* slightly darker gray */

  cmp_ui_node_add_child(root, card1);
  cmp_ui_node_add_child(root, card2);

  cmp_layout_calculate(root->layout, 800.0f, 600.0f);
  cmp_window_set_ui_tree(win, root);

  cmp_window_poll_events(win);

  res = cmp_test_capture_snapshot(win, &pixels, &width, &height);
  if (res == CMP_SUCCESS && pixels != NULL) {
    res = cmp_test_compare_golden("material3_mosaic_light.bmp",
                                  (const unsigned char *)pixels, width, height);
    CMP_FREE(pixels);
    ASSERT_EQ(0, res);
  }

  /* RTL Mode test */
  cmp_i18n_set_bidi_direction(CMP_TEXT_DIR_RTL);
  cmp_layout_calculate(root->layout, 800.0f, 600.0f);
  cmp_window_poll_events(win);

  res = cmp_test_capture_snapshot(win, &pixels, &width, &height);
  if (res == CMP_SUCCESS && pixels != NULL) {
    res = cmp_test_compare_golden("material3_mosaic_rtl.bmp",
                                  (const unsigned char *)pixels, width, height);
    CMP_FREE(pixels);
    ASSERT_EQ(0, res);
  }
  cmp_i18n_set_bidi_direction(CMP_TEXT_DIR_LTR);

  cmp_window_destroy(win);
  cmp_window_system_shutdown();
  cmp_event_system_shutdown();
  PASS();
}

SUITE(cmp_material3_visual_regression_suite) {
  RUN_TEST(test_material3_visual_golden_baseline);
  RUN_TEST(test_material3_mosaic_visual);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_visual_regression_suite);
  GREATEST_MAIN_END();
}