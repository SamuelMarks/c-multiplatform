#ifndef CMP_PREDICTIVE_H
#define CMP_PREDICTIVE_H

/**
 * @file cmp_predictive.h
 * @brief Predictive back gesture handling for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_math.h"

/** @brief Predictive back edge is unspecified. */
#define CMP_PREDICTIVE_BACK_EDGE_UNKNOWN 0u
/** @brief Predictive back originates from the left edge. */
#define CMP_PREDICTIVE_BACK_EDGE_LEFT 1u
/** @brief Predictive back originates from the right edge. */
#define CMP_PREDICTIVE_BACK_EDGE_RIGHT 2u

/**
 * @brief Predictive back event payload.
 */
typedef struct CMPPredictiveBackEvent {
  CMPScalar progress;   /**< Normalized progress in the 0..1 range. */
  CMPScalar x;          /**< Current X position in window coordinates. */
  CMPScalar y;          /**< Current Y position in window coordinates. */
  CMPScalar velocity_x; /**< X velocity in pixels per second. */
  CMPScalar velocity_y; /**< Y velocity in pixels per second. */
  cmp_u32 edge;         /**< Back edge (CMP_PREDICTIVE_BACK_EDGE_*). */
  cmp_u32 time_ms;      /**< Timestamp in milliseconds. */
} CMPPredictiveBackEvent;

/**
 * @brief Predictive back callback signature.
 * @param ctx Callback context pointer.
 * @param event Predictive back event payload.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPPredictiveBackCallback)(
    void *ctx, const CMPPredictiveBackEvent *event);

/**
 * @brief Predictive back handler virtual table.
 */
typedef struct CMPPredictiveBackHandlerVTable {
  CMPPredictiveBackCallback on_start;    /**< Gesture started callback. */
  CMPPredictiveBackCallback on_progress; /**< Gesture progress callback. */
  CMPPredictiveBackCallback on_commit;   /**< Gesture committed callback. */
  CMPPredictiveBackCallback on_cancel;   /**< Gesture cancelled callback. */
} CMPPredictiveBackHandlerVTable;

/**
 * @brief Predictive back handler interface.
 */
typedef struct CMPPredictiveBackHandler {
  void *ctx;                                    /**< Handler context pointer. */
  const CMPPredictiveBackHandlerVTable *vtable; /**< Handler virtual table. */
} CMPPredictiveBackHandler;

/**
 * @brief Predictive back state snapshot.
 */
typedef struct CMPPredictiveBackState {
  CMPBool active;               /**< CMP_TRUE when a back gesture is active. */
  CMPPredictiveBackEvent event; /**< Last received event payload. */
} CMPPredictiveBackState;

/**
 * @brief Predictive back controller.
 */
typedef struct CMPPredictiveBack {
  CMPBool initialized;               /**< CMP_TRUE when initialized. */
  CMPBool active;                    /**< CMP_TRUE when a gesture is active. */
  CMPPredictiveBackHandler handler;  /**< Handler for gesture callbacks. */
  CMPPredictiveBackEvent last_event; /**< Most recent event payload. */
} CMPPredictiveBack;

/**
 * @brief Initialize a predictive back event with defaults.
 * @param event Event to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_predictive_back_event_init(CMPPredictiveBackEvent *event);

/**
 * @brief Initialize a predictive back handler interface.
 * @param handler Handler interface to initialize.
 * @param ctx Handler context pointer.
 * @param vtable Handler virtual table.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_predictive_back_handler_init(CMPPredictiveBackHandler *handler, void *ctx,
                                 const CMPPredictiveBackHandlerVTable *vtable);

/**
 * @brief Initialize a predictive back controller.
 * @param predictive Controller instance.
 * @param handler Handler interface (may be NULL to clear).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_predictive_back_init(
    CMPPredictiveBack *predictive, const CMPPredictiveBackHandler *handler);

/**
 * @brief Shut down a predictive back controller.
 * @param predictive Controller instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_predictive_back_shutdown(CMPPredictiveBack *predictive);

/**
 * @brief Update the predictive back handler.
 * @param predictive Controller instance.
 * @param handler Handler interface (may be NULL to clear).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_predictive_back_set_handler(
    CMPPredictiveBack *predictive, const CMPPredictiveBackHandler *handler);

/**
 * @brief Retrieve the predictive back handler.
 * @param predictive Controller instance.
 * @param out_handler Receives the handler interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_predictive_back_get_handler(
    const CMPPredictiveBack *predictive, CMPPredictiveBackHandler *out_handler);

/**
 * @brief Retrieve the predictive back controller state.
 * @param predictive Controller instance.
 * @param out_state Receives the state snapshot.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_predictive_back_get_state(
    const CMPPredictiveBack *predictive, CMPPredictiveBackState *out_state);

/**
 * @brief Notify the controller that a predictive back gesture started.
 * @param predictive Controller instance.
 * @param event Gesture payload.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_predictive_back_start(
    CMPPredictiveBack *predictive, const CMPPredictiveBackEvent *event);

/**
 * @brief Notify the controller of predictive back gesture progress.
 * @param predictive Controller instance.
 * @param event Gesture payload.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_predictive_back_progress(
    CMPPredictiveBack *predictive, const CMPPredictiveBackEvent *event);

/**
 * @brief Notify the controller that a predictive back gesture committed.
 * @param predictive Controller instance.
 * @param event Gesture payload.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_predictive_back_commit(
    CMPPredictiveBack *predictive, const CMPPredictiveBackEvent *event);

/**
 * @brief Notify the controller that a predictive back gesture cancelled.
 * @param predictive Controller instance.
 * @param event Gesture payload.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_predictive_back_cancel(
    CMPPredictiveBack *predictive, const CMPPredictiveBackEvent *event);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for predictive back event validation.
 * @param event Event payload to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_predictive_back_test_validate_event(const CMPPredictiveBackEvent *event);

/**
 * @brief Test wrapper for predictive back handler validation.
 * @param handler Handler interface to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_predictive_back_test_validate_handler(
    const CMPPredictiveBackHandler *handler);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_PREDICTIVE_H */
