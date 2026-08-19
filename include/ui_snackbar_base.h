/**
 * @file ui_snackbar_base.h
 * @brief Snackbar base component declarations.
 *
 * @defgroup SnackbarBase Snackbar Base
 * @brief Base implementation for snackbar notifications.
 * @{
 */

#ifndef UI_SNACKBAR_BASE_H
#define UI_SNACKBAR_BASE_H

/**
 * @brief Opaque structure for a computed signal.
 */
struct ui_computed;
/**
 * @brief Opaque structure for a signal.
 */
struct ui_signal;

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_timer.h"
#include "ui_event.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct ui_snackbar_base
 * @brief Opaque handle for a snackbar base component instance.
 */
struct ui_snackbar_base;

/**
 * @struct ui_overlay_director
 * @brief Opaque handle for the overlay director responsible for presenting
 * snackbars.
 */
struct ui_overlay_director;

/**
 * @brief Signature for the inline action callback.
 *
 * @param snackbar The snackbar instance calling the action.
 * @param user_data User data passed in the configuration.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_snackbar_action_cb)(struct ui_snackbar_base *snackbar,
                                            void *user_data);

/**
 * @struct ui_snackbar_config
 * @brief Configuration for a single snackbar notification.
 */
struct ui_snackbar_config {
  /** @brief The message to display. */
  const char *message;
  /** @brief The text label for the action button, or NULL if none. */
  const char *action_label;
  /** @brief Callback executed when the action button is clicked. */
  ui_snackbar_action_cb action_callback;
  /** @brief User data to pass to the action callback. */
  void *action_user_data;
  /** @brief Time to stay visible before auto-dismiss (0 for infinite). */
  double duration_secs;
};

/**
 * @brief Creates a new base snackbar instance (acting as a queue/manager for
 * snackbars).
 *
 * @param timer The timer instance to use for auto-dismissal tracking.
 * @param director The overlay director to use for rendering.
 * @param out_snackbar Pointer to receive the allocated snackbar instance.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY if allocation fails.
 */
ui_error_t ui_snackbar_base_create(struct ui_timer *timer,
                                   struct ui_overlay_director *director,
                                   struct ui_snackbar_base **out_snackbar);

/**
 * @brief Destroys a snackbar instance and frees its resources.
 *
 * @param snackbar The snackbar instance to destroy. If null, does nothing.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_snackbar_base_destroy(struct ui_snackbar_base *snackbar);

/**
 * @brief Enqueues a snackbar to be shown.
 *
 * @param snackbar The snackbar base manager.
 * @param config The configuration for the snackbar to show.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if null.
 */
ui_error_t ui_snackbar_base_enqueue(struct ui_snackbar_base *snackbar,
                                    const struct ui_snackbar_config *config);

/**
 * @brief Dismisses the currently active snackbar.
 *
 * @param snackbar The snackbar base manager.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_snackbar_base_dismiss_current(struct ui_snackbar_base *snackbar);

/**
 * @brief Ticks the snackbar logic to handle timers and queue progression.
 *
 * @param snackbar The snackbar base manager.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_snackbar_base_tick(struct ui_snackbar_base *snackbar);

/**
 * @brief Process events (like clicks on the action button).
 *
 * @param snackbar The snackbar base manager.
 * @param event The event to process.
 * @param timestamp_ms Event timestamp in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_snackbar_base_process_event(struct ui_snackbar_base *snackbar,
                                          const struct ui_event *event,
                                          double timestamp_ms);

/**
 * @brief Binds the open state to a signal.
 *
 * @param widget The snackbar widget manager.
 * @param open_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_snackbar_base_bind_open(struct ui_snackbar_base *widget,
                                      struct ui_signal *open_signal);

/**
 * @brief Retrieves the computed signal indicating if the widget is animating.
 *
 * @param widget The snackbar widget manager.
 * @param out_animating Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_snackbar_base_get_animating_signal(struct ui_snackbar_base *widget,
                                      struct ui_computed **out_animating);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SNACKBAR_BASE_H */

/** @} */
