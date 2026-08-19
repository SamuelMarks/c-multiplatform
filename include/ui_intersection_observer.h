/**
 * @file ui_intersection_observer.h
 * @brief Intersection observer for tracking elements entering or leaving
 * viewport bounds.
 */

#ifndef UI_INTERSECTION_OBSERVER_H
#define UI_INTERSECTION_OBSERVER_H

/* clang-format off */
#include "ui_error.h"
#include "ui_dom_node.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents an intersection observer instance.
 */
struct ui_intersection_observer;

/**
 * @brief Represents the details of an intersection event.
 */
struct ui_intersection_observer_entry {
  struct ui_dom_node *target; /**< The element whose intersection changed */
  int is_intersecting;        /**< 1 if intersecting, 0 otherwise */
  float intersection_ratio;   /**< The ratio of the intersection area to the
                                 target's bounding box area */
};

/**
 * @brief Callback function type for intersection events.
 * @param observer The observer instance.
 * @param entries Array of intersection entries.
 * @param entry_count Number of entries in the array.
 * @param user_data Opaque pointer passed during subscription.
 */
typedef ui_error_t (*ui_intersection_observer_cb_t)(
    struct ui_intersection_observer *observer,
    const struct ui_intersection_observer_entry *entries, int entry_count,
    void *user_data);

/**
 * @brief Creates a new intersection observer.
 * @param root The root element to observe against. If NULL, the viewport is
 * used.
 * @param root_margin_px The margin (in pixels) applied to the root's bounding
 * box.
 * @param thresholds An array of intersection ratio thresholds to trigger
 * events.
 * @param threshold_count The number of thresholds in the array.
 * @param out_observer Pointer to receive the new observer instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_intersection_observer_create(struct ui_dom_node *root, int root_margin_px,
                                const float *thresholds, int threshold_count,
                                struct ui_intersection_observer **out_observer);

/**
 * @brief Destroys an intersection observer.
 * @param observer The observer to destroy.
 */
ui_error_t
ui_intersection_observer_destroy(struct ui_intersection_observer *observer);

/**
 * @brief Starts observing a target element.
 * @param observer The observer instance.
 * @param target The DOM node to observe.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_intersection_observer_observe(struct ui_intersection_observer *observer,
                                 struct ui_dom_node *target);

/**
 * @brief Stops observing a target element.
 * @param observer The observer instance.
 * @param target The DOM node to stop observing.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_intersection_observer_unobserve(struct ui_intersection_observer *observer,
                                   struct ui_dom_node *target);

/**
 * @brief Disconnects the observer, stopping observation of all targets.
 * @param observer The observer instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_intersection_observer_disconnect(struct ui_intersection_observer *observer);

/**
 * @brief Subscribes a callback to receive intersection events.
 * @param observer The observer instance.
 * @param callback The function to call when intersections change.
 * @param user_data Opaque pointer to pass to the callback.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_intersection_observer_subscribe(struct ui_intersection_observer *observer,
                                   ui_intersection_observer_cb_t callback,
                                   void *user_data);

/**
 * @brief Triggers an evaluation of intersections. Typically called during the
 * layout/render loop when scroll or layout changes.
 * @param observer The observer instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_intersection_observer_evaluate(struct ui_intersection_observer *observer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_INTERSECTION_OBSERVER_H */
