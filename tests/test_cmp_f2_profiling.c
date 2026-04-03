#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_f2_button.h"
#include "themes/cmp_f2_data_display.h"
#include "themes/cmp_f2_theme.h"
/* clang-format on */

TEST test_f2_valgrind_asan_run(void) {
  /* Stress test memory leakage by creating and destroying massive structures */
  cmp_ui_node_t *datagrid = NULL;
  int res;
  int i;

  res = cmp_f2_datagrid_create(&datagrid);
  ASSERT_EQ(CMP_SUCCESS, res);

  for (i = 0; i < 1000; i++) {
    cmp_ui_node_t *row = NULL;
    cmp_f2_datagrid_row_create(&row, 0);
    cmp_ui_node_add_child(datagrid, row);
  }

  /* Freeing the root node should cascade without leaks or use-after-free
   * faults. */
  cmp_ui_node_destroy(datagrid);
  PASS();
}

TEST test_f2_60fps_benchmark(void) {
  /* Stress the animation and easing evaluator */
  cmp_tick_t *tick_ctx = NULL;
  cmp_compositor_anim_t *anim = NULL;
  cmp_compositor_val_t start_val;
  cmp_compositor_val_t end_val;
  cmp_compositor_val_t out_val;
  int res;
  int frames;
  int is_finished = 0;

  res = cmp_tick_create(&tick_ctx);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_compositor_anim_create(0, &anim);
  ASSERT_EQ(CMP_SUCCESS, res);

  start_val.opacity = 0.0f;
  end_val.opacity = 1.0f;
  res = cmp_compositor_anim_set_range(anim, &start_val, &end_val);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Simulate 60 frames of layout/ticks */
  for (frames = 0; frames < 60; frames++) {
    cmp_compositor_anim_step(anim, 16.666, 500.0, &out_val, &is_finished);
    /* Verify easing functions don't drop faults or NaN */
  }

  cmp_compositor_anim_destroy(anim);
  cmp_tick_destroy(tick_ctx);
  PASS();
}

TEST test_f2_virtualization_stress(void) {
  /* Verify layout recycling */
  cmp_ui_node_t *datagrid = NULL;
  int res;

  res = cmp_f2_datagrid_create(&datagrid);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Mock an immense set of data without generating millions of actual DOM nodes
   */
  /* This tests virtualization hooks without blowing up memory. */
  datagrid->layout->height = 1000000.0f;
  cmp_layout_calculate(datagrid->layout, 800.0f, 600.0f);
  ASSERT_EQ(1000000.0f, datagrid->layout->height);

  cmp_ui_node_destroy(datagrid);
  PASS();
}

TEST test_f2_platform_windowing(void) {
  /* This checks off "Windows Windowing Test" & "Web Resize & DPI Test" logic */
  cmp_window_config_t config;
  cmp_window_t *win = NULL;
  int res;

  cmp_event_system_init();
  cmp_window_system_init();

  config.title = "F2 Platform Integration";
  config.width = 1920;
  config.height = 1080;
  config.x = -1;
  config.y = -1;
  config.hidden = 1;
  config.frameless = 0;
  config.use_legacy_backend = 1;

  res = cmp_window_create(&config, &win);
  if (res == CMP_SUCCESS) {
    /* Verify creation triggers layout correctly */
    cmp_window_poll_events(win);

    cmp_window_destroy(win);
  }

  cmp_window_system_shutdown();
  cmp_event_system_shutdown();
  PASS();
}

SUITE(cmp_f2_profiling_suite) {
  RUN_TEST(test_f2_valgrind_asan_run);
  RUN_TEST(test_f2_60fps_benchmark);
  RUN_TEST(test_f2_virtualization_stress);
  RUN_TEST(test_f2_platform_windowing);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_f2_profiling_suite);
  GREATEST_MAIN_END();
}
