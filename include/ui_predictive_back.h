/* clang-format off */
#ifndef UI_PREDICTIVE_BACK_H
#define UI_PREDICTIVE_BACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_error.h"
#include "ui_signal.h"
#include "ui_event.h"

/* clang-format on */

struct ui_predictive_back;

/**
 * @brief Creates a new predictive back gesture tracker.
 *
 * @param out_tracker Pointer to receive the allocated tracker.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_predictive_back_create(struct ui_predictive_back **out_tracker);

/**
 * @brief Destroys a predictive back gesture tracker.
 *
 * @param tracker The tracker to destroy.
 */
ui_error_t ui_predictive_back_destroy(struct ui_predictive_back *tracker);

/**
 * @brief Configures the threshold properties for the swipe.
 *
 * @param tracker The tracker.
 * @param edge_width_px How far from the screen edge the swipe must start.
 * @param screen_width_px Total screen width to calculate fractional progress.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_predictive_back_configure(struct ui_predictive_back *tracker,
                                        int edge_width_px, int screen_width_px);

/**
 * @brief Binds a signal that will receive the fractional progress (float 0.0
 * to 1.0).
 *
 * @param tracker The tracker.
 * @param progress_signal The signal to update during a swipe.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_predictive_back_bind_progress(struct ui_predictive_back *tracker,
                                            struct ui_signal *progress_signal);

/**
 * @brief Binds a signal that will emit when a swipe completes and routing
 * should occur. The payload is an int (1 for successful back navigation).
 *
 * @param tracker The tracker.
 * @param commit_signal The signal to emit on success.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_predictive_back_bind_commit(struct ui_predictive_back *tracker,
                                          struct ui_signal *commit_signal);

/**
 * @brief Processes an input event (typically touch) to track edge swipe
 * physics.
 *
 * @param tracker The tracker.
 * @param event The input event.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_predictive_back_process_event(struct ui_predictive_back *tracker,
                                            const struct ui_event *event);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_PREDICTIVE_BACK_H */
