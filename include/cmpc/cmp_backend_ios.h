#ifndef CMP_BACKEND_IOS_H
#define CMP_BACKEND_IOS_H

/**
 * @file cmp_backend_ios.h
 * @brief iOS backend for LibCMPC (UIKit/CoreGraphics).
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ws.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_predictive.h"

/** @brief Opaque iOS backend instance. */
typedef struct CMPIOSBackend CMPIOSBackend;

/**
 * @brief Configuration for the iOS backend.
 */
typedef struct CMPIOSBackendConfig {
  const CMPAllocator
      *allocator; /**< Allocator for backend memory (NULL uses default). */
  cmp_usize handle_capacity; /**< Maximum number of handles to manage. */
  cmp_usize clipboard_limit; /**< Maximum clipboard byte length accepted. */
  CMPBool enable_logging;    /**< Enable logging via cmp_log_write. */
  CMPBool inline_tasks;      /**< Execute posted tasks inline when CMP_TRUE. */
  CMPPredictiveBack
      *predictive_back; /**< Predictive back controller (optional). */
} CMPIOSBackendConfig;

/**
 * @brief Report whether iOS support is available in this build.
 * @param out_available Receives CMP_TRUE if iOS is available, CMP_FALSE
 * otherwise.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ios_backend_is_available(CMPBool *out_available);

/**
 * @brief Initialize an iOS backend configuration with defaults.
 * @param config Config to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ios_backend_config_init(CMPIOSBackendConfig *config);

/**
 * @brief Create an iOS backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ios_backend_create(const CMPIOSBackendConfig *config,
                                            CMPIOSBackend **out_backend);

/**
 * @brief Destroy an iOS backend instance.
 * @param backend Backend instance to destroy.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ios_backend_destroy(CMPIOSBackend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ios_backend_get_ws(CMPIOSBackend *backend,
                                            CMPWS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ios_backend_get_gfx(CMPIOSBackend *backend,
                                             CMPGfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ios_backend_get_env(CMPIOSBackend *backend,
                                             CMPEnv *out_env);

/**
 * @brief Assign the predictive back controller for the backend.
 * @param backend Backend instance.
 * @param predictive Predictive back controller (may be NULL to clear).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ios_backend_set_predictive_back(
    CMPIOSBackend *backend, CMPPredictiveBack *predictive);

/**
 * @brief Retrieve the predictive back controller for the backend.
 * @param backend Backend instance.
 * @param out_predictive Receives the predictive back controller pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ios_backend_get_predictive_back(
    CMPIOSBackend *backend, CMPPredictiveBack **out_predictive);

/**
 * @brief Notify the backend of a predictive back start event.
 * @param backend Backend instance.
 * @param event Predictive back event payload.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ios_backend_predictive_back_start(
    CMPIOSBackend *backend, const CMPPredictiveBackEvent *event);

/**
 * @brief Notify the backend of a predictive back progress event.
 * @param backend Backend instance.
 * @param event Predictive back event payload.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ios_backend_predictive_back_progress(
    CMPIOSBackend *backend, const CMPPredictiveBackEvent *event);

/**
 * @brief Notify the backend of a predictive back commit event.
 * @param backend Backend instance.
 * @param event Predictive back event payload.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ios_backend_predictive_back_commit(
    CMPIOSBackend *backend, const CMPPredictiveBackEvent *event);

/**
 * @brief Notify the backend of a predictive back cancel event.
 * @param backend Backend instance.
 * @param event Predictive back event payload.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ios_backend_predictive_back_cancel(
    CMPIOSBackend *backend, const CMPPredictiveBackEvent *event);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for iOS backend config validation.
 * @param config Backend configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_ios_backend_test_validate_config(const CMPIOSBackendConfig *config);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_BACKEND_IOS_H */
