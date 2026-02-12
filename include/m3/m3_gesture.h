#ifndef M3_GESTURE_H
#define M3_GESTURE_H

/**
 * @file m3_gesture.h
 * @brief Gesture recognition and dispatch for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_render.h"

/** @brief Default max tap duration in milliseconds. */
#define M3_GESTURE_DEFAULT_TAP_MAX_MS 250u
/** @brief Default max tap movement in pixels. */
#define M3_GESTURE_DEFAULT_TAP_MAX_DISTANCE 8.0f
/** @brief Default max double tap gap in milliseconds. */
#define M3_GESTURE_DEFAULT_DOUBLE_TAP_MAX_MS 400u
/** @brief Default max double tap distance in pixels. */
#define M3_GESTURE_DEFAULT_DOUBLE_TAP_MAX_DISTANCE 16.0f
/** @brief Default long press duration in milliseconds. */
#define M3_GESTURE_DEFAULT_LONG_PRESS_MS 500u
/** @brief Default drag start threshold in pixels. */
#define M3_GESTURE_DEFAULT_DRAG_START_DISTANCE 8.0f
/** @brief Default fling velocity threshold in pixels per second. */
#define M3_GESTURE_DEFAULT_FLING_MIN_VELOCITY 1000.0f

/**
 * @brief Gesture recognizer configuration.
 */
typedef struct M3GestureConfig {
  m3_u32 tap_max_ms;         /**< Max tap duration in milliseconds. */
  M3Scalar tap_max_distance; /**< Max tap distance in pixels (>= 0). */
  m3_u32 double_tap_max_ms;  /**< Max delay between taps in milliseconds. */
  M3Scalar double_tap_max_distance; /**< Max distance between taps in pixels (>=
                                       0). */
  m3_u32 long_press_ms;             /**< Long press duration in milliseconds. */
  M3Scalar drag_start_distance; /**< Drag start threshold in pixels (>= 0). */
  M3Scalar fling_min_velocity;  /**< Minimum fling velocity in pixels per second
                                   (>= 0). */
} M3GestureConfig;

/**
 * @brief Gesture dispatcher state.
 */
typedef struct M3GestureDispatcher {
  M3Bool initialized;        /**< M3_TRUE when initialized. */
  M3GestureConfig config;    /**< Gesture configuration. */
  M3Widget *active_widget;   /**< Active target widget (not owned). */
  M3Bool pointer_active;     /**< M3_TRUE when tracking an active pointer. */
  m3_i32 active_pointer;     /**< Active pointer identifier. */
  M3Bool drag_active;        /**< M3_TRUE when drag gesture is active. */
  m3_u32 down_time;          /**< Pointer down timestamp in milliseconds. */
  m3_u32 last_time;          /**< Last pointer timestamp in milliseconds. */
  M3Scalar down_x;           /**< Pointer down X position. */
  M3Scalar down_y;           /**< Pointer down Y position. */
  M3Scalar last_x;           /**< Last pointer X position. */
  M3Scalar last_y;           /**< Last pointer Y position. */
  M3Scalar velocity_x;       /**< Last computed velocity along X. */
  M3Scalar velocity_y;       /**< Last computed velocity along Y. */
  M3Bool has_last_tap;       /**< M3_TRUE when a previous tap is recorded. */
  m3_u32 last_tap_time;      /**< Last tap timestamp in milliseconds. */
  M3Scalar last_tap_x;       /**< Last tap X position. */
  M3Scalar last_tap_y;       /**< Last tap Y position. */
  M3Widget *last_tap_widget; /**< Last tap widget (not owned). */
} M3GestureDispatcher;

/**
 * @brief Initialize a gesture configuration with defaults.
 * @param config Configuration to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_gesture_config_init(M3GestureConfig *config);

/**
 * @brief Initialize a gesture dispatcher.
 * @param dispatcher Dispatcher instance.
 * @param config Configuration (NULL uses defaults).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_gesture_dispatcher_init(M3GestureDispatcher *dispatcher,
                                              const M3GestureConfig *config);

/**
 * @brief Shut down a gesture dispatcher.
 * @param dispatcher Dispatcher instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_gesture_dispatcher_shutdown(M3GestureDispatcher *dispatcher);

/**
 * @brief Reset a gesture dispatcher to its idle state.
 * @param dispatcher Dispatcher instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_gesture_dispatcher_reset(M3GestureDispatcher *dispatcher);

/**
 * @brief Update the gesture dispatcher configuration.
 * @param dispatcher Dispatcher instance.
 * @param config Configuration to apply.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_gesture_dispatcher_set_config(
    M3GestureDispatcher *dispatcher, const M3GestureConfig *config);

/**
 * @brief Retrieve the gesture dispatcher configuration.
 * @param dispatcher Dispatcher instance.
 * @param out_config Receives the configuration.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_gesture_dispatcher_get_config(
    const M3GestureDispatcher *dispatcher, M3GestureConfig *out_config);

/**
 * @brief Dispatch an input event through the gesture recognizer.
 * @param dispatcher Dispatcher instance.
 * @param root Root render node for hit testing.
 * @param event Input event to process.
 * @param out_target Receives the active target widget (optional).
 * @param out_handled Receives M3_TRUE if a gesture event was handled.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_gesture_dispatch(M3GestureDispatcher *dispatcher,
                                       const M3RenderNode *root,
                                       const M3InputEvent *event,
                                       M3Widget **out_target,
                                       M3Bool *out_handled);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for gesture config validation.
 * @param config Configuration to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_gesture_test_validate_config(const M3GestureConfig *config);

/**
 * @brief Test wrapper for gesture hit testing.
 * @param node Render node to test.
 * @param x X coordinate in pixels.
 * @param y Y coordinate in pixels.
 * @param out_widget Receives the hit widget (may be NULL).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_gesture_test_hit_test(const M3RenderNode *node,
                                            M3Scalar x, M3Scalar y,
                                            M3Widget **out_widget);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_GESTURE_H */
