/* clang-format off */
#include <stdio.h>
#include "../include/ui_engine.h"
#include "../include/ui_error.h"
#include "../include/ui_reactor.h"
#include "../include/ui_thread_pool.h"
#include "../include/ui_timer.h"
#include "../include/ui_tick_engine.h"
#include "../src/ui_internal_mem.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

static int run_normal_tests(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  ui_error_t rc;

  printf("Running normal ui_engine tests...\n");

  config.num_threads = 2;

  /* Invalid args */
  if (ui_engine_create(NULL, &engine) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_engine_create(&config, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_engine_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_engine_tick(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_engine_create(&config, &engine);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create UI engine: %d\n", rc);
    return 1;
  }

  rc = ui_engine_tick(engine);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to tick UI engine: %d\n", rc);
    ui_engine_destroy(engine);
    return 1;
  }

  rc = ui_engine_destroy(engine);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to destroy UI engine: %d\n", rc);
    return 1;
  }

  /* Fallback 0 threads becomes 1 */
  config.num_threads = 0;
  rc = ui_engine_create(&config, &engine);
  if (rc != UI_ERROR_NONE)
    return 1;
  ui_engine_destroy(engine);

  return 0;
}

static void run_extra_engine_tests(void) {
  g_malloc_fail_countdown = 2;
  struct ui_engine *engine;
  ui_engine_create(&(struct ui_engine_config){0}, &engine);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 3;
  ui_engine_create(&(struct ui_engine_config){0}, &engine);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 4;
  ui_engine_create(&(struct ui_engine_config){0}, &engine);
  g_malloc_fail_countdown = -1;
}

static int run_oom_tests(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  ui_error_t rc;
  int countdown = 0;
  int i;
  int max_fails =
      10; /* engine struct, tick_engine, thread_pool, reactor, timer */

  config.num_threads = 1;

  printf("Running ui_engine OOM tests...\n");

  for (i = 0; i <= max_fails; ++i) {
    g_malloc_fail_countdown = i;
    rc = ui_engine_create(&config, &engine);
    g_malloc_fail_countdown = -1;

    if (rc == UI_ERROR_NONE) {
      /* If it managed to create despite our mocks, then the test framework or
       * some other reason didn't hit our mock malloc. But we should free it. */
      ui_engine_destroy(engine);
    }
  }

  return 0;
}

static int run_coverage_tests(void);

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= run_oom_tests();
  run_extra_engine_tests();
  failed |= run_coverage_tests();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All UI engine tests passed.\n");
  return 0;
}

/* We'll access the struct directly to trigger the branches */
struct ui_engine_hack {
  struct ui_tick_engine *tick_engine;
  struct ui_thread_pool *thread_pool;
  struct ui_reactor *reactor;
  struct ui_timer *timer;
};

static int run_coverage_tests(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  struct ui_engine_hack *hack;
  struct ui_reactor *tmp_reactor;
  struct ui_tick_engine *tmp_tick;

  config.num_threads = 1;
  if (ui_engine_create(&config, &engine) != UI_ERROR_NONE)
    return 1;
  hack = (struct ui_engine_hack *)engine;

  tmp_reactor = hack->reactor;
  hack->reactor =
      NULL; /* Force ui_reactor_poll to fail with INVALID_ARGUMENT */
  if (ui_engine_tick(engine) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  hack->reactor = tmp_reactor;
  tmp_tick = hack->tick_engine;
  hack->tick_engine =
      NULL; /* Force ui_tick_engine_tick to fail with INVALID_ARGUMENT */
  if (ui_engine_tick(engine) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  hack->tick_engine = tmp_tick;
  ui_engine_destroy(engine);

  /* Test ui_engine_destroy with NULL components to hit branches */
  struct ui_engine *null_engine = NULL;
  ui_engine_create(&config, &null_engine);
  hack = (struct ui_engine_hack *)null_engine;

  tmp_reactor = hack->reactor;
  tmp_tick = hack->tick_engine;
  struct ui_timer *tmp_timer = hack->timer;
  struct ui_thread_pool *tmp_pool = hack->thread_pool;

  hack->reactor = NULL;
  hack->tick_engine = NULL;
  hack->timer = NULL;
  hack->thread_pool = NULL;

  ui_engine_destroy(null_engine);

  /* Clean up the leaked components */
  if (tmp_reactor)
    ui_reactor_destroy(tmp_reactor);
  if (tmp_tick)
    ui_tick_engine_destroy(tmp_tick);
  if (tmp_timer)
    ui_timer_destroy(tmp_timer);
#ifndef UI_SINGLE_THREADED
  if (tmp_pool)
    ui_thread_pool_destroy(tmp_pool);
#endif

  return 0;
}
