/**
 * @file ui_breakpoint_observer.h
 * @brief Window breakpoint observer for responsive design.
 */

#ifndef UI_BREAKPOINT_OBSERVER_H
#define UI_BREAKPOINT_OBSERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_signal.h"
/* clang-format on */

/** @brief Opaque handle to a breakpoint observer. */
struct ui_breakpoint_observer;

/** @brief Forward declaration of the window manager base component. */
struct ui_window_manager_base;

/**
 * @brief Represents standard breakpoints.
 */
enum ui_breakpoint {
  UI_BREAKPOINT_XSMALL, /**< Extra small (e.g. < 600px) */
  UI_BREAKPOINT_SMALL,  /**< Small (e.g. >= 600px) */
  UI_BREAKPOINT_MEDIUM, /**< Medium (e.g. >= 900px) */
  UI_BREAKPOINT_LARGE,  /**< Large (e.g. >= 1200px) */
  UI_BREAKPOINT_XLARGE, /**< Extra large (e.g. >= 1536px) */
  UI_BREAKPOINT_COUNT   /**< Number of standard breakpoints */
};

/**
 * @brief Creates a breakpoint observer attached to a window manager.
 *
 * @param window_manager The window manager to observe for resize events.
 * @param out_observer Pointer to receive the allocated observer.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_breakpoint_observer_create(struct ui_window_manager_base *window_manager,
                              struct ui_breakpoint_observer **out_observer);

/**
 * @brief Destroys a breakpoint observer.
 *
 * @param observer The observer to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_breakpoint_observer_destroy(struct ui_breakpoint_observer *observer);

/**
 * @brief Retrieves the reactive signal representing whether a specific
 * breakpoint is currently active.
 *
 * The signal contains an integer value: 1 if active, 0 if inactive.
 *
 * @param observer The observer instance.
 * @param breakpoint The breakpoint to query.
 * @param out_signal Pointer to receive the signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_breakpoint_observer_get_signal(struct ui_breakpoint_observer *observer,
                                  enum ui_breakpoint breakpoint,
                                  struct ui_signal **out_signal);

/**
 * @brief Ticks the observer to process debounced resize events.
 *
 * @param observer The observer instance.
 * @param current_width The current width of the viewport.
 * @param current_time_ms The current monotonic time in milliseconds.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_breakpoint_observer_tick(struct ui_breakpoint_observer *observer,
                                       float current_width,
                                       double current_time_ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_BREAKPOINT_OBSERVER_H */
