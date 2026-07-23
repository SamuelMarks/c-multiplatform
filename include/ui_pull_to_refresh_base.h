#ifndef UI_PULL_TO_REFRESH_BASE_H
#define UI_PULL_TO_REFRESH_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_event.h"
/* clang-format on */

struct ui_pull_to_refresh_base;

/**
 * @brief States for the pull-to-refresh component.
 */
enum ui_pull_to_refresh_state {
  UI_PULL_TO_REFRESH_RESTING = 0,
  UI_PULL_TO_REFRESH_PULLING,
  UI_PULL_TO_REFRESH_REFRESHING,
  UI_PULL_TO_REFRESH_COMPLETING
};

/**
 * @brief Callback invoked when a refresh is triggered.
 */
typedef enum ui_error (*ui_pull_to_refresh_on_refresh_t)(
    struct ui_pull_to_refresh_base *ptr, void *user_data);

/**
 * @brief Creates a pull-to-refresh base component.
 */
enum ui_error
ui_pull_to_refresh_base_create(struct ui_pull_to_refresh_base **out_ptr);

/**
 * @brief Destroys a pull-to-refresh base component.
 */
enum ui_error
ui_pull_to_refresh_base_destroy(struct ui_pull_to_refresh_base *ptr);

/**
 * @brief Sets the callback for when the refresh action is fully triggered.
 */
enum ui_error ui_pull_to_refresh_base_set_on_refresh(
    struct ui_pull_to_refresh_base *ptr,
    ui_pull_to_refresh_on_refresh_t on_refresh, void *user_data);

/**
 * @brief Marks the refresh as complete (transitions from REFRESHING to
 * COMPLETING -> RESTING).
 */
enum ui_error
ui_pull_to_refresh_base_complete(struct ui_pull_to_refresh_base *ptr);

/**
 * @brief Gets the current state of the pull-to-refresh mechanism.
 */
enum ui_pull_to_refresh_state
ui_pull_to_refresh_base_get_state(const struct ui_pull_to_refresh_base *ptr);

/**
 * @brief Gets the current pull progress (0.0 to 1.0+).
 */
enum ui_error
ui_pull_to_refresh_base_get_progress(const struct ui_pull_to_refresh_base *ptr,
                                     float *out_progress);

/**
 * @brief Processes an incoming input event (gesture/pan tracking).
 */
enum ui_error
ui_pull_to_refresh_base_process_event(struct ui_pull_to_refresh_base *ptr,
                                      const struct ui_event *event,
                                      double timestamp_ms);

/**
 * @brief Integrates physics and timers (springing back, state transitions).
 */
enum ui_error
ui_pull_to_refresh_base_on_tick(struct ui_pull_to_refresh_base *ptr,
                                double delta_ms);

/**
 * @brief Retrieves the underlying container component.
 */
enum ui_error
ui_pull_to_refresh_base_get_component(struct ui_pull_to_refresh_base *ptr,
                                      struct ui_component **out_component);

/**
 * @brief Sets the spinner component that visually represents the progress.
 */
enum ui_error
ui_pull_to_refresh_base_set_spinner(struct ui_pull_to_refresh_base *ptr,
                                    struct ui_component *spinner_comp);

/**
 * @brief Binds the refreshing state to a signal.
 *
 * @param widget The widget.
 * @param refreshing_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_pull_to_refresh_base_bind_refreshing(struct ui_pull_to_refresh_base *widget,
                                        struct ui_signal *refreshing_signal);

/**
 * @brief Retrieves the computed signal indicating if the widget is refreshing.
 *
 * @param widget The widget.
 * @param out_refreshing Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_pull_to_refresh_base_get_refreshing_signal(
    struct ui_pull_to_refresh_base *widget,
    struct ui_computed **out_refreshing);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_PULL_TO_REFRESH_BASE_H */
