/* clang-format off */
#include "ui_safe_area_manager.h"
#include "ui_arena.h"
#include "ui_error.h"
#include "ui_signal.h"
#include <stdio.h>
/* clang-format on */

static int test_safe_area_lifecycle(void) {
  struct ui_arena *arena;
  struct ui_safe_area_manager *manager;
  enum ui_error rc;

  rc = ui_arena_create(1024 * 1024, &arena);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_safe_area_manager_create(arena, &manager);
  if (rc != UI_ERROR_NONE) {
    ui_arena_destroy(arena);
    return 1;
  }

  rc = ui_safe_area_manager_destroy(manager);
  if (rc != UI_ERROR_NONE) {
    ui_arena_destroy(arena);
    return 1;
  }

  ui_arena_destroy(arena);
  return 0;
}

static int test_safe_area_insets(void) {
  struct ui_arena *arena;
  struct ui_safe_area_manager *manager;
  enum ui_error rc;
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

  ui_arena_destroy(arena);
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

  if (result == 0) {
    printf("All safe area manager tests PASSED\n");
  } else {
    printf("Some safe area manager tests FAILED\n");
  }

  return result;
}
