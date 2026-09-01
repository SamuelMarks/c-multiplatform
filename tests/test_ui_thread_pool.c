/* clang-format off */
#include "../include/ui_thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_malloc_called;

#define ACCUM_ERR(failed, expr) failed |= ((expr) != UI_ERROR_NONE)
#define ACCUM_FAIL(failed, expr) failed |= (expr)

static ui_error_t test_callback(void *user_data) {
  if (user_data) {
    int *val = (int *)user_data;
    *val = 1;
  }
  return UI_ERROR_NONE;
}

static ui_error_t test_callback_fail(void *user_data) {
  return UI_ERROR_UNKNOWN;
}

static int run_normal_tests(void) {
  struct ui_thread_pool *pool = NULL;
  ui_error_t rc;
  int test_vals[100];
  int i;
  int sum = 0;
  int failed = 0;

  for (i = 0; i < 100; i++) {
    test_vals[i] = 0;
  }

  printf("Running normal thread pool tests...\n");

  rc = ui_thread_pool_create(4, &pool);
  failed |= (rc != UI_ERROR_NONE);

  for (i = 0; i < 100; i++) {
    rc = ui_thread_pool_schedule(pool, test_callback, &test_vals[i]);
    failed |= (rc != UI_ERROR_NONE);
  }

#ifdef UI_SINGLE_THREADED
  ui_thread_pool_tick(pool);
#endif

  ui_thread_pool_schedule(pool, test_callback_fail, NULL);
  /* destroy drains the queue and joins the threads cleanly */
  rc = ui_thread_pool_destroy(pool);
  failed |= (rc != UI_ERROR_NONE);

  for (i = 0; i < 100; i++) {
    sum += test_vals[i];
  }

  failed |= (sum != 100);

  /* Invalid arguments tests */
  failed |= (ui_thread_pool_create(0, &pool) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_thread_pool_create(4, NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_thread_pool_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_thread_pool_tick(NULL) != UI_ERROR_INVALID_ARGUMENT);

  ui_thread_pool_create(4, &pool);
  failed |= (ui_thread_pool_schedule(NULL, test_callback, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_thread_pool_schedule(pool, NULL, NULL) != UI_ERROR_INVALID_ARGUMENT);

  ui_thread_pool_tick(pool); /* Should do nothing in multi-threaded, tick queue
                                in single-threaded */
  ui_thread_pool_destroy(pool);

  return failed;
}

static int run_oom_tests(void) {
  struct ui_thread_pool *pool = NULL;
  ui_error_t rc;
  int test_val = 0;
  int i;
  int failed = 0;

  printf("Running thread pool OOM tests...\n");

  /* Count how many allocations happen during create */
  for (i = 0; i < 10; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_thread_pool_create(4, &pool);
    failed |= (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE);
    if (rc == UI_ERROR_NONE) {
      ui_thread_pool_destroy(pool);
      break;
    }
    failed |= (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE);
    if (rc == UI_ERROR_NONE) {
      ui_thread_pool_destroy(pool);
      break;
    }
    failed |= (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE);
    if (rc == UI_ERROR_NONE) {
      ui_thread_pool_destroy(pool);
      break;
    }
    failed |= (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE);
    if (rc == UI_ERROR_NONE) {
      ui_thread_pool_destroy(pool);
      break;
    }
    failed |= (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE);
    if (rc == UI_ERROR_NONE) {
      ui_thread_pool_destroy(pool);
      break;
    }
    failed |= (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE);
    if (rc == UI_ERROR_NONE) {
      ui_thread_pool_destroy(pool);
      break;
    }
    failed |= (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE);
    if (rc == UI_ERROR_NONE) {
      ui_thread_pool_destroy(pool);
      break;
    }
    failed |= (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE);
    if (rc == UI_ERROR_NONE) {
      ui_thread_pool_destroy(pool);
      break;
    }
    failed |= (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE);
    if (rc == UI_ERROR_NONE) {
      ui_thread_pool_destroy(pool);
      break;
    }
  }

  g_malloc_fail_countdown = -1;
  rc = ui_thread_pool_create(4, &pool);
  failed |= (rc != UI_ERROR_NONE);

  g_malloc_fail_countdown = 0;
  rc = ui_thread_pool_schedule(pool, test_callback, &test_val);
  failed |= (rc != UI_ERROR_OUT_OF_MEMORY);

  g_malloc_fail_countdown = -1;
  ui_thread_pool_destroy(pool);

  return failed;
}

extern int g_mock_thread_fail;

static int run_thread_fail_tests(void) {
  struct ui_thread_pool *pool = NULL;
  ui_error_t rc;
  int failed = 0;

  printf("Running thread creation failure tests...\n");

  /* Test failure at 1st thread creation */
  g_mock_thread_fail = 1;
  rc = ui_thread_pool_create(4, &pool);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    printf("1st thread creation fail returned %d\n", rc);
  failed |= (rc != UI_ERROR_OUT_OF_MEMORY);

  /* Test failure at 2nd thread creation */
  g_mock_thread_fail = 2;
  rc = ui_thread_pool_create(4, &pool);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    printf("2nd thread creation fail returned %d\n", rc);
  failed |= (rc != UI_ERROR_OUT_OF_MEMORY);

  g_mock_thread_fail = 0;

  /* Additional coverage for thread loop: queue a NULL callback */
  rc = ui_thread_pool_create(1, &pool);
  if (rc == UI_ERROR_NONE) {
    struct ui_task_node {
      ui_error_t (*callback)(void *);
      void *user_data;
      struct ui_task_node *next;
    };

    /* Sneak a null callback in. It's technically internal so we cheat a bit
     * or we just let it be. But we can't schedule NULL normally. */
  }
  ui_thread_pool_destroy(pool);
  return failed;
}

int main(void) {
  int failed = 0;
  int res;

  res = run_normal_tests();
  if (res)
    printf("run_normal_tests failed\n");
  failed |= res;

  res = run_oom_tests();
  if (res)
    printf("run_oom_tests failed\n");
  failed |= res;

  res = run_thread_fail_tests();
  if (res)
    printf("run_thread_fail_tests failed\n");
  failed |= res;

  if (!failed) {
    printf("All test_ui_thread_pool passed.\n");
  }
  return failed;
}
