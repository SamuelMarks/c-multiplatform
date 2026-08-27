/**
 * @file ui_timer.c
 * @brief Implementation of the abstract and monotonic timer logic.
 */
#if !defined(_WIN32)
#if !defined(_POSIX_C_SOURCE)
/** @brief internal */
#define _POSIX_C_SOURCE 199309L
#endif
#endif

/* clang-format off */
#include "../include/ui_timer.h"
#include "ui_internal_mem.h"

#if defined(_WIN32)

#ifndef UI_WINAPI
#if defined(_MSC_VER)
/** @cond */
#define UI_WINAPI __stdcall
/** @endcond */
#elif defined(__GNUC__)
/** @cond */
#define UI_WINAPI __attribute__((stdcall))
/** @endcond */
#else
/** @cond */
#define UI_WINAPI
/** @endcond */
#endif
#endif

#if defined(_MSC_VER)
/** @cond */
#define UI_TIMER_INT64 __int64
/** @endcond */
#elif defined(__GNUC__) || defined(__clang__)
__extension__ typedef long long ui_timer_int64;
/** @cond */
#define UI_TIMER_INT64 ui_timer_int64
/** @endcond */
#else
/** @cond */
#define UI_TIMER_INT64 long long
/** @endcond */
#endif

extern int UI_WINAPI QueryPerformanceCounter(UI_TIMER_INT64* lpPerformanceCount);
extern int UI_WINAPI QueryPerformanceFrequency(UI_TIMER_INT64* lpFrequency);

#else
#include <time.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_ui_timer_clock_gettime_fail;
#endif

#endif

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

/** @brief internal */
enum ui_timer_type { UI_TIMER_CUSTOM, UI_TIMER_MONOTONIC };

/**
 * @struct ui_timer
 * @struct ui_timer
 * @brief Internal implementation of the timer object.
 */
struct ui_timer {
  /* @brief Type of the timer. */
  enum ui_timer_type type; /**< type */
  /* @brief Custom time source callback. */
  ui_error_t (*custom_time_source)(void *, double *); /**< ) */
  /* @brief User data for the custom time source. */
  void *custom_user_data; /**< custom_user_data */

#if defined(_WIN32)
  /* @brief Windows QueryPerformanceFrequency value. */
  UI_TIMER_INT64 qpf; /**< qpf */
#endif
};

ui_error_t ui_timer_create_custom(const struct ui_timer_config *config,
                                  struct ui_timer **out_timer) {
  struct ui_timer *timer = NULL;

  if (!config || !out_timer || !config->time_source) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  timer = (struct ui_timer *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_timer));
  if (!timer) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  timer->type = UI_TIMER_CUSTOM;
  timer->custom_time_source = config->time_source;
  timer->custom_user_data = config->user_data;
#if defined(_WIN32)
  timer->qpf = 0;
#endif

  *out_timer = timer;
  return UI_ERROR_NONE;
}

ui_error_t ui_timer_create_monotonic(struct ui_timer **out_timer) {
  struct ui_timer *timer = NULL;

  if (!out_timer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  timer = (struct ui_timer *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_timer));
  if (!timer) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  timer->type = UI_TIMER_MONOTONIC;
  timer->custom_time_source = NULL;
  timer->custom_user_data = NULL;

#if defined(_WIN32)
  if (!QueryPerformanceFrequency(&timer->qpf) || timer->qpf == 0) {
    C_MULTIPLATFORM_FREE(timer);
    return UI_ERROR_UNKNOWN;
  }
#endif

  *out_timer = timer;
  return UI_ERROR_NONE;
}

ui_error_t ui_timer_destroy(struct ui_timer *timer) {
  if (!timer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(timer);
  return UI_ERROR_NONE;
}

ui_error_t ui_timer_now(struct ui_timer *timer, double *out_time_secs) {
  if (!timer || !out_time_secs) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (timer->type == UI_TIMER_CUSTOM) {
    return timer->custom_time_source(timer->custom_user_data, out_time_secs);
  }

#if defined(_WIN32)
  {
    UI_TIMER_INT64 now = 0;
    if (!QueryPerformanceCounter(&now)) {
      return UI_ERROR_UNKNOWN;
    }
    *out_time_secs = (double)now / (double)timer->qpf;
    return UI_ERROR_NONE;
  }
#elif defined(__EMSCRIPTEN__)
  {
    *out_time_secs = emscripten_get_now() / 1000.0;
    return UI_ERROR_NONE;
  }
#else
  {
    struct timespec ts;
    int rc_clock;
#ifdef UI_TEST_MOCK_ALLOC
    if (g_ui_timer_clock_gettime_fail) {
      rc_clock = -1;
    } else
#endif
    {
      rc_clock = clock_gettime(CLOCK_MONOTONIC, &ts);
    }
    if (rc_clock != 0) {
      return UI_ERROR_UNKNOWN;
    }
    *out_time_secs = (double)ts.tv_sec + ((double)ts.tv_nsec / 1e9);
    return UI_ERROR_NONE;
  }
#endif
}
