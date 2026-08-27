/* clang-format off */
#include "ui_breakpoint_observer.h"
#include "ui_internal_mem.h"
#include "ui_window_manager_base.h"
#include "ui_arena.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_breakpoint_mock_fail = 0;
/*
 * @brief mock_ui_signal_set.
 * @param signal Parameter signal.
 * @param value Parameter value.
 * @return Return value.
 */
static ui_error_t mock_ui_signal_set(struct ui_signal *signal,
                                     union ui_signal_payload value) {
  if (g_breakpoint_mock_fail > 0) {
    g_breakpoint_mock_fail--;
    if (g_breakpoint_mock_fail == 0) {
      return UI_ERROR_UNKNOWN;
    }
  }
  return ui_signal_set(signal, value);
}
/** @cond */
#define ui_signal_set mock_ui_signal_set
/** @endcond */
#endif

/** @brief internal */
#define DEBOUNCE_DELAY_MS 50.0

/**
 * @struct ui_breakpoint_observer
 * @struct ui_breakpoint_observer
 * @brief Internal representation of a breakpoint observer.
 */
struct ui_breakpoint_observer {
  /* @brief Reference to the window manager (if needed for context). */
  struct ui_window_manager_base *window_manager; /**< window_manager */
  /* @brief Pointer to the memory arena used for allocations. */
  struct ui_arena *arena; /**< arena */
  /* @brief Array of boolean signals for each breakpoint. */
  struct ui_signal *signals[UI_BREAKPOINT_COUNT]; /**< signals */

  /* @brief The width evaluated during the last successful tick. */
  float last_width; /**< last_width */
  /* @brief The current pending width being debounced. */
  float target_width; /**< target_width */
  /* @brief Timestamp of the last width change detection. */
  double last_resize_time; /**< last_resize_time */
  /* @brief Flag indicating if a resize is currently being debounced. */
  int is_debouncing; /**< is_debouncing */
};

/*
 * @brief evaluate_breakpoints.
 * @param observer Parameter observer.
 * @param width Parameter width.
 * @return Return value.
 */
static ui_error_t evaluate_breakpoints(struct ui_breakpoint_observer *observer,
                                       float width) {
  union ui_signal_payload is_xsmall = {0};
  union ui_signal_payload is_small = {0};
  union ui_signal_payload is_medium = {0};
  union ui_signal_payload is_large = {0};
  union ui_signal_payload is_xlarge = {0};
  ui_error_t rc = UI_ERROR_NONE;

  is_xsmall.bool_val = width < 600.0f ? 1 : 0;
  is_small.bool_val = width >= 600.0f && width < 900.0f ? 1 : 0;
  is_medium.bool_val = width >= 900.0f && width < 1200.0f ? 1 : 0;
  is_large.bool_val = width >= 1200.0f && width < 1536.0f ? 1 : 0;
  is_xlarge.bool_val = width >= 1536.0f ? 1 : 0;

  rc = ui_signal_set(observer->signals[UI_BREAKPOINT_XSMALL], is_xsmall);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  rc = ui_signal_set(observer->signals[UI_BREAKPOINT_SMALL], is_small);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  rc = ui_signal_set(observer->signals[UI_BREAKPOINT_MEDIUM], is_medium);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  rc = ui_signal_set(observer->signals[UI_BREAKPOINT_LARGE], is_large);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  rc = ui_signal_set(observer->signals[UI_BREAKPOINT_XLARGE], is_xlarge);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return rc;
}

/*
 * @brief ui_breakpoint_observer_create.
 * @param window_manager Parameter window_manager.
 * @param out_observer Parameter out_observer.
 * @return Return value.
 */
ui_error_t
ui_breakpoint_observer_create(struct ui_window_manager_base *window_manager,
                              struct ui_breakpoint_observer **out_observer) {
  struct ui_breakpoint_observer *observer;
  union ui_signal_payload initial_payload = {0};
  int i;
  ui_error_t rc;

  if (!out_observer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  observer = (struct ui_breakpoint_observer *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_breakpoint_observer));
  if (!observer) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(observer, 0, sizeof(struct ui_breakpoint_observer));

  rc = ui_arena_create(1024, &observer->arena);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(observer);
    return rc;
  }

  observer->window_manager = window_manager;
  observer->last_width = 0.0f;
  observer->target_width = 0.0f;
  observer->last_resize_time = 0.0;
  observer->is_debouncing = 0;

  initial_payload.bool_val = 0;

  for (i = 0; i < UI_BREAKPOINT_COUNT; i++) {
    (void)ui_signal_create(
        observer->arena, initial_payload, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
        UI_SIGNAL_MODE_SINGLE_THREADED, &observer->signals[i]);
  }

  *out_observer = observer;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_breakpoint_observer_destroy.
 * @param observer Parameter observer.
 * @return Return value.
 */
ui_error_t
ui_breakpoint_observer_destroy(struct ui_breakpoint_observer *observer) {
  int i;
  if (!observer) {
    return UI_ERROR_NONE;
  }

  for (i = 0; i < UI_BREAKPOINT_COUNT; i++) {
    (void)ui_signal_destroy(observer->signals[i]);
  }

  (void)ui_arena_destroy(observer->arena);

  C_MULTIPLATFORM_FREE(observer);
  return UI_ERROR_NONE;
}

/*
 * @brief ui_breakpoint_observer_get_signal.
 * @param observer Parameter observer.
 * @param breakpoint Parameter breakpoint.
 * @param out_signal Parameter out_signal.
 * @return Return value.
 */
ui_error_t
ui_breakpoint_observer_get_signal(struct ui_breakpoint_observer *observer,
                                  enum ui_breakpoint breakpoint,
                                  struct ui_signal **out_signal) {
  if (!observer || !out_signal || breakpoint >= UI_BREAKPOINT_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_signal = observer->signals[breakpoint];
  return UI_ERROR_NONE;
}

/*
 * @brief ui_breakpoint_observer_tick.
 * @param observer Parameter observer.
 * @param current_width Parameter current_width.
 * @param current_time_ms Parameter current_time_ms.
 * @return Return value.
 */
ui_error_t ui_breakpoint_observer_tick(struct ui_breakpoint_observer *observer,
                                       float current_width,
                                       double current_time_ms) {
  if (!observer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (current_width != observer->target_width) {
    observer->target_width = current_width;
    observer->last_resize_time = current_time_ms;
    observer->is_debouncing = 1;
  }

  if (observer->is_debouncing &&
      (current_time_ms - observer->last_resize_time >= DEBOUNCE_DELAY_MS)) {
    ui_error_t rc;
    observer->last_width = observer->target_width;
    rc = evaluate_breakpoints(observer, observer->last_width);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    observer->is_debouncing = 0;
  }

  return UI_ERROR_NONE;
}
