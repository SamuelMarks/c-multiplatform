/* clang-format off */
#include "ui_theme_manager.h"
#include "ui_arena.h"
#include "ui_error.h"
#include "ui_signal.h"
#include <stdio.h>
/* clang-format on */

#define ACCUM_ERR(failed, expr) failed |= ((expr) != UI_ERROR_NONE)
#define ACCUM_FAIL(failed, expr) failed |= (expr)

struct ui_theme_manager {
  struct ui_arena *arena;
  enum ui_theme_mode current_mode;
  ui_signal_t *change_signal;
};

static int test_theme_manager_lifecycle(void) {
  struct ui_arena *arena;
  struct ui_theme_manager *manager;
  enum ui_error rc;
  int failed = 0;

  rc = ui_arena_create(1024 * 1024, &arena);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_theme_manager_create(arena, &manager);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_theme_manager_destroy(manager);
  failed |= (rc != UI_ERROR_NONE);

  ui_arena_destroy(arena);
  return failed;
}

static int test_theme_manager_mode(void) {
  struct ui_arena *arena;
  struct ui_theme_manager *manager;
  enum ui_error rc;
  enum ui_theme_mode mode;
  ui_signal_t *signal;
  union ui_signal_payload payload;
  int failed = 0;

  rc = ui_arena_create(1024 * 1024, &arena);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_theme_manager_create(arena, &manager);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_theme_manager_get_mode(manager, &mode);
  failed |= (rc != UI_ERROR_NONE || mode != UI_THEME_MODE_SYSTEM);

  rc = ui_theme_manager_set_mode(manager, UI_THEME_MODE_DARK);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_theme_manager_get_mode(manager, &mode);
  failed |= (rc != UI_ERROR_NONE || mode != UI_THEME_MODE_DARK);

  rc = ui_theme_manager_get_change_signal(manager, &signal);
  failed |= (rc != UI_ERROR_NONE || signal == NULL);

  rc = ui_signal_get(signal, &payload);
  failed |=
      (rc != UI_ERROR_NONE || payload.int_val != (ui_int32)UI_THEME_MODE_DARK);

  rc = ui_theme_manager_destroy(manager);
  failed |= (rc != UI_ERROR_NONE);

  ui_arena_destroy(arena);
  return failed;
}

extern int g_malloc_fail_countdown;

static int test_theme_manager_nulls(void) {
  struct ui_theme_manager *manager = NULL;
  enum ui_theme_mode mode;
  ui_signal_t *signal = NULL;
  struct ui_arena *arena = NULL;
  int failed = 0;

  failed |=
      (ui_theme_manager_create(NULL, &manager) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_theme_manager_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_theme_manager_set_mode(NULL, UI_THEME_MODE_DARK) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_theme_manager_get_mode(NULL, &mode) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_theme_manager_get_change_signal(NULL, &signal) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_theme_manager_get_mode(manager, NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_theme_manager_get_change_signal(manager, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_theme_manager_create(arena, NULL) != UI_ERROR_INVALID_ARGUMENT);

  ui_arena_create(1, &arena);

  g_malloc_fail_countdown = 0;
  failed |=
      (ui_theme_manager_create(arena, &manager) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 1;
  failed |=
      (ui_theme_manager_create(arena, &manager) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_theme_manager_create(arena, &manager);

  if (manager) {
    ui_signal_t *old_signal = manager->change_signal;
    manager->change_signal = NULL;

    failed |= (ui_theme_manager_set_mode(manager, UI_THEME_MODE_DARK) !=
               UI_ERROR_INVALID_ARGUMENT);
    failed |= (ui_theme_manager_destroy(manager) != UI_ERROR_INVALID_ARGUMENT);

    manager->change_signal = old_signal;
  }

  ui_arena_destroy(arena);

  return failed;
}

int main(void) {
  int result = 0;
  printf("Running ui_theme_manager tests...\n");

  result |= test_theme_manager_lifecycle();
  result |= test_theme_manager_mode();
  result |= test_theme_manager_nulls();

  if (result == 0) {
    printf("All theme manager tests PASSED\n");
  }

  return result;
}
