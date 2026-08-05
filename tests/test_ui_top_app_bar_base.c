/* clang-format off */
#include "ui_top_app_bar_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include "ui_signal.h"
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

static int test_top_app_bar_lifecycle(void) {
  struct ui_arena *arena;
  struct ui_arena *small_arena;
  struct ui_top_app_bar_base *bar;
  struct ui_top_app_bar_config config;
  ui_error_t rc;
  int failed = 0;

  rc = ui_arena_create(1024 * 1024, &arena);
  failed |= (rc != UI_ERROR_NONE);
  rc = ui_arena_create(1, &small_arena);
  failed |= (rc != UI_ERROR_NONE);

  config.initial_state = UI_TOP_APP_BAR_STATE_EXPANDED;
  config.expanded_height = 120.0f;
  config.collapsed_height = 64.0f;
  config.scroll_threshold = 50.0f;

  /* Null arguments */
  if (ui_top_app_bar_base_create(NULL, &config, &bar) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("1\n");
    failed = 1;
  }
  if (ui_top_app_bar_base_create(arena, NULL, &bar) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("2\n");
    failed = 1;
  }
  if (ui_top_app_bar_base_create(arena, &config, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("3\n");
    failed = 1;
  }
  if (ui_top_app_bar_base_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("4\n");
    failed = 1;
  }

  if (ui_top_app_bar_base_handle_scroll(NULL, 10.0f, 10.0f) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("5\n");
    failed = 1;
  }
  if (ui_top_app_bar_base_get_state_signal(NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("6\n");
    failed = 1;
  }
  if (ui_top_app_bar_base_get_height_signal(NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("7\n");
    failed = 1;
  }

  /* Test allocation failures */
#ifdef UI_TEST_MOCK_ALLOC
  int i;
  for (i = 0; i < 10; i++) {
    g_malloc_fail_countdown = i;
    if (ui_top_app_bar_base_create(arena, &config, &bar) == UI_ERROR_NONE) {
      ui_top_app_bar_base_destroy(bar);
    }
  }
  g_malloc_fail_countdown = -1;
#endif

  rc = ui_top_app_bar_base_create(arena, &config, &bar);
  failed |= (rc != UI_ERROR_NONE);

  config.initial_state = UI_TOP_APP_BAR_STATE_COLLAPSED;
  rc = ui_top_app_bar_base_create(arena, &config, &bar);
  failed |= (rc != UI_ERROR_NONE);

  failed |= (ui_top_app_bar_base_get_state_signal(bar, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_top_app_bar_base_get_height_signal(bar, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);

  rc = ui_top_app_bar_base_destroy(bar);
  failed |= (rc != UI_ERROR_NONE);

  (void)ui_arena_destroy(arena);
  (void)ui_arena_destroy(small_arena);
  return failed;
}

static int test_top_app_bar_scroll(void) {
  struct ui_arena *arena;
  struct ui_top_app_bar_base *bar;
  struct ui_top_app_bar_config config;
  ui_error_t rc;
  ui_signal_t *height_signal;
  ui_signal_t *state_signal;
  union ui_signal_payload payload;
  int failed = 0;

  rc = ui_arena_create(1024 * 1024, &arena);
  failed |= (rc != UI_ERROR_NONE);

  config.initial_state = UI_TOP_APP_BAR_STATE_EXPANDED;
  config.expanded_height = 120.0f;
  config.collapsed_height = 64.0f;
  config.scroll_threshold = 50.0f;

  rc = ui_top_app_bar_base_create(arena, &config, &bar);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_top_app_bar_base_handle_scroll(bar, 10.0f, 10.0f);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_top_app_bar_base_get_height_signal(bar, &height_signal);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_signal_get(height_signal, &payload);
  failed |= (rc != UI_ERROR_NONE || payload.float_val != 110.0f);

  rc = ui_top_app_bar_base_get_state_signal(bar, &state_signal);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_signal_get(state_signal, &payload);
  failed |= (rc != UI_ERROR_NONE ||
             payload.int_val != (ui_int32)UI_TOP_APP_BAR_STATE_FLOATING);

  /* Scroll past collapsed */
  rc = ui_top_app_bar_base_handle_scroll(bar, 100.0f, 90.0f);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_signal_get(height_signal, &payload);
  failed |= (rc != UI_ERROR_NONE || payload.float_val != 64.0f);

  rc = ui_signal_get(state_signal, &payload);
  failed |= (rc != UI_ERROR_NONE ||
             payload.int_val != (ui_int32)UI_TOP_APP_BAR_STATE_COLLAPSED);

  /* Scroll back up */
  rc = ui_top_app_bar_base_handle_scroll(bar, 0.0f, -100.0f);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_signal_get(state_signal, &payload);
  failed |= (rc != UI_ERROR_NONE ||
             payload.int_val != (ui_int32)UI_TOP_APP_BAR_STATE_EXPANDED);

  rc = ui_top_app_bar_base_destroy(bar);
  failed |= (rc != UI_ERROR_NONE);

  (void)ui_arena_destroy(arena);
  return failed;
}

int main(void) {
  int result = 0;
  printf("Running ui_top_app_bar_base tests...\n");

  result |= test_top_app_bar_lifecycle();
  result |= test_top_app_bar_scroll();

  if (result == 0) {
    printf("All top app bar tests PASSED\n");
  }

  return result;
}
