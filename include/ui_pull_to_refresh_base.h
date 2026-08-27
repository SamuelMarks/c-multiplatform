/**
 * @file ui_pull_to_refresh_base.h
 */
#ifndef UI_PULL_TO_REFRESH_BASE_H
#define UI_PULL_TO_REFRESH_BASE_H

/**
 * \file ui_pull_to_refresh_base.h
 * \brief UI Pull-to-Refresh Base component.
 *
 * This file contains definitions for a pull-to-refresh component,
 * managing the state machine and gesture tracking.
 */

/** \brief Forward declaration of ui_computed */
struct ui_computed;

/** \brief Forward declaration of ui_signal */
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_event.h"
/* clang-format on */

/** \brief Opaque handle representing a pull-to-refresh component */
struct ui_pull_to_refresh_base;

/**
 * @brief States for the pull-to-refresh component.
 */
enum ui_pull_to_refresh_state {
  UI_PULL_TO_REFRESH_RESTING = 0,    /**< Idle state */
  UI_PULL_TO_REFRESH_PULLING = 1,    /**< Currently being pulled by gesture */
  UI_PULL_TO_REFRESH_REFRESHING = 2, /**< Threshold reached, refreshing */
  UI_PULL_TO_REFRESH_COMPLETING = 3  /**< Refresh finished, animating back */
};

/**
 * @brief Callback invoked when a refresh is triggered.
 *
 * @param ptr The pull-to-refresh component.
 * @param user_data Opaque pointer passed to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_pull_to_refresh_on_refresh_t)(
    struct ui_pull_to_refresh_base *ptr, void *user_data);

/**
 * @brief Creates a pull-to-refresh base component.
 *
 * @param out_ptr Pointer to receive the allocated component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_pull_to_refresh_base_create(struct ui_pull_to_refresh_base **out_ptr);

/**
 * @brief Destroys a pull-to-refresh base component.
 *
 * @param ptr The component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_pull_to_refresh_base_destroy(struct ui_pull_to_refresh_base *ptr);

/**
 * @brief Sets the callback for when the refresh action is fully triggered.
 *
 * @param ptr The component.
 * @param on_refresh The callback to invoke.
 * @param user_data Opaque pointer passed to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_pull_to_refresh_base_set_on_refresh(
    struct ui_pull_to_refresh_base *ptr,
    ui_pull_to_refresh_on_refresh_t on_refresh, void *user_data);

/**
 * @brief Marks the refresh as complete (transitions from REFRESHING to
 * COMPLETING -> RESTING).
 *
 * @param ptr The component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_pull_to_refresh_base_complete(struct ui_pull_to_refresh_base *ptr);

/**
 * @brief Gets the current state of the pull-to-refresh mechanism.
 *
 * @param ptr The component.
 * @return The current state enum.
 */
enum ui_pull_to_refresh_state
ui_pull_to_refresh_base_get_state(const struct ui_pull_to_refresh_base *ptr);

/**
 * @brief Gets the current pull progress (0.0 to 1.0+).
 *
 * @param ptr The component.
 * @param out_progress Pointer to receive the progress value.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_pull_to_refresh_base_get_progress(const struct ui_pull_to_refresh_base *ptr,
                                     float *out_progress);

/**
 * @brief Processes an incoming input event (gesture/pan tracking).
 *
 * @param ptr The component.
 * @param event The incoming event.
 * @param timestamp_ms The timestamp in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_pull_to_refresh_base_process_event(struct ui_pull_to_refresh_base *ptr,
                                      const struct ui_event *event,
                                      double timestamp_ms);

/**
 * @brief Integrates physics and timers (springing back, state transitions).
 *
 * @param ptr The component.
 * @param delta_ms The time delta in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_pull_to_refresh_base_on_tick(struct ui_pull_to_refresh_base *ptr,
                                           double delta_ms);

/**
 * @brief Retrieves the underlying container component.
 *
 * @param ptr The component.
 * @param out_component Pointer to receive the underlying component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_pull_to_refresh_base_get_component(struct ui_pull_to_refresh_base *ptr,
                                      struct ui_component **out_component);

/**
 * @brief Sets the spinner component that visually represents the progress.
 *
 * @param ptr The component.
 * @param spinner_comp The spinner component to attach.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_pull_to_refresh_base_set_spinner(struct ui_pull_to_refresh_base *ptr,
                                    struct ui_component *spinner_comp);

/**
 * @brief Binds the refreshing state to a signal.
 *
 * @param widget The widget.
 * @param refreshing_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_pull_to_refresh_base_bind_refreshing(struct ui_pull_to_refresh_base *widget,
                                        struct ui_signal *refreshing_signal);

/**
 * @brief Retrieves the computed signal indicating if the widget is refreshing.
 *
 * @param widget The widget.
 * @param out_refreshing Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_pull_to_refresh_base_get_refreshing_signal(
    struct ui_pull_to_refresh_base *widget,
    struct ui_computed **out_refreshing);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_PULL_TO_REFRESH_BASE_H */
