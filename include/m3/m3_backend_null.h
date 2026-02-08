#ifndef M3_BACKEND_NULL_H
#define M3_BACKEND_NULL_H

/**
 * @file m3_backend_null.h
 * @brief Null backend implementation for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_env.h"
#include "m3_api_gfx.h"
#include "m3_api_ws.h"
#include "m3_core.h"

/** @brief Opaque null backend instance. */
typedef struct M3NullBackend M3NullBackend;

/**
 * @brief Configuration for the null backend.
 */
typedef struct M3NullBackendConfig {
    const M3Allocator *allocator; /**< Allocator for backend memory (NULL uses default). */
    m3_usize handle_capacity; /**< Maximum number of handles to manage. */
    m3_usize clipboard_limit; /**< Maximum clipboard byte length accepted. */
    M3Bool enable_logging; /**< Enable logging via m3_log_write. */
    M3Bool inline_tasks; /**< Execute posted tasks inline when M3_TRUE. */
} M3NullBackendConfig;

/**
 * @brief Initialize a null backend configuration with defaults.
 * @param config Config to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_null_backend_config_init(M3NullBackendConfig *config);

/**
 * @brief Create a null backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_null_backend_create(const M3NullBackendConfig *config, M3NullBackend **out_backend);

/**
 * @brief Destroy a null backend instance.
 * @param backend Backend instance to destroy.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_null_backend_destroy(M3NullBackend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_null_backend_get_ws(M3NullBackend *backend, M3WS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_null_backend_get_gfx(M3NullBackend *backend, M3Gfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_null_backend_get_env(M3NullBackend *backend, M3Env *out_env);

#ifdef M3_TESTING
/**
 * @brief Override backend initialization state (tests only).
 * @param backend Backend instance.
 * @param initialized M3_TRUE to mark initialized, M3_FALSE otherwise.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_null_backend_test_set_initialized(M3NullBackend *backend, M3Bool initialized);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_BACKEND_NULL_H */
