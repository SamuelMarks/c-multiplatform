/* clang-format off */
#include "ui_top_app_bar_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include "ui_signal.h"
#include <assert.h>
#include <stdio.h>
/* clang-format on */

struct ui_top_app_bar_base {
  struct ui_arena *arena;
  struct ui_top_app_bar_config config;
  ui_signal_t *state_signal;
  ui_signal_t *height_signal;
};

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

static int test_top_app_bar_lifecycle(void) {
  struct ui_arena *arena;
  struct ui_top_app_bar_base *bar;
  struct ui_top_app_bar_config config;
  ui_error_t rc;
  int failed = 0;
#ifdef UI_TEST_MOCK_ALLOC
  int i;
  extern int g_top_app_bar_mock_fail;
#endif

  rc = ui_arena_create(1024 * 1024, &arena);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  config.initial_state = UI_TOP_APP_BAR_STATE_EXPANDED;
  config.expanded_height = 120.0f;
  config.collapsed_height = 64.0f;
  config.scroll_threshold = 50.0f;

  /* Null arguments */
  if (ui_top_app_bar_base_create(NULL, &config, &bar) !=
      UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }
  if (ui_top_app_bar_base_create(arena, NULL, &bar) !=
      UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }
  if (ui_top_app_bar_base_create(arena, &config, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }
  if (ui_top_app_bar_base_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  if (ui_top_app_bar_base_handle_scroll(NULL, 10.0f, 10.0f) !=
      UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }
  if (ui_top_app_bar_base_get_state_signal(NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }
  if (ui_top_app_bar_base_get_height_signal(NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  /* Test allocation failures */
#ifdef UI_TEST_MOCK_ALLOC
  for (i = 0; i < 10; i++) {
    g_malloc_fail_countdown = i;
    if (ui_top_app_bar_base_create(arena, &config, &bar) == UI_ERROR_NONE) {
      ui_top_app_bar_base_destroy(bar);
    }
  }
  g_malloc_fail_countdown = -1;

  /* Mocks */
  g_top_app_bar_mock_fail = 1;
  if (ui_top_app_bar_base_create(arena, &config, &bar) != UI_ERROR_UNKNOWN) {
    failed = 1;
  }
  g_top_app_bar_mock_fail = 0;

  g_top_app_bar_mock_fail = 2;
  if (ui_top_app_bar_base_create(arena, &config, &bar) != UI_ERROR_UNKNOWN) {
    failed = 1;
  }
  g_top_app_bar_mock_fail = 0;

  rc = ui_top_app_bar_base_create(arena, &config, &bar);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  g_top_app_bar_mock_fail = 3;
  if (ui_top_app_bar_base_handle_scroll(bar, 10.0f, 10.0f) !=
      UI_ERROR_UNKNOWN) {
    failed = 1;
  }
  g_top_app_bar_mock_fail = 0;

  g_top_app_bar_mock_fail = 4;
  if (ui_top_app_bar_base_handle_scroll(bar, 10.0f, 10.0f) !=
      UI_ERROR_UNKNOWN) {
    failed = 1;
  }
  g_top_app_bar_mock_fail = 0;

  g_top_app_bar_mock_fail = 5;
  if (ui_top_app_bar_base_handle_scroll(bar, 10.0f, 10.0f) !=
      UI_ERROR_UNKNOWN) {
    failed = 1;
  }
  g_top_app_bar_mock_fail = 0;

  g_top_app_bar_mock_fail = 6;
  if (ui_top_app_bar_base_handle_scroll(bar, 10.0f, 10.0f) !=
      UI_ERROR_UNKNOWN) {
    failed = 1;
  }
  g_top_app_bar_mock_fail = 0;

  rc = ui_top_app_bar_base_destroy(bar);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }
#endif

  rc = ui_top_app_bar_base_create(arena, &config, &bar);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  /* Test destroying with NULL signals */
  ui_signal_destroy(bar->state_signal);
  bar->state_signal = NULL;
  ui_signal_destroy(bar->height_signal);
  bar->height_signal = NULL;
  rc = ui_top_app_bar_base_destroy(bar);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  config.initial_state = UI_TOP_APP_BAR_STATE_COLLAPSED;
  rc = ui_top_app_bar_base_create(arena, &config, &bar);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  if (ui_top_app_bar_base_get_state_signal(bar, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }
  if (ui_top_app_bar_base_get_height_signal(bar, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  rc = ui_top_app_bar_base_destroy(bar);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
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
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  config.initial_state = UI_TOP_APP_BAR_STATE_EXPANDED;
  config.expanded_height = 120.0f;
  config.collapsed_height = 64.0f;
  config.scroll_threshold = 50.0f;

  rc = ui_top_app_bar_base_create(arena, &config, &bar);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  rc = ui_top_app_bar_base_handle_scroll(bar, 10.0f, 10.0f);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc = ui_top_app_bar_base_get_height_signal(bar, &height_signal);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc = ui_signal_get(height_signal, &payload);
  if (rc != UI_ERROR_NONE || payload.float_val != 110.0f) {
    failed = 1;
  }

  rc = ui_top_app_bar_base_get_state_signal(bar, &state_signal);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc = ui_signal_get(state_signal, &payload);
  if (rc != UI_ERROR_NONE ||
      payload.int_val != (ui_int32)UI_TOP_APP_BAR_STATE_FLOATING) {
    failed = 1;
  }

  /* Scroll past collapsed */
  rc = ui_top_app_bar_base_handle_scroll(bar, 100.0f, 90.0f);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  /* Scroll with no change to cover unchanged branches */
  rc = ui_top_app_bar_base_handle_scroll(bar, 100.0f, 0.0f);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc = ui_signal_get(height_signal, &payload);
  if (rc != UI_ERROR_NONE || payload.float_val != 64.0f) {
    failed = 1;
  }

  rc = ui_signal_get(state_signal, &payload);
  if (rc != UI_ERROR_NONE ||
      payload.int_val != (ui_int32)UI_TOP_APP_BAR_STATE_COLLAPSED) {
    failed = 1;
  }

  /* Scroll back up past expanded */
  rc = ui_top_app_bar_base_handle_scroll(bar, 0.0f, -100.0f);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc = ui_signal_get(state_signal, &payload);
  if (rc != UI_ERROR_NONE ||
      payload.int_val != (ui_int32)UI_TOP_APP_BAR_STATE_EXPANDED) {
    failed = 1;
  }

  rc = ui_top_app_bar_base_destroy(bar);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  return failed;
}

#include <string.h>

static int test_top_app_bar_edge_cases(void) {
  struct ui_top_app_bar_base *bar = NULL;
  struct ui_top_app_bar_base dummy;
  struct ui_component *comp;
  ui_error_t rc;
  int i;

  assert(UI_ERROR_INVALID_ARGUMENT ==
         ui_top_app_bar_base_get_component(NULL, &comp));
  assert(UI_ERROR_INVALID_ARGUMENT ==
         ui_top_app_bar_base_get_component(&dummy, NULL));

#ifdef UI_TEST_MOCK_ALLOC
  for (i = 0; i < 50; i++) {
    struct ui_arena *arena;
    struct ui_top_app_bar_config config;
    memset(&config, 0, sizeof(config));

    rc = ui_arena_create(2048, &arena);
    assert(rc == UI_ERROR_NONE);

    g_malloc_fail_countdown = i;
    rc = ui_top_app_bar_base_create(arena, &config, &bar);
    if (rc != UI_ERROR_NONE && bar) {
      ui_top_app_bar_base_destroy(bar);
    }
    g_malloc_fail_countdown = -1;

    ui_arena_destroy(arena);
  }
#endif

  return 0;
}

int main(void) {
  int result = 0;
  printf("Running ui_top_app_bar_base tests...\n");

  result |= test_top_app_bar_lifecycle();
  result |= test_top_app_bar_scroll();
  result |= test_top_app_bar_edge_cases();

  if (result == 0) {
    printf("All top app bar tests PASSED\n");
  }

  return result;
}
