/* clang-format off */
#include "ui_top_app_bar_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include "ui_signal.h"
#include <stdio.h>
/* clang-format on */

static int test_top_app_bar_lifecycle(void) {
  struct ui_arena *arena;
  struct ui_top_app_bar_base *bar;
  struct ui_top_app_bar_config config;
  enum ui_error rc;
  int failed = 0;

  rc = ui_arena_create(1024 * 1024, &arena);
  failed |= (rc != UI_ERROR_NONE);

  config.initial_state = UI_TOP_APP_BAR_STATE_EXPANDED;
  config.expanded_height = 120.0f;
  config.collapsed_height = 64.0f;
  config.scroll_threshold = 50.0f;

  rc = ui_top_app_bar_base_create(arena, &config, &bar);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_top_app_bar_base_destroy(bar);
  failed |= (rc != UI_ERROR_NONE);

  ui_arena_destroy(arena);
  return failed;
}

static int test_top_app_bar_scroll(void) {
  struct ui_arena *arena;
  struct ui_top_app_bar_base *bar;
  struct ui_top_app_bar_config config;
  enum ui_error rc;
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

  ui_arena_destroy(arena);
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
