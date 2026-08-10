/* clang-format off */
#include "../include/ui_execution_context.h"
#include "../include/ui_thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */
/* Mocking support */
extern int g_malloc_fail_countdown;
extern int g_malloc_called;

void *C_MULTIPLATFORM_MALLOC(size_t size);
void C_MULTIPLATFORM_FREE(void *ptr);
static ui_error_t test_callback(void *user_data) {
  int *val = (int *)user_data;
  *val = 42;
  return UI_ERROR_NONE;
}

static ui_error_t failing_callback(void *user_data) { return UI_ERROR_UNKNOWN; }

static ui_error_t test_cancel_callback(void *user_data) {
  int *val = (int *)user_data;
  *val = 99; /* Should not be executed */
  return UI_ERROR_NONE;
}

static ui_error_t thread_pool_callback(void *user_data) {
  int *val = (int *)user_data;
  *val = 100;
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_execution_context *ctx = NULL;
  ui_error_t rc;
  int test_val = 0;

  printf("Running normal tests...\n");

  rc = ui_execution_context_create(&ctx);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create context\n");
    return 1;
  }

  rc = ui_execution_context_schedule(ctx, test_callback, &test_val);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to schedule task\n");
    return 1;
  }

  rc = ui_execution_context_tick(ctx);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to execute tick\n");
    return 1;
  }

  if (test_val != 42) {
    printf("Task was not executed correctly\n");
    return 1;
  }

  /* Test failure case */
  rc = ui_execution_context_schedule(ctx, failing_callback, NULL);
  if (rc == UI_ERROR_NONE) {
    rc = ui_execution_context_tick(ctx);
    if (rc == UI_ERROR_NONE) {
      printf("Failed to propagate error from task\n");
      return 1;
    }
  }

  /* Test cancellation */
  test_val = 0;
  ui_execution_context_schedule(ctx, test_cancel_callback, &test_val);
  ui_execution_context_cancel(ctx);
  ui_execution_context_tick(ctx);
  if (test_val == 99) {
    printf("Task was executed after cancellation!\n");
    return 1;
  }

  rc = ui_execution_context_destroy(ctx);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to destroy context\n");
    return 1;
  }

  /* Invalid arguments tests */
  if (ui_execution_context_create(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on NULL out_ctx\n");
    return 1;
  }

  if (ui_execution_context_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on NULL ctx\n");
    return 1;
  }

  if (ui_execution_context_schedule(NULL, test_callback, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on NULL ctx to schedule\n");
    return 1;
  }

  /* Create dummy context to test schedule error */
  ui_execution_context_create(&ctx);
  if (ui_execution_context_schedule(ctx, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on NULL callback\n");
    return 1;
  }

  if (ui_execution_context_cancel(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on NULL ctx to cancel\n");
    return 1;
  }

  (void)ui_execution_context_destroy(ctx);

  if (ui_execution_context_get_current(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_execution_context_tick(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on NULL ctx to tick\n");
    return 1;
  }

  return 0;
}

static int run_paradigm_tests(void) {
  printf("Running execution paradigm tests...\n");

  /* Single-Threaded environment */
  {
    struct ui_execution_context *ctx = NULL;
    int test_val = 0;
    ui_execution_context_create(&ctx);
    ui_execution_context_set_current(ctx);
    struct ui_execution_context *get_ctx = NULL;
    ui_execution_context_get_current(&get_ctx);
    if (get_ctx != ctx)
      return 1;
    ui_execution_context_schedule(ctx, test_callback, &test_val);
    ui_execution_context_tick(ctx);
    if (test_val != 42)
      return 1;
    (void)ui_execution_context_destroy(ctx);
    ui_execution_context_set_current(NULL);
  }

  /* Thread-Pool environment */
  {
    struct ui_thread_pool *pool = NULL;
    int test_val = 0;
    if (ui_thread_pool_create(4, &pool) == UI_ERROR_NONE) {
      ui_thread_pool_schedule(pool, thread_pool_callback, &test_val);
      ui_thread_pool_destroy(pool); /* waits for completion */
      if (test_val != 100)
        return 1;
    }
  }

  /* Multi-Reactor environment */
  {
    struct ui_thread_pool *reactors[2] = {NULL, NULL};
    struct ui_execution_context *ctxs[2] = {NULL, NULL};
    int test_val1 = 0, test_val2 = 0;

    ui_execution_context_create(&ctxs[0]);
    ui_execution_context_create(&ctxs[1]);

    if (ui_thread_pool_create(1, &reactors[0]) == UI_ERROR_NONE &&
        ui_thread_pool_create(1, &reactors[1]) == UI_ERROR_NONE) {

      ui_thread_pool_schedule(reactors[0], thread_pool_callback, &test_val1);
      ui_thread_pool_schedule(reactors[1], thread_pool_callback, &test_val2);

      ui_thread_pool_destroy(reactors[0]);
      ui_thread_pool_destroy(reactors[1]);

      if (test_val1 != 100 || test_val2 != 100)
        return 1;
    }

    (void)ui_execution_context_destroy(ctxs[0]);
    (void)ui_execution_context_destroy(ctxs[1]);
  }

  return 0;
}

static int run_oom_tests(void) {
  struct ui_execution_context *ctx = NULL;
  ui_error_t rc;

  printf("Running OOM tests...\n");

  g_malloc_fail_countdown = 0; /* fail immediately */
  rc = ui_execution_context_create(&ctx);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OUT_OF_MEMORY but got %d\n", rc);
    return 1;
  }
  if (ctx != NULL) {
    printf("Expected out_ctx to be untouched/NULL on failure\n");
    return 1;
  }

  g_malloc_fail_countdown = -1; /* reset */
  return 0;
}

static ui_error_t failing_task_cb(void *user_data) { return UI_ERROR_UNKNOWN; }

static int test_execution_context_fail(void) {
  struct ui_execution_context *ctx = NULL;

  ui_execution_context_create(&ctx);
  ui_execution_context_schedule(ctx, failing_task_cb, NULL);

  if (ui_execution_context_tick(ctx) != UI_ERROR_UNKNOWN) {
    (void)ui_execution_context_destroy(ctx);
    return 1;
  }

  (void)ui_execution_context_destroy(ctx);
  return 0;
}

int main(void) {
  int failed = 0;

  failed |= run_normal_tests();
  failed |= run_paradigm_tests();
  failed |= run_oom_tests();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
