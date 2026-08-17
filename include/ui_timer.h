/**
 * @file ui_timer.h
 * @brief Defines the timer logic for abstract and monotonic timers.
 */
#ifndef UI_TIMER_H
#define UI_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @struct ui_timer
 * @brief Opaque handle representing an abstract timer.
 */
struct ui_timer;

/**
 * @struct ui_timer_config
 * @brief Configuration for a custom timer source.
 */
struct ui_timer_config {
  /**
   * @brief Callback function to retrieve the current time.
   *
   * @param user_data Opaque user data provided during creation.
   * @param out_time_secs Pointer to receive the current time in seconds.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*time_source)(void *user_data, double *out_time_secs);

  /** @brief Opaque user data passed to the time_source callback. */
  void *user_data;
};

/**
 * @brief Creates a timer slaved to a custom source (e.g., A/V presentation
 * timestamps).
 *
 * @param config Pointer to the configuration struct.
 * @param out_timer Pointer to receive the new timer handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_timer_create_custom(const struct ui_timer_config *config,
                                  struct ui_timer **out_timer);

/**
 * @brief Creates a timer slaved to the OS monotonic clock.
 *
 * @param out_timer Pointer to receive the new timer handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_timer_create_monotonic(struct ui_timer **out_timer);

/**
 * @brief Destroys a timer and frees its resources.
 *
 * @param timer The timer to destroy.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if timer is NULL.
 */
ui_error_t ui_timer_destroy(struct ui_timer *timer);

/**
 * @brief Gets the current time from the timer in seconds.
 *
 * @param timer The timer instance.
 * @param out_time_secs Pointer to receive the time.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_timer_now(struct ui_timer *timer, double *out_time_secs);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TIMER_H */
