#ifndef M3_PREDICTIVE_H
#define M3_PREDICTIVE_H

/**
 * @file m3_predictive.h
 * @brief Predictive back gesture handling for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_math.h"

/** @brief Predictive back edge is unspecified. */
#define M3_PREDICTIVE_BACK_EDGE_UNKNOWN 0u
/** @brief Predictive back originates from the left edge. */
#define M3_PREDICTIVE_BACK_EDGE_LEFT 1u
/** @brief Predictive back originates from the right edge. */
#define M3_PREDICTIVE_BACK_EDGE_RIGHT 2u

/**
 * @brief Predictive back event payload.
 */
typedef struct M3PredictiveBackEvent {
  M3Scalar progress;   /**< Normalized progress in the 0..1 range. */
  M3Scalar x;          /**< Current X position in window coordinates. */
  M3Scalar y;          /**< Current Y position in window coordinates. */
  M3Scalar velocity_x; /**< X velocity in pixels per second. */
  M3Scalar velocity_y; /**< Y velocity in pixels per second. */
  m3_u32 edge;         /**< Back edge (M3_PREDICTIVE_BACK_EDGE_*). */
  m3_u32 time_ms;      /**< Timestamp in milliseconds. */
} M3PredictiveBackEvent;

/**
 * @brief Predictive back callback signature.
 * @param ctx Callback context pointer.
 * @param event Predictive back event payload.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3PredictiveBackCallback)(
    void *ctx, const M3PredictiveBackEvent *event);

/**
 * @brief Predictive back handler virtual table.
 */
typedef struct M3PredictiveBackHandlerVTable {
  M3PredictiveBackCallback on_start;    /**< Gesture started callback. */
  M3PredictiveBackCallback on_progress; /**< Gesture progress callback. */
  M3PredictiveBackCallback on_commit;   /**< Gesture committed callback. */
  M3PredictiveBackCallback on_cancel;   /**< Gesture cancelled callback. */
} M3PredictiveBackHandlerVTable;

/**
 * @brief Predictive back handler interface.
 */
typedef struct M3PredictiveBackHandler {
  void *ctx;                                   /**< Handler context pointer. */
  const M3PredictiveBackHandlerVTable *vtable; /**< Handler virtual table. */
} M3PredictiveBackHandler;

/**
 * @brief Predictive back state snapshot.
 */
typedef struct M3PredictiveBackState {
  M3Bool active;               /**< M3_TRUE when a back gesture is active. */
  M3PredictiveBackEvent event; /**< Last received event payload. */
} M3PredictiveBackState;

/**
 * @brief Predictive back controller.
 */
typedef struct M3PredictiveBack {
  M3Bool initialized;               /**< M3_TRUE when initialized. */
  M3Bool active;                    /**< M3_TRUE when a gesture is active. */
  M3PredictiveBackHandler handler;  /**< Handler for gesture callbacks. */
  M3PredictiveBackEvent last_event; /**< Most recent event payload. */
} M3PredictiveBack;

/**
 * @brief Initialize a predictive back event with defaults.
 * @param event Event to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_predictive_back_event_init(M3PredictiveBackEvent *event);

/**
 * @brief Initialize a predictive back handler interface.
 * @param handler Handler interface to initialize.
 * @param ctx Handler context pointer.
 * @param vtable Handler virtual table.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_predictive_back_handler_init(M3PredictiveBackHandler *handler, void *ctx,
                                const M3PredictiveBackHandlerVTable *vtable);

/**
 * @brief Initialize a predictive back controller.
 * @param predictive Controller instance.
 * @param handler Handler interface (may be NULL to clear).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_predictive_back_init(
    M3PredictiveBack *predictive, const M3PredictiveBackHandler *handler);

/**
 * @brief Shut down a predictive back controller.
 * @param predictive Controller instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_predictive_back_shutdown(M3PredictiveBack *predictive);

/**
 * @brief Update the predictive back handler.
 * @param predictive Controller instance.
 * @param handler Handler interface (may be NULL to clear).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_predictive_back_set_handler(
    M3PredictiveBack *predictive, const M3PredictiveBackHandler *handler);

/**
 * @brief Retrieve the predictive back handler.
 * @param predictive Controller instance.
 * @param out_handler Receives the handler interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_predictive_back_get_handler(
    const M3PredictiveBack *predictive, M3PredictiveBackHandler *out_handler);

/**
 * @brief Retrieve the predictive back controller state.
 * @param predictive Controller instance.
 * @param out_state Receives the state snapshot.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_predictive_back_get_state(
    const M3PredictiveBack *predictive, M3PredictiveBackState *out_state);

/**
 * @brief Notify the controller that a predictive back gesture started.
 * @param predictive Controller instance.
 * @param event Gesture payload.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_predictive_back_start(M3PredictiveBack *predictive,
                                            const M3PredictiveBackEvent *event);

/**
 * @brief Notify the controller of predictive back gesture progress.
 * @param predictive Controller instance.
 * @param event Gesture payload.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_predictive_back_progress(
    M3PredictiveBack *predictive, const M3PredictiveBackEvent *event);

/**
 * @brief Notify the controller that a predictive back gesture committed.
 * @param predictive Controller instance.
 * @param event Gesture payload.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_predictive_back_commit(
    M3PredictiveBack *predictive, const M3PredictiveBackEvent *event);

/**
 * @brief Notify the controller that a predictive back gesture cancelled.
 * @param predictive Controller instance.
 * @param event Gesture payload.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_predictive_back_cancel(
    M3PredictiveBack *predictive, const M3PredictiveBackEvent *event);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for predictive back event validation.
 * @param event Event payload to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_predictive_back_test_validate_event(const M3PredictiveBackEvent *event);

/**
 * @brief Test wrapper for predictive back handler validation.
 * @param handler Handler interface to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_predictive_back_test_validate_handler(
    const M3PredictiveBackHandler *handler);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_PREDICTIVE_H */
