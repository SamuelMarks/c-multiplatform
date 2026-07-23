/* clang-format off */
#include "ui_breakpoint_observer.h"
#include "ui_internal_mem.h"
#include "ui_window_manager_base.h"
#include "ui_arena.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#define DEBOUNCE_DELAY_MS 50.0

struct ui_breakpoint_observer {
  struct ui_window_manager_base *window_manager;
  struct ui_arena *arena;
  struct ui_signal *signals[UI_BREAKPOINT_COUNT];

  float last_width;
  float target_width;
  double last_resize_time;
  int is_debouncing;
};

static enum ui_error
evaluate_breakpoints(struct ui_breakpoint_observer *observer, float width) {
  union ui_signal_payload is_xsmall = {0};
  union ui_signal_payload is_small = {0};
  union ui_signal_payload is_medium = {0};
  union ui_signal_payload is_large = {0};
  union ui_signal_payload is_xlarge = {0};
  enum ui_error rc = UI_ERROR_NONE;

  is_xsmall.bool_val = width < 600.0f ? 1 : 0;
  is_small.bool_val = width >= 600.0f && width < 900.0f ? 1 : 0;
  is_medium.bool_val = width >= 900.0f && width < 1200.0f ? 1 : 0;
  is_large.bool_val = width >= 1200.0f && width < 1536.0f ? 1 : 0;
  is_xlarge.bool_val = width >= 1536.0f ? 1 : 0;

  ui_signal_set(observer->signals[UI_BREAKPOINT_XSMALL], is_xsmall);
  ui_signal_set(observer->signals[UI_BREAKPOINT_SMALL], is_small);
  ui_signal_set(observer->signals[UI_BREAKPOINT_MEDIUM], is_medium);
  ui_signal_set(observer->signals[UI_BREAKPOINT_LARGE], is_large);
  ui_signal_set(observer->signals[UI_BREAKPOINT_XLARGE], is_xlarge);

  return rc;
}

/** \brief ui_error */
enum ui_error
ui_breakpoint_observer_create(struct ui_window_manager_base *window_manager,
                              struct ui_breakpoint_observer **out_observer) {
  struct ui_breakpoint_observer *observer;
  union ui_signal_payload initial_payload = {0};
  int i;
  enum ui_error rc;

  if (!out_observer)
    return UI_ERROR_INVALID_ARGUMENT;

  observer = (struct ui_breakpoint_observer *)UI_MALLOC(
      sizeof(struct ui_breakpoint_observer));
  if (!observer)
    return UI_ERROR_OUT_OF_MEMORY;

  memset(observer, 0, sizeof(struct ui_breakpoint_observer));

  rc = ui_arena_create(1024, &observer->arena);
  if (rc != UI_ERROR_NONE) {
    UI_FREE(observer);
    return rc;
  }

  observer->window_manager = window_manager;
  observer->last_width = 0.0f;
  observer->target_width = 0.0f;
  observer->last_resize_time = 0.0;
  observer->is_debouncing = 0;

  initial_payload.bool_val = 0;

  for (i = 0; i < UI_BREAKPOINT_COUNT; i++) {
    ui_signal_create(observer->arena, initial_payload, UI_SIGNAL_TYPE_BOOL,
                     NULL, NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                     &observer->signals[i]);
  }

  *out_observer = observer;
  return UI_ERROR_NONE;
}

void ui_breakpoint_observer_destroy(struct ui_breakpoint_observer *observer) {
  int i;
  if (!observer)
    return;

  for (i = 0; i < UI_BREAKPOINT_COUNT; i++) {
    ui_signal_destroy(observer->signals[i]);
  }

  ui_arena_destroy(observer->arena);

  UI_FREE(observer);
}

/** \brief ui_error */
enum ui_error
ui_breakpoint_observer_get_signal(struct ui_breakpoint_observer *observer,
                                  enum ui_breakpoint breakpoint,
                                  struct ui_signal **out_signal) {
  if (!observer || !out_signal || breakpoint >= UI_BREAKPOINT_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_signal = observer->signals[breakpoint];
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_breakpoint_observer_tick(struct ui_breakpoint_observer *observer,
                            float current_width, double current_time_ms) {
  if (!observer)
    return UI_ERROR_INVALID_ARGUMENT;

  if (current_width != observer->target_width) {
    observer->target_width = current_width;
    observer->last_resize_time = current_time_ms;
    observer->is_debouncing = 1;
  }

  if (observer->is_debouncing &&
      (current_time_ms - observer->last_resize_time >= DEBOUNCE_DELAY_MS)) {
    observer->last_width = observer->target_width;
    (void)evaluate_breakpoints(observer, observer->last_width);
    observer->is_debouncing = 0;
  }

  return UI_ERROR_NONE;
}
