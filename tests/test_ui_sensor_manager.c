/* clang-format off */
#include "../include/ui_arena.h"
#include "../include/ui_execution_context.h"
#include "../include/ui_sensor_manager.h"
#include "../include/ui_signal.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

/* Forward declare the internal mock tick since it's not exported by the header,
   but we want to test its effect on signals */
ui_error_t ui_sensor_manager_tick_mock(struct ui_sensor_manager *manager);

static int test_sensor_manager_basic(void) {
  struct ui_execution_context *ctx = NULL;
  struct ui_arena *arena = NULL;
  struct ui_sensor_manager *manager = NULL;
  struct ui_signal *signal = NULL;
  struct ui_sensor_vector3 vec_accel;
  struct ui_sensor_vector3 vec_gyro;
  union ui_signal_payload initial_payload = {0};
  ui_error_t rc;

  rc = ui_execution_context_create(&ctx);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_execution_context_create failed\n");
    return 1;
  }

  rc = ui_arena_create(1024, &arena);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_arena_create failed\n");
    return 1;
  }

  rc = ui_sensor_manager_create(&manager);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_sensor_manager_create failed\n");
    return 1;
  }

  initial_payload.ptr_val = NULL;
  rc = ui_signal_create(arena, initial_payload, UI_SIGNAL_TYPE_POINTER, NULL,
                        NULL, UI_SIGNAL_MODE_SINGLE_THREADED, &signal);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_signal_create failed\n");
    return 1;
  }

  rc = ui_sensor_manager_bind_orientation(manager, signal);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_sensor_manager_bind_orientation failed\n");
    return 1;
  }

  rc = ui_sensor_manager_start(manager);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_sensor_manager_start failed\n");
    return 1;
  }

  /* Start again should be noop */
  rc = ui_sensor_manager_start(manager);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Tick the mock sensor, which should update the signal */
  rc = ui_sensor_manager_tick_mock(manager);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_sensor_manager_tick_mock failed\n");
    return 1;
  }

  rc = ui_sensor_manager_get_accelerometer(manager, &vec_accel);
  if (rc != UI_ERROR_NONE || vec_accel.y != 9.81) {
    fprintf(stderr, "ui_sensor_manager_get_accelerometer failed\n");
    return 1;
  }

  rc = ui_sensor_manager_get_gyroscope(manager, &vec_gyro);
  if (rc != UI_ERROR_NONE || vec_gyro.x != 0.01) {
    fprintf(stderr, "ui_sensor_manager_get_gyroscope failed\n");
    return 1;
  }

  rc = ui_sensor_manager_stop(manager);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_sensor_manager_stop failed\n");
    return 1;
  }

  /* Stop again should be noop */
  rc = ui_sensor_manager_stop(manager);
  if (rc != UI_ERROR_NONE)
    return 1;

  (void)ui_signal_destroy(signal);
  (void)ui_sensor_manager_destroy(manager);
  (void)ui_execution_context_destroy(ctx);
  (void)ui_arena_destroy(arena);

  return 0;
}

static int test_sensor_manager_nulls(void) {
  struct ui_sensor_manager *manager = NULL;
  struct ui_signal *signal = (struct ui_signal *)0x123;
  struct ui_sensor_vector3 vec;

  if (ui_sensor_manager_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_sensor_manager_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_sensor_manager_start(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_sensor_manager_stop(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_sensor_manager_bind_orientation(NULL, signal) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  (void)ui_sensor_manager_create(&manager);

  if (ui_sensor_manager_bind_orientation(manager, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_sensor_manager_get_accelerometer(NULL, &vec) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_sensor_manager_get_accelerometer(manager, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_sensor_manager_get_gyroscope(NULL, &vec) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_sensor_manager_get_gyroscope(manager, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_sensor_manager_tick_mock(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_sensor_manager_tick_mock(manager) != UI_ERROR_INVALID_ARGUMENT)
    return 1; /* not running */

  /* tick mock without signal bounded */
  (void)ui_sensor_manager_start(manager);
  if (ui_sensor_manager_tick_mock(manager) != UI_ERROR_NONE)
    return 1;

  (void)ui_sensor_manager_destroy(manager);

  return 0;
}

static int test_sensor_manager_oom(void) {
  struct ui_sensor_manager *manager = NULL;
  g_malloc_fail_countdown = 0;
  if (ui_sensor_manager_create(&manager) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;
  return 0;
}

static void test_sensor_manager_signal_err(void);
int main(void) {
  int failed = 0;

  failed |= test_sensor_manager_basic();
  failed |= test_sensor_manager_nulls();
  failed |= test_sensor_manager_oom();
  test_sensor_manager_signal_err();

  if (failed) {
    fprintf(stderr, "test_ui_sensor_manager failed\n");
    return 1;
  }
  printf("test_ui_sensor_manager passed\n");
  return 0;
}

static void test_sensor_manager_signal_err(void) {
  struct ui_execution_context *ctx = NULL;
  struct ui_arena *arena = NULL;
  struct ui_sensor_manager *manager = NULL;
  struct ui_signal *signal = NULL;
  union ui_signal_payload initial_payload = {0};

  (void)ui_execution_context_create(&ctx);
  (void)ui_arena_create(1024, &arena);
  (void)ui_sensor_manager_create(&manager);

  initial_payload.ptr_val = NULL;
  (void)ui_signal_create(arena, initial_payload, UI_SIGNAL_TYPE_POINTER, NULL,
                         NULL, UI_SIGNAL_MODE_SINGLE_THREADED, &signal);

  (void)ui_sensor_manager_bind_orientation(manager, signal);
  (void)ui_sensor_manager_start(manager);

  /* Corrupt signal type so ui_signal_set will fail */
  /* ui_signal is opaque here, we can't directly corrupt its type. */
  /* How can ui_signal_set fail? It allocates a copy of subscribers array. We
   * can trigger OOM. */
  g_malloc_fail_countdown = 0;
  (void)ui_sensor_manager_tick_mock(manager);
  g_malloc_fail_countdown = -1;

  (void)ui_signal_destroy(signal);
  (void)ui_sensor_manager_destroy(manager);
  (void)ui_execution_context_destroy(ctx);
  (void)ui_arena_destroy(arena);
}
