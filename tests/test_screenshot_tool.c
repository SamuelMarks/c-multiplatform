/**
 * @file test_screenshot_tool.c
 * @brief Unit tests and OOM branch coverage for the 2D rasterizer and
 * screenshot tool registry.
 */

/* clang-format off */
#include "greatest.h"
#include "ui_test_mock_mem.h"
#include "design_system_renderer.h"
#include "rasterizer.h"
#include "ui_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_canvas_create_and_destroy(void) {
  struct canvas *c = NULL;
  ui_error_t rc;

  /* Invalid arguments */
  rc = canvas_create(0, 100, &c);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, c);

  rc = canvas_create(100, 0, &c);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, c);

  rc = canvas_create(100, 100, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = canvas_create(10, 10, &c);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(c != NULL);
  ASSERT_EQ(10, c->width);
  ASSERT_EQ(10, c->height);
  ASSERT(c->pixels != NULL);

  /* Destroy valid */
  rc = canvas_destroy(c);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy NULL */
  rc = canvas_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_canvas_create_oom(void) {
  struct canvas *c = NULL;
  ui_error_t rc;

  /* Fail first malloc (canvas structure) */
  g_malloc_fail_countdown = 0;
  rc = canvas_create(20, 20, &c);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, c);

  /* Fail second malloc (pixel buffer) */
  g_malloc_fail_countdown = 1;
  rc = canvas_create(20, 20, &c);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, c);

  g_malloc_fail_countdown = -1;
  PASS();
}

TEST test_canvas_clear(void) {
  struct canvas *c = NULL;
  ui_error_t rc;
  ui_color_t col = UI_COLOR_ARGB(255, 100, 150, 200);

  rc = canvas_clear(NULL, col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = canvas_create(4, 4, &c);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = canvas_clear(c, col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(100, c->pixels[0]);
  ASSERT_EQ(150, c->pixels[1]);
  ASSERT_EQ(200, c->pixels[2]);
  ASSERT_EQ(255, c->pixels[3]);

  rc = canvas_destroy(c);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_draw_primitives(void) {
  struct canvas *c = NULL;
  ui_error_t rc;
  ui_color_t col = UI_COLOR_ARGB(255, 50, 100, 150);

  rc = canvas_create(50, 50, &c);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Rounded rect */
  rc = draw_rounded_rect(NULL, 5.0f, 5.0f, 20.0f, 20.0f, 4.0f, col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = draw_rounded_rect(c, 5.0f, 5.0f, 0.0f, 20.0f, 4.0f, col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = draw_rounded_rect(c, 5.0f, 5.0f, 20.0f, -5.0f, 4.0f, col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = draw_rounded_rect(c, 5.0f, 5.0f, 20.0f, 20.0f, 4.0f, col);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Rounded rect stroke */
  rc =
      draw_rounded_rect_stroke(NULL, 5.0f, 5.0f, 20.0f, 20.0f, 4.0f, 1.0f, col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = draw_rounded_rect_stroke(c, 5.0f, 5.0f, 20.0f, 20.0f, 4.0f, 0.0f, col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = draw_rounded_rect_stroke(c, 5.0f, 5.0f, 20.0f, 20.0f, 4.0f, 1.5f, col);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Shadow */
  rc = draw_shadow(NULL, 5.0f, 5.0f, 20.0f, 20.0f, 4.0f, 2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = draw_shadow(c, 5.0f, 5.0f, 20.0f, 20.0f, 4.0f, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = draw_shadow(c, 5.0f, 5.0f, 20.0f, 20.0f, 4.0f, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = draw_shadow(c, 5.0f, 5.0f, 20.0f, 20.0f, 4.0f, 3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = draw_shadow(c, 5.0f, 5.0f, 20.0f, 20.0f, 4.0f, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Circle & Circle stroke */
  rc = draw_circle(NULL, 25.0f, 25.0f, 10.0f, col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = draw_circle(c, 25.0f, 25.0f, 0.0f, col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = draw_circle(c, 25.0f, 25.0f, 10.0f, col);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = draw_circle_stroke(NULL, 25.0f, 25.0f, 10.0f, 1.0f, col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = draw_circle_stroke(c, 25.0f, 25.0f, 0.0f, 1.0f, col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = draw_circle_stroke(c, 25.0f, 25.0f, 10.0f, 1.5f, col);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Line */
  rc = draw_line(NULL, 0.0f, 0.0f, 50.0f, 50.0f, 1.0f, col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = draw_line(c, 0.0f, 0.0f, 50.0f, 50.0f, 0.0f, col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = draw_line(c, 0.0f, 0.0f, 50.0f, 50.0f, 2.0f, col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = draw_line(c, 10.0f, 10.0f, 10.0f, 10.0f, 2.0f, col); /* zero len */
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = canvas_destroy(c);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_text_and_icons(void) {
  struct canvas *c = NULL;
  ui_error_t rc;
  float w = 0.0f;
  int icon_i;
  ui_color_t col = UI_COLOR_ARGB(255, 0, 0, 0);

  rc = measure_text_width(NULL, 1.0f, &w);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = measure_text_width("Test", 1.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = measure_text_width("Hello", 1.0f, &w);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(40.0f, w);

  rc = canvas_create(100, 100, &c);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Text */
  rc = draw_text(NULL, 10.0f, 10.0f, "Hi", 1.0f, col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = draw_text(c, 10.0f, 10.0f, NULL, 1.0f, col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = draw_text(c, 10.0f, 10.0f, "Test text \x10\x7F", 1.0f, col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = draw_text(c, 10.0f, 10.0f, "Hi", 0.0f, col);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Text centered */
  rc = draw_text_centered(NULL, 50.0f, 10.0f, "Hi", 1.0f, col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = draw_text_centered(c, 50.0f, 10.0f, "Centered", 1.0f, col);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test font loading */
  rc = rasterizer_load_font(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = rasterizer_load_font("nonexistent_font.ttf");
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);
  rc = rasterizer_load_font("tests/minimal.ttf");
  if (rc == UI_ERROR_NONE) {
    rc = measure_text_width("Hello", 1.0f, &w);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = draw_text(c, 10.0f, 10.0f, "Hi", 1.0f, col);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Icons: loop all types */
  for (icon_i = 0; icon_i < (int)ICON_TYPE_COUNT; ++icon_i) {
    rc = draw_icon(c, 50.0f, 50.0f, (enum icon_type)icon_i, 20.0f, col);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Invalid icon */
  rc = draw_icon(c, 50.0f, 50.0f, (enum icon_type)9999, 20.0f, col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = draw_icon(NULL, 50.0f, 50.0f, ICON_CHECK, 20.0f, col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = canvas_destroy(c);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_save_png(void) {
  struct canvas *c = NULL;
  ui_error_t rc;

  rc = save_canvas_png(NULL, "test.png");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = canvas_create(16, 16, &c);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = save_canvas_png(c, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = canvas_clear(c, UI_COLOR_ARGB(255, 255, 0, 0));
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = save_canvas_png(c, "test_screenshot_output.png");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  remove("test_screenshot_output.png");

  rc = canvas_destroy(c);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

static ui_error_t mock_render_all(const struct design_system_options *opts) {
  if (!opts)
    return UI_ERROR_INVALID_ARGUMENT;
  return UI_ERROR_NONE;
}

TEST test_design_system_registry(void) {
  const struct design_system_driver *const *drivers = NULL;
  const struct design_system_driver *found = NULL;
  size_t count = 0;
  ui_error_t rc;
  struct design_system_driver d1 = {"test_ds", "Test DS", "Desc",
                                    mock_render_all};

  rc = ds_registry_reset();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Register NULL */
  rc = ds_registry_register(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Register valid */
  rc = ds_registry_register(&d1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Register duplicate */
  rc = ds_registry_register(&d1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Find */
  rc = ds_registry_find(NULL, &found);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ds_registry_find("test_ds", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ds_registry_find("unknown", &found);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);
  ASSERT_EQ(NULL, found);

  rc = ds_registry_find("test_ds", &found);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(&d1, found);

  /* Get all */
  rc = ds_registry_get_all(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ds_registry_get_all(&drivers, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ds_registry_get_all(&drivers, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, count);
  ASSERT_EQ(&d1, drivers[0]);

  /* Reset */
  rc = ds_registry_reset();
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ds_registry_get_all(&drivers, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, count);

  PASS();
}

SUITE(screenshot_tool_suite) {
  RUN_TEST(test_canvas_create_and_destroy);
  RUN_TEST(test_canvas_create_oom);
  RUN_TEST(test_canvas_clear);
  RUN_TEST(test_draw_primitives);
  RUN_TEST(test_text_and_icons);
  RUN_TEST(test_save_png);
  RUN_TEST(test_design_system_registry);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(screenshot_tool_suite);
  GREATEST_MAIN_END();
}
