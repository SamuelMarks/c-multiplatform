/* clang-format off */
#include "ui_safe_area_manager.h"
#include "ui_arena.h"
#include "ui_error.h"
#include "ui_signal.h"
#include <stdio.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_safe_area_lifecycle(void) {
  struct ui_arena *arena;
  struct ui_safe_area_manager *manager;
  ui_error_t rc;

  rc = ui_arena_create(1024 * 1024, &arena);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_safe_area_manager_create(arena, &manager);
  if (rc != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_arena_destroy(arena);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    return 1;
  }

  rc = ui_safe_area_manager_destroy(manager);
  if (rc != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_arena_destroy(arena);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    return 1;
  }

  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_safe_area_insets(void) {
  struct ui_arena *arena;
  struct ui_safe_area_manager *manager;
  ui_error_t rc;
  struct ui_safe_area_insets insets;
  struct ui_safe_area_insets *payload_insets;
  ui_signal_t *signal;
  union ui_signal_payload payload;

  rc = ui_arena_create(1024 * 1024, &arena);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_safe_area_manager_create(arena, &manager);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_safe_area_manager_get_insets(manager, &insets);
  if (rc != UI_ERROR_NONE || insets.top != 0.0f)
    return 1;

  insets.top = 44.0f;
  insets.bottom = 34.0f;
  insets.left = 0.0f;
  insets.right = 0.0f;
  rc = ui_safe_area_manager_set_insets(manager, &insets);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_safe_area_manager_get_insets(manager, &insets);
  if (rc != UI_ERROR_NONE || insets.top != 44.0f || insets.bottom != 34.0f)
    return 1;

  rc = ui_safe_area_manager_get_change_signal(manager, &signal);
  if (rc != UI_ERROR_NONE || signal == NULL)
    return 1;

  rc = ui_signal_get(signal, &payload);
  if (rc != UI_ERROR_NONE)
    return 1;

  payload_insets = (struct ui_safe_area_insets *)payload.ptr_val;
  if (payload_insets->top != 44.0f || payload_insets->bottom != 34.0f)
    return 1;

  rc = ui_safe_area_manager_destroy(manager);
  if (rc != UI_ERROR_NONE)
    return 1;

  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_safe_area_edge_cases(void) {
  struct ui_arena *arena;
  struct ui_safe_area_manager *manager;
  ui_error_t rc;
  struct ui_safe_area_insets insets;
  ui_signal_t *signal;

  ui_arena_create(1024 * 1024, &arena);
  ui_safe_area_manager_create(arena, &manager);

  /* Invalid arguments */
  assert(ui_safe_area_manager_create(NULL, &manager) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_safe_area_manager_create(arena, NULL) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_safe_area_manager_destroy(NULL) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_safe_area_manager_set_insets(NULL, &insets) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_safe_area_manager_set_insets(manager, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_safe_area_manager_get_insets(NULL, &insets) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_safe_area_manager_get_insets(manager, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_safe_area_manager_get_change_signal(NULL, &signal) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_safe_area_manager_get_change_signal(manager, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  /* Equality logic branches */
  insets.top = 0;
  insets.bottom = 0;
  insets.left = 0;
  insets.right = 0;
  ui_safe_area_manager_set_insets(manager, &insets);

  insets.top = 10;
  ui_safe_area_manager_set_insets(manager, &insets);

  insets.top = 0;
  insets.bottom = 10;
  ui_safe_area_manager_set_insets(manager, &insets);

  insets.bottom = 0;
  insets.left = 10;
  ui_safe_area_manager_set_insets(manager, &insets);

  insets.left = 0;
  insets.right = 10;
  ui_safe_area_manager_set_insets(manager, &insets);

  ui_safe_area_manager_set_insets(manager, &insets); /* same */

  /* OOM tests */
#ifdef UI_TEST_MOCK_ALLOC
  {
    int i;
    for (i = 0; i < 5; i++) {
      struct ui_safe_area_manager *oom_manager = NULL;
      g_malloc_fail_countdown = i;
      rc = ui_safe_area_manager_create(arena, &oom_manager);
      g_malloc_fail_countdown = -1;
      if (rc == UI_ERROR_NONE) {
        ui_safe_area_manager_destroy(oom_manager);
      }
    }
    for (i = 0; i < 2; i++) {
      struct ui_safe_area_manager *oom_manager = NULL;
      ui_safe_area_manager_create(arena, &oom_manager);
      g_malloc_fail_countdown = i;
      rc = ui_safe_area_manager_set_insets(oom_manager, &insets);
      g_malloc_fail_countdown = -1;
      ui_safe_area_manager_destroy(oom_manager);
    }
    for (i = 0; i < 2; i++) {
      struct ui_safe_area_manager *oom_manager = NULL;
      ui_safe_area_manager_create(arena, &oom_manager);
      g_malloc_fail_countdown = i;
      rc = ui_safe_area_manager_destroy(oom_manager);
      g_malloc_fail_countdown = -1;
      if (rc != UI_ERROR_NONE) {
        ui_safe_area_manager_destroy(oom_manager);
      }
    }
  }
#endif

  ui_safe_area_manager_destroy(manager);
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

int main(void) {
  int result = 0;
  printf("Running ui_safe_area_manager tests...\n");

  if (test_safe_area_lifecycle() != 0) {
    printf("test_safe_area_lifecycle FAILED\n");
    result = 1;
  }
  if (test_safe_area_insets() != 0) {
    printf("test_safe_area_insets FAILED\n");
    result = 1;
  }
  if (test_safe_area_edge_cases() != 0) {
    printf("test_safe_area_edge_cases FAILED\n");
    result = 1;
  }

  if (result == 0) {
    printf("All safe area manager tests PASSED\n");
  } else {
    printf("Some safe area manager tests FAILED\n");
  }

  return result;
}
