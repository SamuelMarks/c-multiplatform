#ifndef M3_BACKEND_ANDROID_H
#define M3_BACKEND_ANDROID_H

/**
 * @file m3_backend_android.h
 * @brief Android backend for LibM3C.
 *
 * @note The Android backend stores JNI/NDK handles provided by the host
 *       application. When using JNI, pass global references for any jobjects.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_env.h"
#include "m3_api_gfx.h"
#include "m3_api_ws.h"
#include "m3_core.h"

/** @brief Opaque Android backend instance. */
typedef struct M3AndroidBackend M3AndroidBackend;

/**
 * @brief Configuration for the Android backend.
 */
typedef struct M3AndroidBackendConfig {
  const M3Allocator
      *allocator; /**< Allocator for backend memory (NULL uses default). */
  m3_usize handle_capacity; /**< Maximum number of handles to manage. */
  m3_usize clipboard_limit; /**< Maximum clipboard byte length accepted. */
  M3Bool enable_logging;    /**< Enable logging via m3_log_write. */
  M3Bool inline_tasks;      /**< Execute posted tasks inline when M3_TRUE. */
  void *java_vm;            /**< JavaVM* pointer (Android only). */
  void *jni_env;            /**< JNIEnv* pointer (Android only). */
  void *activity;      /**< jobject Activity global reference (Android only). */
  void *asset_manager; /**< AAssetManager* pointer (Android only). */
  void *native_window; /**< ANativeWindow* pointer (Android only). */
  void *input_queue;   /**< AInputQueue* pointer (Android only). */
  void *looper;        /**< ALooper* pointer (Android only). */
} M3AndroidBackendConfig;

/**
 * @brief Report whether Android support is available in this build.
 * @param out_available Receives M3_TRUE if Android is available, M3_FALSE
 * otherwise.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_android_backend_is_available(M3Bool *out_available);

/**
 * @brief Initialize an Android backend configuration with defaults.
 * @param config Config to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_android_backend_config_init(M3AndroidBackendConfig *config);

/**
 * @brief Create an Android backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_android_backend_create(
    const M3AndroidBackendConfig *config, M3AndroidBackend **out_backend);

/**
 * @brief Destroy an Android backend instance.
 * @param backend Backend instance to destroy.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_android_backend_destroy(M3AndroidBackend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_android_backend_get_ws(M3AndroidBackend *backend,
                                             M3WS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_android_backend_get_gfx(M3AndroidBackend *backend,
                                              M3Gfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_android_backend_get_env(M3AndroidBackend *backend,
                                              M3Env *out_env);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for Android backend config validation.
 * @param config Backend configuration.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_android_backend_test_validate_config(const M3AndroidBackendConfig *config);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_BACKEND_ANDROID_H */
