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
  UI_GESTURE_NONE = 0,
  UI_GESTURE_TAP,
  UI_GESTURE_LONG_PRESS,
  UI_GESTURE_PAN,
  UI_GESTURE_SWIPE,
  UI_GESTURE_PINCH,
  UI_GESTURE_ROTATION
};

/**
 * @brief States of a gesture's lifecycle.
 */
enum ui_gesture_state {
  UI_GESTURE_STATE_POSSIBLE = 0,
  UI_GESTURE_STATE_BEGAN,
  UI_GESTURE_STATE_CHANGED,
  UI_GESTURE_STATE_ENDED,
  UI_GESTURE_STATE_CANCELLED,
  UI_GESTURE_STATE_FAILED
};

/**
 * @brief Event payload produced when a gesture is recognized.
 */
struct ui_gesture_event {
  enum ui_gesture_type type;
  enum ui_gesture_state state;
  int x;
  int y;
  float delta_x;
  float delta_y;
  float velocity_x;
  float velocity_y;
  float scale;
  float rotation;
};

/**
 * @brief Opaque gesture recognizer context.
 */
struct ui_gesture_recognizer;

/**
 * @brief Creates a new gesture recognizer.
 * @param out_recognizer Pointer to receive the allocated recognizer.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_gesture_recognizer_create(struct ui_gesture_recognizer **out_recognizer);

/**
 * @brief Destroys a gesture recognizer.
 * @param recognizer The recognizer to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_gesture_recognizer_destroy(struct ui_gesture_recognizer *recognizer);

/**
 * @brief Processes a raw input event and updates the recognizer's state.
 * @param recognizer The recognizer.
 * @param event The raw input event (mouse, touch, pen).
 * @param timestamp_ms Current time in milliseconds.
 * @param out_gesture_event Pointer to receive the generated gesture event, if
 * any.
 * @return UI_ERROR_NONE on success. out_gesture_event->type will be
 * UI_GESTURE_NONE if no gesture is emitted.
 */
ui_error_t ui_gesture_recognizer_process_event(
    struct ui_gesture_recognizer *recognizer, const struct ui_event *event,
    double timestamp_ms, struct ui_gesture_event *out_gesture_event);

/**
 * @brief Signals a timer tick to the gesture recognizer for evaluating long
 * presses.
 * @param recognizer The recognizer.
 * @param timestamp_ms Current time in milliseconds.
 * @param out_gesture_event Pointer to receive the generated gesture event, if
 * any.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_gesture_recognizer_update(struct ui_gesture_recognizer *recognizer,
                             double timestamp_ms,
                             struct ui_gesture_event *out_gesture_event);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_GESTURE_H */
