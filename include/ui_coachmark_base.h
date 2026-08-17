#ifndef UI_COACHMARK_BASE_H
#define UI_COACHMARK_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_overlay_director.h"
#include "ui_event.h"
#include "ui_signal.h"
#include "ui_computed.h"
/* clang-format on */

struct ui_coachmark_base;
struct ui_coachmark_step;
struct ui_coachmark_tour;

/**
 * @brief Callback invoked when a tour step changes or ends.
 */
typedef ui_error_t (*ui_coachmark_on_step_change_t)(
    struct ui_coachmark_tour *tour, int current_step, void *user_data);

/**
 * @brief Represents a single step in a coachmark tour.
 */
struct ui_coachmark_step {
  struct ui_component
      *target_component; /**< The element to anchor to and elevate */
  struct ui_component
      *content_component; /**< The content inside the coachmark */
  int allow_skip;         /**< 1 if this step can be skipped, 0 otherwise */
};

/**
 * @brief Creates a coachmark tour component.
 */
ui_error_t ui_coachmark_tour_create(struct ui_overlay_director *director,
                                    struct ui_coachmark_tour **out_tour);

/**
 * @brief Destroys a coachmark tour component.
 */
ui_error_t ui_coachmark_tour_destroy(struct ui_coachmark_tour *tour);

/**
 * @brief Sets the steps for the tour.
 */
ui_error_t ui_coachmark_tour_set_steps(struct ui_coachmark_tour *tour,
                                       const struct ui_coachmark_step *steps,
                                       int step_count);

/**
 * @brief Sets a callback for when the step changes.
 */
ui_error_t
ui_coachmark_tour_set_on_step_change(struct ui_coachmark_tour *tour,
                                     ui_coachmark_on_step_change_t on_change,
                                     void *user_data);

/**
 * @brief Starts the tour at step 0.
 */
ui_error_t ui_coachmark_tour_start(struct ui_coachmark_tour *tour);

/**
 * @brief Advances to the next step.
 */
ui_error_t ui_coachmark_tour_next(struct ui_coachmark_tour *tour);

/**
 * @brief Goes back to the previous step.
 */
ui_error_t ui_coachmark_tour_prev(struct ui_coachmark_tour *tour);

/**
 * @brief Skips/ends the tour.
 */
ui_error_t ui_coachmark_tour_skip(struct ui_coachmark_tour *tour);

/**
 * @brief Binds the coachmark tour's open state to a signal.
 *
 * @param tour The coachmark tour.
 * @param open_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_coachmark_tour_bind_open(struct ui_coachmark_tour *tour,
                                       struct ui_signal *open_signal);

/**
 * @brief Retrieves the computed signal indicating if the tour is animating.
 *
 * @param tour The coachmark tour.
 * @param out_animating Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_coachmark_tour_get_animating_signal(struct ui_coachmark_tour *tour,
                                       struct ui_computed **out_animating);

/**
 * @brief Updates layout/positioning for the current step (useful on window
 * resize).
 */
ui_error_t ui_coachmark_tour_update_layout(struct ui_coachmark_tour *tour,
                                           float viewport_width,
                                           float viewport_height);

/**
 * @brief Processes input events for the coachmark (keyboard navigation like
 * ESC, Tab).
 */
ui_error_t ui_coachmark_tour_process_event(struct ui_coachmark_tour *tour,
                                           const struct ui_event *event);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_COACHMARK_BASE_H */
