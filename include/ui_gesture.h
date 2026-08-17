/**
 * @file ui_gesture.h
 * @brief Touch and mouse gesture recognition.
 *
 * This header provides structures and functions to recognize common gestures
 * like taps, pans, swipes, and pinches from raw input events.
 */

#ifndef UI_GESTURE_H
#define UI_GESTURE_H

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Types of recognized gestures.
 */
enum ui_gesture_type {
  UI_GESTURE_NONE = 0,   /**< No gesture. */
  UI_GESTURE_TAP,        /**< Tap gesture. */
  UI_GESTURE_LONG_PRESS, /**< Long press gesture. */
  UI_GESTURE_PAN,        /**< Pan/drag gesture. */
  UI_GESTURE_SWIPE,      /**< Swipe gesture. */
  UI_GESTURE_PINCH,      /**< Pinch-to-zoom gesture. */
  UI_GESTURE_ROTATION    /**< Two-finger rotation gesture. */
};

/**
 * @brief States of a gesture's lifecycle.
 */
enum ui_gesture_state {
  UI_GESTURE_STATE_POSSIBLE = 0, /**< Gesture is possible but not yet active. */
  UI_GESTURE_STATE_BEGAN,        /**< Gesture has started. */
  UI_GESTURE_STATE_CHANGED,      /**< Gesture is active and updating. */
  UI_GESTURE_STATE_ENDED,        /**< Gesture successfully completed. */
  UI_GESTURE_STATE_CANCELLED,    /**< Gesture was cancelled externally. */
  UI_GESTURE_STATE_FAILED        /**< Gesture conditions were not met. */
};

/**
 * @brief Event payload produced when a gesture is recognized.
 */
struct ui_gesture_event {
  enum ui_gesture_type type; /**< The type of gesture recognized. */
  enum ui_gesture_state
      state;        /**< The current lifecycle state of the gesture. */
  int x;            /**< Focal point X coordinate. */
  int y;            /**< Focal point Y coordinate. */
  float delta_x;    /**< X movement delta (for pan/swipe). */
  float delta_y;    /**< Y movement delta (for pan/swipe). */
  float velocity_x; /**< X velocity in pixels per second. */
  float velocity_y; /**< Y velocity in pixels per second. */
  float scale;      /**< Scale factor (for pinch). */
  float rotation;   /**< Rotation angle (for rotation). */
};

/**
 * @brief Opaque gesture recognizer context.
 */
struct ui_gesture_recognizer;

/**
 * @brief Creates a new gesture recognizer.
 *
 * @param out_recognizer Pointer to receive the allocated recognizer structure.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t
ui_gesture_recognizer_create(struct ui_gesture_recognizer **out_recognizer);

/**
 * @brief Destroys a gesture recognizer.
 *
 * @param recognizer Pointer to the recognizer to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t
ui_gesture_recognizer_destroy(struct ui_gesture_recognizer *recognizer);

/**
 * @brief Processes a raw input event and updates the recognizer's state.
 *
 * @param recognizer Pointer to the recognizer.
 * @param event Pointer to the raw input event (mouse, touch, pen).
 * @param timestamp_ms Current time in milliseconds.
 * @param out_gesture_event Pointer to receive the generated gesture event, if
 * any.
 * @return `UI_ERROR_NONE` on success. `out_gesture_event->type` will be
 * `UI_GESTURE_NONE` if no gesture is emitted.
 */
ui_error_t ui_gesture_recognizer_process_event(
    struct ui_gesture_recognizer *recognizer, const struct ui_event *event,
    double timestamp_ms, struct ui_gesture_event *out_gesture_event);

/**
 * @brief Signals a timer tick to the gesture recognizer for evaluating
 * time-based gestures like long presses.
 *
 * @param recognizer Pointer to the recognizer.
 * @param timestamp_ms Current time in milliseconds.
 * @param out_gesture_event Pointer to receive the generated gesture event, if
 * any.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t
ui_gesture_recognizer_update(struct ui_gesture_recognizer *recognizer,
                             double timestamp_ms,
                             struct ui_gesture_event *out_gesture_event);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_GESTURE_H */
