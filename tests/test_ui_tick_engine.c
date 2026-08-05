/* clang-format off */
#include "../include/ui_tick_engine.h"
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
  (*val)++;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_tick_engine *engine = NULL;
  ui_error_t rc;
  int test_val1 = 0;
  int test_val2 = 0;

  printf("Running normal tick engine tests...\n");

  rc = ui_tick_engine_create(&engine);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create engine\n");
    return 1;
  }

  rc = ui_tick_engine_schedule(engine, test_callback, &test_val1);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to schedule task 1\n");
    return 1;
  }

  rc = ui_tick_engine_schedule(engine, test_callback, &test_val2);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to schedule task 2\n");
    return 1;
  }

  rc = ui_tick_engine_tick(engine);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to tick\n");
    return 1;
  }

  if (test_val1 != 1 || test_val2 != 1) {
    printf("Tasks were not executed correctly: val1=%d val2=%d\n", test_val1,
           test_val2);
    return 1;
  }

  /* Tick again to ensure queue was cleared */
  rc = ui_tick_engine_tick(engine);
  if (test_val1 != 1 || test_val2 != 1) {
    printf("Tasks were executed again after queue should be empty!\n");
    return 1;
  }

  rc = ui_tick_engine_destroy(engine);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to destroy engine\n");
    return 1;
  }

  /* Test destroying engine with pending tasks */
  rc = ui_tick_engine_create(&engine);
  if (rc != UI_ERROR_NONE)
    return 1;
  ui_tick_engine_schedule(engine, test_callback, &test_val1);
  ui_tick_engine_schedule(engine, test_callback, &test_val2);
  rc = ui_tick_engine_destroy(engine);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Invalid arguments tests */
  if (ui_tick_engine_create(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on NULL out_engine\n");
    return 1;
  }

  if (ui_tick_engine_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on NULL engine to destroy\n");
    return 1;
  }

  if (ui_tick_engine_schedule(NULL, test_callback, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on NULL engine to schedule\n");
    return 1;
  }

  ui_tick_engine_create(&engine);
  if (ui_tick_engine_schedule(engine, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on NULL callback\n");
    return 1;
  }
  ui_tick_engine_destroy(engine);

  if (ui_tick_engine_tick(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on NULL engine to tick\n");
    return 1;
  }

  return 0;
}

static int run_oom_tests(void) {
  struct ui_tick_engine *engine = NULL;
  ui_error_t rc;
  int test_val = 0;

  printf("Running tick engine OOM tests...\n");

  /* Test engine creation failure */
  g_malloc_fail_countdown = 0;
  rc = ui_tick_engine_create(&engine);
  if (rc != UI_ERROR_OUT_OF_MEMORY || engine != NULL) {
    printf("Expected OUT_OF_MEMORY on create\n");
    return 1;
  }

  /* Test schedule failure */
  g_malloc_fail_countdown = -1;
  rc = ui_tick_engine_create(&engine);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create engine for OOM schedule test\n");
    return 1;
  }

  g_malloc_fail_countdown = 0;
  rc = ui_tick_engine_schedule(engine, test_callback, &test_val);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OUT_OF_MEMORY on schedule\n");
    return 1;
  }

  g_malloc_fail_countdown = -1;
  ui_tick_engine_destroy(engine);

  return 0;
}

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= run_oom_tests();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}
