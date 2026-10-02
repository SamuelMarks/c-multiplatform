/**
 * @file cupertino_gesture.h
 * @brief Cupertino interactive edge swipe back gesture and accessibility
 * two-finger "Z" scrub gesture.
 */

#ifndef CUPERTINO_CUPERTINO_GESTURE_H
#define CUPERTINO_CUPERTINO_GESTURE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_bidi_manager.h"
#include "ui_error.h"
#include "ui_event.h"
#include <stddef.h>
/* clang-format on */

/**
 * @enum cupertino_pop_gesture_state
 * @brief Lifecycle states of an interactive back swipe transition.
 */
enum cupertino_pop_gesture_state {
  CUPERTINO_POP_GESTURE_INACTIVE = 0, /**< No active pop transition. */
  CUPERTINO_POP_GESTURE_TRACKING,     /**< Finger tracking horizontally. */
  CUPERTINO_POP_GESTURE_COMPLETED,    /**< Transition committed to pop. */
  CUPERTINO_POP_GESTURE_CANCELLED     /**< Transition cancelled, spring back. */
};

/**
 * @struct cupertino_pop_gesture
 * @brief Interactive edge swipe pop gesture recognizer.
 */
struct cupertino_pop_gesture {
  enum cupertino_pop_gesture_state state; /**< Current lifecycle state. */
  enum ui_bidi_direction direction; /**< LTR (left edge) or RTL (right edge). */
  float screen_width;  /**< Width of the viewport container in points. */
  float edge_slop;     /**< Max distance from edge to initiate (e.g. 20pt). */
  float start_x;       /**< Touch start X coordinate. */
  float current_x;     /**< Current touch X coordinate. */
  float translation_x; /**< Total horizontal translation offset. */
  float progress;      /**< Normalized transition progress [0.0, 1.0]. */
  float velocity_x;    /**< Release horizontal velocity in pt/s. */
};

/**
 * @enum cupertino_scrub_state
 * @brief Recognition states for two-finger "Z" accessibility scrub gesture.
 */
enum cupertino_scrub_state {
  CUPERTINO_SCRUB_IDLE = 0,  /**< Awaiting touch. */
  CUPERTINO_SCRUB_SEGMENT_1, /**< Swiped horizontally (top stroke of Z). */
  CUPERTINO_SCRUB_SEGMENT_2, /**< Swiped diagonally down (diagonal stroke of Z).
                              */
  CUPERTINO_SCRUB_RECOGNIZED, /**< Completed bottom horizontal stroke of Z. */
  CUPERTINO_SCRUB_FAILED      /**< Motion pattern diverged from Z shape. */
};

/**
 * @struct cupertino_scrub_gesture
 * @brief Accessibility two-finger "Z" scrub gesture recognizer.
 */
struct cupertino_scrub_gesture {
  enum cupertino_scrub_state state; /**< Internal state machine stage. */
  float start_x;                    /**< Starting touch X coordinate. */
  float start_y;                    /**< Starting touch Y coordinate. */
  float last_x;                     /**< Last recorded X coordinate. */
  float last_y;                     /**< Last recorded Y coordinate. */
  int pointer_count; /**< Number of active fingers (must be 2). */
};

/**
 * @brief Initializes an interactive pop gesture recognizer.
 *
 * @param gesture Pointer to gesture structure.
 * @param screen_width Viewport width in points.
 * @param direction Layout direction (LTR or RTL).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_pop_gesture_init(
    struct cupertino_pop_gesture *gesture, float screen_width,
    enum ui_bidi_direction direction);

/**
 * @brief Handles touch-down event for edge-swipe back navigation.
 *
 * @param gesture Pointer to gesture recognizer.
 * @param touch_x Touch down X coordinate in points.
 * @param out_began Pointer to receive 1 if tracking began, 0 if outside edge.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_pop_gesture_touch_down(
    struct cupertino_pop_gesture *gesture, float touch_x, int *out_began);

/**
 * @brief Handles touch-move event, updating parallax translation and progress.
 *
 * @param gesture Pointer to gesture recognizer.
 * @param current_x Updated touch X coordinate.
 * @param out_progress Pointer to receive normalized progress [0.0, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_pop_gesture_touch_move(struct cupertino_pop_gesture *gesture,
                                 float current_x, float *out_progress);

/**
 * @brief Handles touch-up/release event, deciding whether to complete or cancel
 * pop.
 *
 * @param gesture Pointer to gesture recognizer.
 * @param velocity_x Horizontal release velocity in pt/s.
 * @param out_should_pop Pointer to receive 1 if view should be popped, 0 to
 * cancel.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_pop_gesture_touch_up(struct cupertino_pop_gesture *gesture,
                               float velocity_x, int *out_should_pop);

/**
 * @brief Calculates parallax offset for the underlying previous view.
 * In Apple HIG, the underlying view starts at -33% offset and moves to 0%.
 *
 * @param gesture Pointer to gesture recognizer.
 * @param out_underlying_offset_x Pointer to receive X offset for underlying
 * view.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_pop_gesture_get_underlying_offset(
    const struct cupertino_pop_gesture *gesture,
    float *out_underlying_offset_x);

/**
 * @brief Initializes an accessibility two-finger "Z" scrub gesture recognizer.
 *
 * @param scrub Pointer to scrub gesture structure.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_scrub_gesture_init(struct cupertino_scrub_gesture *scrub);

/**
 * @brief Handles multi-touch down event for two-finger scrub.
 *
 * @param scrub Pointer to scrub recognizer.
 * @param pointer_count Number of concurrent touch contacts.
 * @param x Touch X coordinate.
 * @param y Touch Y coordinate.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_scrub_gesture_touch_down(
    struct cupertino_scrub_gesture *scrub, int pointer_count, float x, float y);

/**
 * @brief Updates position during multi-touch drag, progressing the "Z" shape.
 *
 * @param scrub Pointer to scrub recognizer.
 * @param x Current touch X coordinate.
 * @param y Current touch Y coordinate.
 * @param out_recognized Pointer to receive 1 if full "Z" recognized, 0
 * otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_scrub_gesture_touch_move(struct cupertino_scrub_gesture *scrub,
                                   float x, float y, int *out_recognized);

/**
 * @brief Resets the scrub gesture state machine back to IDLE.
 *
 * @param scrub Pointer to scrub recognizer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_scrub_gesture_reset(struct cupertino_scrub_gesture *scrub);

/**
 * @brief Forwards raw hardware or synthesized pointer/touch events down to
 * Cupertino gestures.
 *
 * @param pop_gesture Interactive pop gesture recognizer (or NULL).
 * @param scrub_gesture Accessibility Z-scrub recognizer (or NULL).
 * @param event Input event from CDK / OS.
 * @param out_handled Pointer to receive 1 if event was consumed, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_gesture_forward_event(struct cupertino_pop_gesture *pop_gesture,
                                struct cupertino_scrub_gesture *scrub_gesture,
                                const struct ui_event *event, int *out_handled);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_GESTURE_H */
