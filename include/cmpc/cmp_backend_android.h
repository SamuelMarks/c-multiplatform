#ifndef CMP_BACKEND_ANDROID_H
#define CMP_BACKEND_ANDROID_H

/**
 * @file cmp_backend_android.h
 * @brief Android backend for LibCMPC.
 *
 * @note The Android backend stores JNI/NDK handles provided by the host
 *       application. When using JNI, pass global references for any jobjects.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ws.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_predictive.h"

/** @brief Opaque Android backend instance. */
typedef struct CMPAndroidBackend CMPAndroidBackend;

/**
 * @brief Configuration for the Android backend.
 */
typedef struct CMPAndroidBackendConfig {
  const CMPAllocator
      *allocator; /**< Allocator for backend memory (NULL uses default). */
  cmp_usize handle_capacity; /**< Maximum number of handles to manage. */
  cmp_usize clipboard_limit; /**< Maximum clipboard byte length accepted. */
  CMPBool enable_logging;    /**< Enable logging via cmp_log_write. */
  CMPBool inline_tasks;      /**< Execute posted tasks inline when CMP_TRUE. */
  CMPPredictiveBack
      *predictive_back; /**< Predictive back controller (optional). */
  void *java_vm;        /**< JavaVM* pointer (Android only). */
  void *jni_env;        /**< JNIEnv* pointer (Android only). */
  void *activity;      /**< jobject Activity global reference (Android only). */
  void *asset_manager; /**< AAssetManager* pointer (Android only). */
  void *native_window; /**< ANativeWindow* pointer (Android only). */
  void *input_queue;   /**< AInputQueue* pointer (Android only). */
  void *looper;        /**< ALooper* pointer (Android only). */
} CMPAndroidBackendConfig;

/**
 * @brief Report whether Android support is available in this build.
 * @param out_available Receives CMP_TRUE if Android is available, CMP_FALSE
 * otherwise.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_android_backend_is_available(CMPBool *out_available);

/**
 * @brief Initialize an Android backend configuration with defaults.
 * @param config Config to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_android_backend_config_init(CMPAndroidBackendConfig *config);

/**
 * @brief Create an Android backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_android_backend_create(
    const CMPAndroidBackendConfig *config, CMPAndroidBackend **out_backend);

/**
 * @brief Destroy an Android backend instance.
 * @param backend Backend instance to destroy.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_android_backend_destroy(CMPAndroidBackend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_android_backend_get_ws(CMPAndroidBackend *backend,
                                                CMPWS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_android_backend_get_gfx(CMPAndroidBackend *backend,
                                                 CMPGfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_android_backend_get_env(CMPAndroidBackend *backend,
                                                 CMPEnv *out_env);

/**
 * @brief Assign the predictive back controller for the backend.
 * @param backend Backend instance.
 * @param predictive Predictive back controller (may be NULL to clear).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_android_backend_set_predictive_back(
    CMPAndroidBackend *backend, CMPPredictiveBack *predictive);

/**
 * @brief Retrieve the predictive back controller for the backend.
 * @param backend Backend instance.
 * @param out_predictive Receives the predictive back controller pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_android_backend_get_predictive_back(
    CMPAndroidBackend *backend, CMPPredictiveBack **out_predictive);

/**
 * @brief Notify the backend of a predictive back start event.
 * @param backend Backend instance.
 * @param event Predictive back event payload.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_android_backend_predictive_back_start(
    CMPAndroidBackend *backend, const CMPPredictiveBackEvent *event);

/**
 * @brief Notify the backend of a predictive back progress event.
 * @param backend Backend instance.
 * @param event Predictive back event payload.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_android_backend_predictive_back_progress(
    CMPAndroidBackend *backend, const CMPPredictiveBackEvent *event);

/**
 * @brief Notify the backend of a predictive back commit event.
 * @param backend Backend instance.
 * @param event Predictive back event payload.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_android_backend_predictive_back_commit(
    CMPAndroidBackend *backend, const CMPPredictiveBackEvent *event);

/**
 * @brief Notify the backend of a predictive back cancel event.
 * @param backend Backend instance.
 * @param event Predictive back event payload.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_android_backend_predictive_back_cancel(
    CMPAndroidBackend *backend, const CMPPredictiveBackEvent *event);

#ifdef CMP_TESTING
/**
 * @brief Force cmp_android_backend_config_init to fail for coverage.
 * @param fail Set to CMP_TRUE to force the next init to fail.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_android_backend_test_set_config_init_fail(CMPBool fail);

/**
 * @brief Test wrapper for Android backend config validation.
 * @param config Backend configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_android_backend_test_validate_config(const CMPAndroidBackendConfig *config);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_BACKEND_ANDROID_H */
