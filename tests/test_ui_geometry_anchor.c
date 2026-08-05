/* clang-format off */
#include "ui_geometry_anchor.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static int s_tests_passed = 0;
static int s_tests_failed = 0;

#define ASSERT_TRUE(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond);         \
      s_tests_failed++;                                                        \
    } else {                                                                   \
      s_tests_passed++;                                                        \
    }                                                                          \
  } while (0)

#define ASSERT_FLOAT_EQ(expected, actual)                                      \
  do {                                                                         \
    if (fabs((expected) - (actual)) > 0.001f) {                                \
      fprintf(stderr, "FAIL: %s:%d: expected %f, got %f\n", __FILE__,          \
              __LINE__, (float)(expected), (float)(actual));                   \
      s_tests_failed++;                                                        \
    } else {                                                                   \
      s_tests_passed++;                                                        \
    }                                                                          \
  } while (0)

static ui_error_t test_invalid_args(void) {
  struct ui_layout_node target;
  struct ui_layout_node overlay;
  struct ui_anchor_config config;
  float out_x, out_y;

  ASSERT_TRUE(ui_geometry_anchor_compute(NULL, &overlay, &config, 100, 100,
                                         &out_x,
                                         &out_y) == UI_ERROR_INVALID_ARGUMENT);
  ASSERT_TRUE(ui_geometry_anchor_compute(&target, NULL, &config, 100, 100,
                                         &out_x,
                                         &out_y) == UI_ERROR_INVALID_ARGUMENT);
  ASSERT_TRUE(ui_geometry_anchor_compute(&target, &overlay, NULL, 100, 100,
                                         &out_x,
                                         &out_y) == UI_ERROR_INVALID_ARGUMENT);
  ASSERT_TRUE(ui_geometry_anchor_compute(&target, &overlay, &config, 100, 100,
                                         NULL,
                                         &out_y) == UI_ERROR_INVALID_ARGUMENT);
  ASSERT_TRUE(ui_geometry_anchor_compute(&target, &overlay, &config, 100, 100,
                                         &out_x,
                                         NULL) == UI_ERROR_INVALID_ARGUMENT);
  return UI_ERROR_NONE;
}

static ui_error_t test_geometry_anchor_basic(void) {
  struct ui_layout_node target;
  struct ui_layout_node overlay;
  struct ui_anchor_config config;
  float out_x, out_y;
  ui_error_t err;

  memset(&target, 0, sizeof(target));
  memset(&overlay, 0, sizeof(overlay));

  target.x = 100.0f;
  target.y = 100.0f;
  target.width = 50.0f;
  target.height = 20.0f;

  overlay.width = 200.0f;
  overlay.height = 100.0f;

  /* Dropdown menu scenario: target bottom-start, overlay top-start */
  config.target_x = UI_ANCHOR_EDGE_START;
  config.target_y = UI_ANCHOR_EDGE_END;
  config.overlay_x = UI_ANCHOR_EDGE_START;
  config.overlay_y = UI_ANCHOR_EDGE_START;
  config.offset_x = 0.0f;
  config.offset_y = 5.0f; /* 5px gap */

  err = ui_geometry_anchor_compute(&target, &overlay, &config, 1000.0f, 1000.0f,
                                   &out_x, &out_y);
  ASSERT_TRUE(err == UI_ERROR_NONE);
  ASSERT_FLOAT_EQ(100.0f,
                  out_x); /* 100 (target start) - 0 (overlay start) + 0 */
  ASSERT_FLOAT_EQ(125.0f,
                  out_y); /* 100 + 20 (target end) - 0 (overlay start) + 5 */

  /* Tooltip scenario: center top of target, bottom center of overlay */
  config.target_x = UI_ANCHOR_EDGE_CENTER;
  config.target_y = UI_ANCHOR_EDGE_START;
  config.overlay_x = UI_ANCHOR_EDGE_CENTER;
  config.overlay_y = UI_ANCHOR_EDGE_END;
  config.offset_x = 0.0f;
  config.offset_y = -10.0f;

  err = ui_geometry_anchor_compute(&target, &overlay, &config, 1000.0f, 1000.0f,
                                   &out_x, &out_y);
  ASSERT_TRUE(err == UI_ERROR_NONE);
  ASSERT_FLOAT_EQ(25.0f,
                  out_x); /* Target center (125) - Overlay center (100) + 0 */
  ASSERT_FLOAT_EQ(0.0f, out_y); /* Target start (100) - Overlay end (100) - 10 =
                                   -10, clamped to 0.0 */
  return UI_ERROR_NONE;
}

static ui_error_t test_geometry_anchor_viewport_clamping(void) {
  struct ui_layout_node target;
  struct ui_layout_node overlay;
  struct ui_anchor_config config;
  float out_x, out_y;

  memset(&target, 0, sizeof(target));
  memset(&overlay, 0, sizeof(overlay));

  target.x = 10.0f;
  target.y = 10.0f;
  target.width = 50.0f;
  target.height = 20.0f;

  overlay.width = 200.0f;
  overlay.height = 100.0f;

  config.target_x = UI_ANCHOR_EDGE_START;
  config.target_y = UI_ANCHOR_EDGE_START;
  config.overlay_x = UI_ANCHOR_EDGE_END;
  config.overlay_y = UI_ANCHOR_EDGE_END;
  config.offset_x = 0.0f;
  config.offset_y = 0.0f;

  /* Raw computation would place overlay end at target start.
     X: 10 - 200 = -190
     Y: 10 - 100 = -90 */

  ui_geometry_anchor_compute(&target, &overlay, &config, 800.0f, 600.0f, &out_x,
                             &out_y);
  ASSERT_FLOAT_EQ(0.0f, out_x); /* Clamped to left edge */
  ASSERT_FLOAT_EQ(0.0f, out_y); /* Clamped to top edge */

  /* Force off right/bottom edges */
  target.x = 780.0f;
  target.y = 580.0f;
  config.target_x = UI_ANCHOR_EDGE_END;
  config.target_y = UI_ANCHOR_EDGE_END;
  config.overlay_x = UI_ANCHOR_EDGE_START;
  config.overlay_y = UI_ANCHOR_EDGE_START;

  /* Raw: X = 780 + 50 = 830. Overlay width = 200. Right edge = 1030
     Viewport is 800. Clamps right edge to 800 - 200 = 600. */
  ui_geometry_anchor_compute(&target, &overlay, &config, 800.0f, 600.0f, &out_x,
                             &out_y);
  ASSERT_FLOAT_EQ(600.0f, out_x);
  ASSERT_FLOAT_EQ(500.0f, out_y); /* 600 - 100 */
  return UI_ERROR_NONE;
}

static int run_extra_anchor(void);

int main(void) {
  test_invalid_args();
  test_geometry_anchor_basic();
  test_geometry_anchor_viewport_clamping();
  run_extra_anchor();

  printf("Tests passed: %d\n", s_tests_passed);
  printf("Tests failed: %d\n", s_tests_failed);

  if (s_tests_failed > 0) {
    return 1;
  }
  return 0;
}

static int run_extra_anchor(void) {
  /* test internal coverage for target center / end and overlay center / end
     without viewport constraints and with viewport constraints to ensure all
     if/else paths are covered */
  struct ui_layout_node target;
  struct ui_layout_node overlay;
  struct ui_anchor_config config;
  float out_x, out_y;

  memset(&target, 0, sizeof(target));
  memset(&overlay, 0, sizeof(overlay));

  target.x = 100.0f;
  target.y = 100.0f;
  target.width = 50.0f;
  target.height = 20.0f;

  overlay.width = 20.0f;
  overlay.height = 10.0f;

  config.target_x = UI_ANCHOR_EDGE_END;
  config.target_y = UI_ANCHOR_EDGE_CENTER;
  config.overlay_x = UI_ANCHOR_EDGE_CENTER;
  config.overlay_y = UI_ANCHOR_EDGE_END;
  config.offset_x = 0;
  config.offset_y = 0;

  ui_geometry_anchor_compute(&target, &overlay, &config, 0.0f, 0.0f, &out_x,
                             &out_y);

  config.overlay_y = UI_ANCHOR_EDGE_CENTER;
  ui_geometry_anchor_compute(&target, &overlay, &config, 0.0f, 0.0f, &out_x,
                             &out_y);
  return 0;
}
