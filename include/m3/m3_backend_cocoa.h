#ifndef M3_BACKEND_COCOA_H
#define M3_BACKEND_COCOA_H

/**
 * @file m3_backend_cocoa.h
 * @brief Cocoa backend for LibM3C on macOS.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_env.h"
#include "m3_api_gfx.h"
#include "m3_api_ws.h"
#include "m3_core.h"

/** @brief Opaque Cocoa backend instance. */
typedef struct M3CocoaBackend M3CocoaBackend;

/**
 * @brief Configuration for the Cocoa backend.
 */
typedef struct M3CocoaBackendConfig {
    const M3Allocator *allocator; /**< Allocator for backend memory (NULL uses default). */
    m3_usize handle_capacity; /**< Maximum number of handles to manage. */
    m3_usize clipboard_limit; /**< Maximum clipboard byte length accepted. */
    M3Bool enable_logging; /**< Enable logging via m3_log_write. */
    M3Bool inline_tasks; /**< Execute posted tasks inline when M3_TRUE. */
} M3CocoaBackendConfig;

/**
 * @brief Report whether Cocoa support is available in this build.
 * @param out_available Receives M3_TRUE if Cocoa is available, M3_FALSE otherwise.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_cocoa_backend_is_available(M3Bool *out_available);

/**
 * @brief Initialize a Cocoa backend configuration with defaults.
 * @param config Config to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_cocoa_backend_config_init(M3CocoaBackendConfig *config);

/**
 * @brief Create a Cocoa backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_cocoa_backend_create(const M3CocoaBackendConfig *config, M3CocoaBackend **out_backend);

/**
 * @brief Destroy a Cocoa backend instance.
 * @param backend Backend instance to destroy.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_cocoa_backend_destroy(M3CocoaBackend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_cocoa_backend_get_ws(M3CocoaBackend *backend, M3WS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_cocoa_backend_get_gfx(M3CocoaBackend *backend, M3Gfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_cocoa_backend_get_env(M3CocoaBackend *backend, M3Env *out_env);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for Cocoa backend config validation.
 * @param config Backend configuration.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_cocoa_backend_test_validate_config(const M3CocoaBackendConfig *config);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_BACKEND_COCOA_H */
