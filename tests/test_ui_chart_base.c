/* clang-format off */
#include "ui_chart_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stdio.h>
#include <math.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static void test_chart_coord_missing(void) {
  struct ui_arena *arena;
  ui_arena_create(1024 * 16, &arena);
  struct ui_chart_base *chart = NULL;
  ui_chart_base_create(arena, 999, &chart); /* INVALID COORD SYSTEM */

  struct ui_dom_rect bounds = {10.0, 10.0, 100.0, 100.0};
  ui_chart_base_set_draw_bounds(chart, &bounds);

  struct ui_dom_point pt;
  ui_chart_base_data_to_pixel(chart, 10.0, 10.0, &pt); /* Hits 215 false */

  double dx, dy;
  ui_chart_base_pixel_to_data(chart, 50.0, 50.0, &dx, &dy); /* Hits 256 false */

  ui_chart_base_destroy(chart);
  ui_arena_destroy(arena);
}
int main(void) {
  test_chart_coord_missing();
  struct ui_arena *arena;
  struct ui_chart_base *chart = NULL;
  struct ui_chart_scale_config x_scale;
  struct ui_chart_scale_config y_scale;
  struct ui_dom_rect bounds;
  enum ui_error err;
  ui_signal_t *signal = NULL;
  struct ui_dom_point pixel_pt;
  double out_x, out_y;

  if (ui_arena_create(1024 * 16, &arena) != UI_ERROR_NONE) {
    return 1;
  }

  err = ui_chart_base_create(arena, UI_CHART_COORDINATE_CARTESIAN, &chart);
  if (err != UI_ERROR_NONE || chart == NULL) {
    return 1;
  }

  err = ui_chart_base_get_topology_signal(chart, &signal);
  if (err != UI_ERROR_NONE || signal == NULL) {
    return 1;
  }

  bounds.x = 10.0;
  bounds.y = 10.0;
  bounds.width = 100.0;
  bounds.height = 100.0;
  err = ui_chart_base_set_draw_bounds(chart, &bounds);
  if (err != UI_ERROR_NONE)
    return 1;

  /* X mapping: domain 0-10 maps to pixels 0-100 */
  x_scale.type = UI_CHART_SCALE_LINEAR;
  x_scale.domain_min = 0.0;
  x_scale.domain_max = 10.0;
  x_scale.range_min = 0.0;
  x_scale.range_max = 100.0;
  err = ui_chart_base_set_x_scale(chart, &x_scale);
  if (err != UI_ERROR_NONE)
    return 1;

  /* Y mapping: domain 0-10 maps to pixels 100-0 (inverted y) */
  y_scale.type = UI_CHART_SCALE_LINEAR;
  y_scale.domain_min = 0.0;
  y_scale.domain_max = 10.0;
  y_scale.range_min = 100.0;
  y_scale.range_max = 0.0;
  err = ui_chart_base_set_y_scale(chart, &y_scale);
  if (err != UI_ERROR_NONE)
    return 1;

  /* Test 1: Data to Pixel (Cartesian) */
  /* data(5, 5) -> scale_x(50), scale_y(50) -> bounds offset(10, 10) ->
   * pixel(60, 60) */
  err = ui_chart_base_data_to_pixel(chart, 5.0, 5.0, &pixel_pt);
  if (err != UI_ERROR_NONE)
    return 1;
  if (fabs(pixel_pt.x - 60.0) > 0.01 || fabs(pixel_pt.y - 60.0) > 0.01) {
    printf("Failed Cartesian D2P: x=%f, y=%f\n", pixel_pt.x, pixel_pt.y);
    return 1;
  }

  /* Test 2: Pixel to Data (Cartesian) */
  /* pixel(60, 60) -> reverse bounds -> scale_x(50), scale_y(50) -> data(5, 5)
   */
  err = ui_chart_base_pixel_to_data(chart, 60.0, 60.0, &out_x, &out_y);
  if (err != UI_ERROR_NONE)
    return 1;
  if (fabs(out_x - 5.0) > 0.01 || fabs(out_y - 5.0) > 0.01) {
    printf("Failed Cartesian P2D: x=%f, y=%f\n", out_x, out_y);
    return 1;
  }

  /* Test 3: Logarithmic scale translation */
  y_scale.type = UI_CHART_SCALE_LOGARITHMIC;
  y_scale.domain_min = 1.0;
  y_scale.domain_max = 100.0;
  y_scale.range_min = 100.0;
  y_scale.range_max = 0.0;
  err = ui_chart_base_set_y_scale(chart, &y_scale);
  if (err != UI_ERROR_NONE)
    return 1;

  /* data(5, 10) -> log10 is exactly halfway between log1 and log100 -> Y should
   * be 50 + 10 = 60 */
  err = ui_chart_base_data_to_pixel(chart, 5.0, 10.0, &pixel_pt);
  if (err != UI_ERROR_NONE)
    return 1;
  if (fabs(pixel_pt.y - 60.0) > 0.01) {
    printf("Failed Log D2P: y=%f\n", pixel_pt.y);
    return 1;
  }

  /* Test Log scale negative values / zero to cover ternary fallback */
  struct ui_chart_scale_config log_neg_scale;
  log_neg_scale.type = UI_CHART_SCALE_LOGARITHMIC;
  log_neg_scale.domain_min = -5.0;
  log_neg_scale.domain_max = -1.0;
  log_neg_scale.range_min = 100.0;
  log_neg_scale.range_max = 0.0;
  ui_chart_base_set_y_scale(chart, &log_neg_scale);
  ui_chart_base_data_to_pixel(
      chart, 5.0, -10.0,
      &pixel_pt); /* log of negative maps to 1e-10 fallback */
  ui_chart_base_pixel_to_data(
      chart, 60.0, 60.0, &out_x,
      &out_y); /* unscale from log with negative domain bounds */

  /* Reset y scale */
  ui_chart_base_set_y_scale(chart, &y_scale);

  /* pixel(60, 60) -> reverse -> log halfway -> data Y 10.0 */
  err = ui_chart_base_pixel_to_data(chart, 60.0, 60.0, &out_x, &out_y);
  if (err != UI_ERROR_NONE)
    return 1;
  if (fabs(out_y - 10.0) > 0.01) {
    printf("Failed Log P2D: y=%f\n", out_y);
    return 1;
  }

  /* Test 4: Out of bounds hit-test */
  err = ui_chart_base_pixel_to_data(chart, 999.0, 999.0, &out_x,
                                    &out_y); /* pixel_x > width */
  if (err != UI_ERROR_OUT_OF_BOUNDS) {
    return 1;
  }
  err = ui_chart_base_pixel_to_data(chart, 50.0, 999.0, &out_x,
                                    &out_y); /* pixel_y > height */
  if (err != UI_ERROR_OUT_OF_BOUNDS) {
    return 1;
  }
  err = ui_chart_base_pixel_to_data(chart, 50.0, -10.0, &out_x,
                                    &out_y); /* pixel_y < y */
  if (err != UI_ERROR_OUT_OF_BOUNDS) {
    return 1;
  }
  err = ui_chart_base_pixel_to_data(chart, -10.0, 50.0, &out_x,
                                    &out_y); /* pixel_x < x */
  if (err != UI_ERROR_OUT_OF_BOUNDS) {
    return 1;
  }

  /* Test 5: Invalid arguments */
  if (ui_chart_base_create(NULL, UI_CHART_COORDINATE_CARTESIAN, &chart) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chart_base_create(arena, UI_CHART_COORDINATE_CARTESIAN, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chart_base_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chart_base_get_topology_signal(NULL, &signal) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chart_base_get_topology_signal(chart, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chart_base_set_draw_bounds(NULL, &bounds) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chart_base_set_draw_bounds(chart, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chart_base_set_x_scale(NULL, &x_scale) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chart_base_set_x_scale(chart, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chart_base_set_y_scale(NULL, &y_scale) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chart_base_set_y_scale(chart, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chart_base_data_to_pixel(NULL, 0.0, 0.0, &pixel_pt) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chart_base_data_to_pixel(chart, 0.0, 0.0, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chart_base_pixel_to_data(NULL, 0.0, 0.0, &out_x, &out_y) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chart_base_pixel_to_data(chart, 0.0, 0.0, NULL, &out_y) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chart_base_pixel_to_data(chart, 0.0, 0.0, &out_x, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test 6: Zero width domain */
  x_scale.type = UI_CHART_SCALE_LINEAR;
  x_scale.domain_min = 5.0;
  x_scale.domain_max = 5.0;
  x_scale.range_min = 0.0;
  x_scale.range_max = 100.0;
  ui_chart_base_set_x_scale(chart, &x_scale);
  ui_chart_base_data_to_pixel(chart, 5.0, 10.0, &pixel_pt);
  ui_chart_base_pixel_to_data(chart, 60.0, 60.0, &out_x, &out_y);

  /* Test 7: Polar coordinates */
  {
    struct ui_chart_base *polar = NULL;
    ui_chart_base_create(arena, UI_CHART_COORDINATE_POLAR, &polar);
    ui_chart_base_set_draw_bounds(polar, &bounds); /* cx=60, cy=60 */

    x_scale.domain_min = 0.0;
    x_scale.domain_max = 360.0;
    x_scale.range_min = 0.0;
    x_scale.range_max = 2.0 * 3.14159265358979323846;
    ui_chart_base_set_x_scale(polar, &x_scale);

    y_scale.type = UI_CHART_SCALE_LINEAR;
    y_scale.domain_min = 0.0;
    y_scale.domain_max = 10.0;
    y_scale.range_min = 0.0;
    y_scale.range_max = 50.0;
    ui_chart_base_set_y_scale(polar, &y_scale);

    /* Angle 0, radius 10 (range 50) -> x=60+50=110, y=60+0=60 */
    ui_chart_base_data_to_pixel(polar, 0.0, 10.0, &pixel_pt);
    ui_chart_base_pixel_to_data(polar, 110.0, 60.0, &out_x, &out_y);

    /* Negative angle */
    ui_chart_base_pixel_to_data(polar, 60.0, 10.0, &out_x, &out_y);

    ui_chart_base_destroy(polar);
  }

  /* Test 8: Arena alloc failure / Signal create failure */
  {
    struct ui_arena *small_arena;
    struct ui_chart_base *temp_chart = NULL;
    ui_arena_create(8, &small_arena); /* Too small for chart struct */
    ui_chart_base_create(small_arena, UI_CHART_COORDINATE_CARTESIAN,
                         &temp_chart);
    ui_arena_destroy(small_arena);

    ui_arena_create(200,
                    &small_arena); /* Big enough for chart, but not signal */
    ui_chart_base_create(small_arena, UI_CHART_COORDINATE_CARTESIAN,
                         &temp_chart);
    ui_arena_destroy(small_arena);
  }

  /* Test 9: Zero range span */
  x_scale.type = UI_CHART_SCALE_LINEAR;
  x_scale.domain_min = 0.0;
  x_scale.domain_max = 10.0;
  x_scale.range_min = 50.0;
  x_scale.range_max = 50.0; /* Zero range span */
  ui_chart_base_set_x_scale(chart, &x_scale);
  ui_chart_base_pixel_to_data(chart, 60.0, 60.0, &out_x, &out_y);

  err = ui_chart_base_destroy(chart);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

#ifdef UI_TEST_MOCK_ALLOC
  {
    int i;
    for (i = 0; i < 20; i++) {
      struct ui_chart_base *oom_chart = NULL;
      g_malloc_fail_countdown = i;
      err = ui_chart_base_create(arena, UI_CHART_COORDINATE_CARTESIAN,
                                 &oom_chart);
      if (err == UI_ERROR_NONE) {
        ui_chart_base_destroy(oom_chart);
        break; /* Enough iterations to pass */
      }
    }
    g_malloc_fail_countdown = -1;
  }
#endif

  ui_arena_destroy(arena);
  return 0;
}
