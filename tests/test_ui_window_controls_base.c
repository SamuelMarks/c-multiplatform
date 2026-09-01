/* clang-format off */
#include "ui_window_controls_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <ui_reactive_graph.h>
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

static ui_error_t failing_notify_fn(void *user_data) {
  (void)user_data;
  return UI_ERROR_OUT_OF_BOUNDS;
}

int main(void) {
  struct ui_arena *arena;
  struct ui_window_controls_base *controls = NULL;
  ui_error_t err;
  ui_signal_t *state_signal = NULL;
  enum ui_window_control_hit_region region;
  int failed = 0;
  int i;

  if (ui_arena_create(1024 * 16, &arena) != UI_ERROR_NONE) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_create(NULL, &controls);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_create(arena, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

#ifdef UI_TEST_MOCK_ALLOC
  /* OOM test for ui_window_controls_base alloc.
     Since arena blocks are allocated lazily, this fails the first block
     allocation. */
  g_malloc_fail_countdown = 0;
  err = ui_window_controls_base_create(arena, &controls);
  if (err != UI_ERROR_OUT_OF_MEMORY) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }
  g_malloc_fail_countdown = -1;

  /* Force ui_signal_create to fail by using a tight arena.
     We set default_block_size to 1 so each allocation requests exactly what it
     needs. The 1st alloc succeeds, the 2nd alloc fails due to countdown=1. */
  {
    struct ui_arena *tight_arena;
    if (ui_arena_create(1, &tight_arena) == UI_ERROR_NONE) {
      g_malloc_fail_countdown = 1;
      err = ui_window_controls_base_create(tight_arena, &controls);
      if (err != UI_ERROR_OUT_OF_MEMORY) {
        failed = 1;
        printf("Failed at line %d\n", __LINE__);
      }
      g_malloc_fail_countdown = -1;
      {
        ui_error_t rc_cleanup = ui_arena_destroy(tight_arena);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
  }
#endif

  err = ui_window_controls_base_create(arena, &controls);
  if (err != UI_ERROR_NONE || controls == NULL) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_get_state_signal(NULL, &state_signal);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_get_state_signal(controls, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_get_state_signal(controls, &state_signal);
  if (err != UI_ERROR_NONE || state_signal == NULL) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_set_state(NULL, UI_WINDOW_STATE_MAXIMIZED);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_set_state(controls, UI_WINDOW_STATE_MAXIMIZED);
  if (err != UI_ERROR_NONE) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_set_state(controls, UI_WINDOW_STATE_MAXIMIZED);
  if (err != UI_ERROR_NONE) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  /* Subscribe a failing node to force ui_signal_set to return an error (line
   * 99) */
  {
    struct ui_reactive_node dummy_node;
    struct ui_reactive_node *prev_node = NULL;
    union ui_signal_payload dummy_payload;

    dummy_node.notify_fn = failing_notify_fn;
    dummy_node.user_data = NULL;
    ui_reactive_graph_set_current_node(&dummy_node, &prev_node);
    ui_signal_get(state_signal, &dummy_payload);
    ui_reactive_graph_set_current_node(prev_node, NULL);

    err =
        ui_window_controls_base_set_state(controls, UI_WINDOW_STATE_MINIMIZED);
    if (err != UI_ERROR_OUT_OF_BOUNDS) {
      failed = 1;
      printf("Failed at line %d\n", __LINE__);
    }
  }

  err = ui_window_controls_base_set_region_rect(
      NULL, UI_WINDOW_CONTROL_HIT_REGION_CLOSE, 100, 0, 50, 20);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_set_region_rect(
      controls, UI_WINDOW_CONTROL_HIT_REGION_CLOSE, 100, 0, 50, 20);
  if (err != UI_ERROR_NONE) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_set_region_rect(
      controls, UI_WINDOW_CONTROL_HIT_REGION_DRAG, 0, 0, 100, 20);
  if (err != UI_ERROR_NONE) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  /* Test updating an existing region */
  err = ui_window_controls_base_set_region_rect(
      controls, UI_WINDOW_CONTROL_HIT_REGION_CLOSE, 110, 0, 50, 20);
  if (err != UI_ERROR_NONE) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_hit_test(NULL, 120, 10, &region);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_hit_test(controls, 120, 10, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_hit_test(controls, 120, 10, &region);
  if (err != UI_ERROR_NONE || region != UI_WINDOW_CONTROL_HIT_REGION_CLOSE) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_hit_test(controls, 100, 10, &region);
  if (err != UI_ERROR_NONE || region != UI_WINDOW_CONTROL_HIT_REGION_NONE) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_hit_test(controls, 160, 10, &region);
  if (err != UI_ERROR_NONE || region != UI_WINDOW_CONTROL_HIT_REGION_NONE) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_hit_test(controls, 120, -1, &region);
  if (err != UI_ERROR_NONE || region != UI_WINDOW_CONTROL_HIT_REGION_NONE) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_hit_test(controls, 120, 20, &region);
  if (err != UI_ERROR_NONE || region != UI_WINDOW_CONTROL_HIT_REGION_NONE) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_hit_test(controls, 50, 10, &region);
  if (err != UI_ERROR_NONE || region != UI_WINDOW_CONTROL_HIT_REGION_DRAG) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_hit_test(controls, 200, 200, &region);
  if (err != UI_ERROR_NONE || region != UI_WINDOW_CONTROL_HIT_REGION_NONE) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_trigger_hover_intent(
      NULL, UI_WINDOW_CONTROL_HIT_REGION_MAXIMIZE);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_trigger_hover_intent(
      controls, UI_WINDOW_CONTROL_HIT_REGION_NONE);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_trigger_hover_intent(
      controls, UI_WINDOW_CONTROL_HIT_REGION_MAXIMIZE);
  if (err != UI_ERROR_NONE) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  /* Fill up to UI_MAX_WINDOW_REGIONS (16)
     Already have 2 regions (DRAG and CLOSE). So 14 more to reach 16.
     We start region values from 10 to avoid updating existing ones. */
  for (i = 10; i < 10 + 15; ++i) {
    err = ui_window_controls_base_set_region_rect(
        controls, (enum ui_window_control_hit_region)i, 0, 0, 10, 10);
    if (i < 10 + 14) {
      if (err != UI_ERROR_NONE) {
        failed = 1;
        printf("Failed at line %d\n", __LINE__);
      }
    } else {
      if (err != UI_ERROR_OUT_OF_BOUNDS) {
        failed = 1;
        printf("Failed at line %d\n", __LINE__);
      }
    }
  }

  err = ui_window_controls_base_destroy(NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  err = ui_window_controls_base_destroy(controls);
  if (err != UI_ERROR_NONE) {
    failed = 1;
    printf("Failed at line %d\n", __LINE__);
  }

  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return failed;
}
