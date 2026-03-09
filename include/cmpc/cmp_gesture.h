#ifndef CMP_GESTURE_H
#define CMP_GESTURE_H

/**
 * @file cmp_gesture.h
 * @brief Gesture recognition and dispatch for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_render.h"

/** @brief Default max tap duration in milliseconds. */
#define CMP_GESTURE_DEFAULT_TAP_MAX_MS 250u
/** @brief Default max tap movement in pixels. */
#define CMP_GESTURE_DEFAULT_TAP_MAX_DISTANCE 8.0f
/** @brief Default max double tap gap in milliseconds. */
#define CMP_GESTURE_DEFAULT_DOUBLE_TAP_MAX_MS 400u
/** @brief Default max double tap distance in pixels. */
#define CMP_GESTURE_DEFAULT_DOUBLE_TAP_MAX_DISTANCE 16.0f
/** @brief Default long press duration in milliseconds. */
#define CMP_GESTURE_DEFAULT_LONG_PRESS_MS 500u
/** @brief Default drag start threshold in pixels. */
#define CMP_GESTURE_DEFAULT_DRAG_START_DISTANCE 8.0f
/** @brief Default fling velocity threshold in pixels per second. */
#define CMP_GESTURE_DEFAULT_FLING_MIN_VELOCITY 1000.0f

/**
 * @brief Gesture recognizer configuration.
 */
typedef struct CMPGestureConfig {
  cmp_u32 tap_max_ms;         /**< Max tap duration in milliseconds. */
  CMPScalar tap_max_distance; /**< Max tap distance in pixels (>= 0). */
  cmp_u32 double_tap_max_ms;  /**< Max delay between taps in milliseconds. */
  CMPScalar double_tap_max_distance; /**< Max distance between taps in pixels
                                       (>= 0). */
  cmp_u32 long_press_ms;         /**< Long press duration in milliseconds. */
  CMPScalar drag_start_distance; /**< Drag start threshold in pixels (>= 0). */
  CMPScalar fling_min_velocity; /**< Minimum fling velocity in pixels per second
                                  (>= 0). */
} CMPGestureConfig;

/**
 * @brief Gesture dispatcher state.
 */
typedef struct CMPGestureDispatcher {
  CMPBool initialized;        /**< CMP_TRUE when initialized. */
  CMPGestureConfig config;    /**< Gesture configuration. */
  CMPWidget *active_widget;   /**< Active target widget (not owned). */
  CMPBool pointer_active;     /**< CMP_TRUE when tracking an active pointer. */
  cmp_i32 active_pointer;     /**< Active pointer identifier. */
  CMPBool drag_active;        /**< CMP_TRUE when drag gesture is active. */
  cmp_u32 down_time;          /**< Pointer down timestamp in milliseconds. */
  cmp_u32 last_time;          /**< Last pointer timestamp in milliseconds. */
  CMPScalar down_x;           /**< Pointer down X position. */
  CMPScalar down_y;           /**< Pointer down Y position. */
  CMPScalar last_x;           /**< Last pointer X position. */
  CMPScalar last_y;           /**< Last pointer Y position. */
  CMPScalar velocity_x;       /**< Last computed velocity along X. */
  CMPScalar velocity_y;       /**< Last computed velocity along Y. */
  CMPBool has_last_tap;       /**< CMP_TRUE when a previous tap is recorded. */
  cmp_u32 last_tap_time;      /**< Last tap timestamp in milliseconds. */
  CMPScalar last_tap_x;       /**< Last tap X position. */
  CMPScalar last_tap_y;       /**< Last tap Y position. */
  CMPWidget *last_tap_widget; /**< Last tap widget (not owned). */
} CMPGestureDispatcher;

/**
 * @brief Initialize a gesture configuration with defaults.
 * @param config Configuration to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_gesture_config_init(CMPGestureConfig *config);

/**
 * @brief Initialize a gesture dispatcher.
 * @param dispatcher Dispatcher instance.
 * @param config Configuration (NULL uses defaults).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_gesture_dispatcher_init(
    CMPGestureDispatcher *dispatcher, const CMPGestureConfig *config);

/**
 * @brief Shut down a gesture dispatcher.
 * @param dispatcher Dispatcher instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_gesture_dispatcher_shutdown(CMPGestureDispatcher *dispatcher);

/**
 * @brief Reset a gesture dispatcher to its idle state.
 * @param dispatcher Dispatcher instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_gesture_dispatcher_reset(CMPGestureDispatcher *dispatcher);

/**
 * @brief Update the gesture dispatcher configuration.
 * @param dispatcher Dispatcher instance.
 * @param config Configuration to apply.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_gesture_dispatcher_set_config(
    CMPGestureDispatcher *dispatcher, const CMPGestureConfig *config);

/**
 * @brief Retrieve the gesture dispatcher configuration.
 * @param dispatcher Dispatcher instance.
 * @param out_config Receives the configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_gesture_dispatcher_get_config(
    const CMPGestureDispatcher *dispatcher, CMPGestureConfig *out_config);

/**
 * @brief Dispatch an input event through the gesture recognizer.
 * @param dispatcher Dispatcher instance.
 * @param root Root render node for hit testing.
 * @param event Input event to process.
 * @param out_target Receives the active target widget (optional).
 * @param out_handled Receives CMP_TRUE if a gesture event was handled.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_gesture_dispatch(CMPGestureDispatcher *dispatcher,
                                          const CMPRenderNode *root,
                                          const CMPInputEvent *event,
                                          CMPWidget **out_target,
                                          CMPBool *out_handled);

#if defined(CMP_TESTING) && !defined(DOXYGEN_SHOULD_SKIP_THIS)
/**
 * @brief Test wrapper for gesture config validation.
 * @param config Configuration to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_gesture_test_validate_config(const CMPGestureConfig *config);

/**
 * @brief Test wrapper for gesture hit testing.
 * @param node Render node to test.
 * @param x X coordinate in pixels.
 * @param y Y coordinate in pixels.
 * @param out_widget Receives the hit widget (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_gesture_test_hit_test(const CMPRenderNode *node,
                                               CMPScalar x, CMPScalar y,
                                               CMPWidget **out_widget);
#endif

/**
 * @brief Helper for test coverage.
 * @param enable Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_gesture_test_set_contains_error(CMPBool enable);
/**
 * @brief Helper for test coverage.
 * @param enable Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_gesture_test_set_reset_fail(CMPBool enable);
/**
 * @brief Helper for test coverage.
 * @param enable Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_gesture_test_set_velocity_error(CMPBool enable);
/**
 * @brief Helper for test coverage.
 * @param enable Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_gesture_test_set_distance_error(CMPBool enable);
/**
 * @brief Helper for test coverage.
 * @param calls Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_gesture_test_set_distance_error_after(cmp_i32 calls);
/**
 * @brief Helper for test coverage.
 * @param enable Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_gesture_test_set_config_init_fail(CMPBool enable);
/**
 * @brief Helper for test coverage.
 * @param rect Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_gesture_test_validate_rect(const CMPRect *rect);
/**
 * @brief Helper for test coverage.
 * @param x0 Parameter for testing.
 * @param y0 Parameter for testing.
 * @param x1 Parameter for testing.
 * @param y1 Parameter for testing.
 * @param out_dist Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_gesture_test_distance_sq(CMPScalar x0, CMPScalar y0,
                                                  CMPScalar x1, CMPScalar y1,
                                                  CMPScalar *out_dist);
/**
 * @brief Helper for test coverage.
 * @param start Parameter for testing.
 * @param end Parameter for testing.
 * @param out_delta Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_gesture_test_time_delta(cmp_u32 start, cmp_u32 end,
                                                 cmp_u32 *out_delta);
/**
 * @brief Helper for test coverage.
 * @param x0 Parameter for testing.
 * @param y0 Parameter for testing.
 * @param x1 Parameter for testing.
 * @param y1 Parameter for testing.
 * @param dt_ms Parameter for testing.
 * @param out_vx Parameter for testing.
 * @param out_vy Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_gesture_test_velocity(CMPScalar x0, CMPScalar y0,
                                               CMPScalar x1, CMPScalar y1,
                                               cmp_u32 dt_ms, CMPScalar *out_vx,
                                               CMPScalar *out_vy);
/**
 * @brief Helper for test coverage.
 * @param widget Parameter for testing.
 * @param event Parameter for testing.
 * @param out_handled Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_gesture_test_dispatch_to_widget(
    CMPWidget *widget, const CMPInputEvent *event, CMPBool *out_handled);
/**
 * @brief Helper for test coverage.
 * @param widget Parameter for testing.
 * @param source Parameter for testing.
 * @param type Parameter for testing.
 * @param gesture Parameter for testing.
 * @param out_handled Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_gesture_test_emit(CMPWidget *widget,
                                           const CMPInputEvent *source,
                                           cmp_u32 type,
                                           const CMPGestureEvent *gesture,
                                           CMPBool *out_handled);
CMP_API int
    CMP_CALL
    /**
     * @brief Helper for test coverage.
     * @param dispatcher Parameter for testing.
     * @return Test result.
     */
    cmp_gesture_test_reset_pointer(CMPGestureDispatcher *dispatcher);
CMP_API int
    CMP_CALL
    /**
     * @brief Helper for test coverage.
     * @param dispatcher Parameter for testing.
     * @return Test result.
     */
    cmp_gesture_test_clear_state(CMPGestureDispatcher *dispatcher);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_GESTURE_H */
