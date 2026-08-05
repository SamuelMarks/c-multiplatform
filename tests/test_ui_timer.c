/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include "../include/ui_timer.h"
#include "../include/ui_error.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
extern int g_ui_timer_clock_gettime_fail;
#endif

static ui_error_t custom_source(void *user_data, double *out_time_secs) {
  double *val = (double *)user_data;
  if (out_time_secs) {
    *out_time_secs = *val;
  }
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_timer *timer = NULL;
  ui_error_t rc;
  double time1 = 0.0;
  double time2 = 0.0;

  struct ui_timer_config config;
  double custom_time = 42.5;
  int failed = 0;

  printf("Running normal timer tests...\n");

  /* Test invalid arguments */
  failed |= (ui_timer_create_monotonic(NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_timer_create_custom(NULL, &timer) != UI_ERROR_INVALID_ARGUMENT);

  config.time_source = custom_source;
  config.user_data = &custom_time;
  failed |=
      (ui_timer_create_custom(&config, NULL) != UI_ERROR_INVALID_ARGUMENT);

  config.time_source = NULL;
  failed |=
      (ui_timer_create_custom(&config, &timer) != UI_ERROR_INVALID_ARGUMENT);
  config.time_source = custom_source;

  failed |= (ui_timer_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_timer_create_monotonic(&timer);
  failed |= (rc != UI_ERROR_NONE);

  failed |= (ui_timer_now(NULL, &time1) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_timer_now(timer, NULL) != UI_ERROR_INVALID_ARGUMENT);

  ui_timer_destroy(timer);
  timer = NULL;

  /* Test Monotonic Timer */
  rc = ui_timer_create_monotonic(&timer);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_timer_now(timer, &time1);
  failed |= (rc != UI_ERROR_NONE);

  {
    volatile int i;
    for (i = 0; i < 10000; ++i) {
      /* busy wait */
    }
  }

  rc = ui_timer_now(timer, &time2);
  failed |= (rc != UI_ERROR_NONE);

  failed |= (time2 < time1);

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
  g_ui_timer_clock_gettime_fail = 1;
  failed |= (ui_timer_now(timer, &time2) != UI_ERROR_UNKNOWN);
  g_ui_timer_clock_gettime_fail = 0;
#endif

  ui_timer_destroy(timer);
  timer = NULL;

  /* Test Custom Timer */
  rc = ui_timer_create_custom(&config, &timer);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_timer_now(timer, &time1);
  failed |= (rc != UI_ERROR_NONE);

  failed |= (time1 != 42.5);

  ui_timer_destroy(timer);

  return failed;
}

static int run_oom_tests(void) {
  struct ui_timer *timer = NULL;
  ui_error_t rc;
  struct ui_timer_config config;
  double custom_time = 42.5;
  int failed = 0;

  config.time_source = custom_source;
  config.user_data = &custom_time;

  printf("Running timer OOM tests...\n");

  g_malloc_fail_countdown = 0;
  rc = ui_timer_create_monotonic(&timer);
  failed |= (rc != UI_ERROR_OUT_OF_MEMORY);

  g_malloc_fail_countdown = 0;
  rc = ui_timer_create_custom(&config, &timer);
  failed |= (rc != UI_ERROR_OUT_OF_MEMORY);

  g_malloc_fail_countdown = -1;
  return failed;
}

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= run_oom_tests();

  if (!failed) {
    printf("All test_ui_timer passed.\n");
  }
  return failed;
}
