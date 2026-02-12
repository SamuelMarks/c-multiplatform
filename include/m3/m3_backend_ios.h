#ifndef M3_BACKEND_IOS_H
#define M3_BACKEND_IOS_H

/**
 * @file m3_backend_ios.h
 * @brief iOS backend for LibM3C (UIKit/CoreGraphics).
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_env.h"
#include "m3_api_gfx.h"
#include "m3_api_ws.h"
#include "m3_core.h"
#include "m3_predictive.h"

/** @brief Opaque iOS backend instance. */
typedef struct M3IOSBackend M3IOSBackend;

/**
 * @brief Configuration for the iOS backend.
 */
typedef struct M3IOSBackendConfig {
  const M3Allocator
      *allocator; /**< Allocator for backend memory (NULL uses default). */
  m3_usize handle_capacity; /**< Maximum number of handles to manage. */
  m3_usize clipboard_limit; /**< Maximum clipboard byte length accepted. */
  M3Bool enable_logging;    /**< Enable logging via m3_log_write. */
  M3Bool inline_tasks;      /**< Execute posted tasks inline when M3_TRUE. */
  M3PredictiveBack
      *predictive_back; /**< Predictive back controller (optional). */
} M3IOSBackendConfig;

/**
 * @brief Report whether iOS support is available in this build.
 * @param out_available Receives M3_TRUE if iOS is available, M3_FALSE
 * otherwise.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ios_backend_is_available(M3Bool *out_available);

/**
 * @brief Initialize an iOS backend configuration with defaults.
 * @param config Config to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ios_backend_config_init(M3IOSBackendConfig *config);

/**
 * @brief Create an iOS backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ios_backend_create(const M3IOSBackendConfig *config,
                                         M3IOSBackend **out_backend);

/**
 * @brief Destroy an iOS backend instance.
 * @param backend Backend instance to destroy.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ios_backend_destroy(M3IOSBackend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ios_backend_get_ws(M3IOSBackend *backend, M3WS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ios_backend_get_gfx(M3IOSBackend *backend,
                                          M3Gfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ios_backend_get_env(M3IOSBackend *backend,
                                          M3Env *out_env);

/**
 * @brief Assign the predictive back controller for the backend.
 * @param backend Backend instance.
 * @param predictive Predictive back controller (may be NULL to clear).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ios_backend_set_predictive_back(
    M3IOSBackend *backend, M3PredictiveBack *predictive);

/**
 * @brief Retrieve the predictive back controller for the backend.
 * @param backend Backend instance.
 * @param out_predictive Receives the predictive back controller pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ios_backend_get_predictive_back(
    M3IOSBackend *backend, M3PredictiveBack **out_predictive);

/**
 * @brief Notify the backend of a predictive back start event.
 * @param backend Backend instance.
 * @param event Predictive back event payload.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ios_backend_predictive_back_start(
    M3IOSBackend *backend, const M3PredictiveBackEvent *event);

/**
 * @brief Notify the backend of a predictive back progress event.
 * @param backend Backend instance.
 * @param event Predictive back event payload.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ios_backend_predictive_back_progress(
    M3IOSBackend *backend, const M3PredictiveBackEvent *event);

/**
 * @brief Notify the backend of a predictive back commit event.
 * @param backend Backend instance.
 * @param event Predictive back event payload.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ios_backend_predictive_back_commit(
    M3IOSBackend *backend, const M3PredictiveBackEvent *event);

/**
 * @brief Notify the backend of a predictive back cancel event.
 * @param backend Backend instance.
 * @param event Predictive back event payload.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ios_backend_predictive_back_cancel(
    M3IOSBackend *backend, const M3PredictiveBackEvent *event);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for iOS backend config validation.
 * @param config Backend configuration.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_ios_backend_test_validate_config(const M3IOSBackendConfig *config);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_BACKEND_IOS_H */
