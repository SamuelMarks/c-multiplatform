#ifndef M3_BACKEND_WIN32_H
#define M3_BACKEND_WIN32_H

/**
 * @file m3_backend_win32.h
 * @brief Win32 (GDI) backend for LibM3C on Windows.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_env.h"
#include "m3_api_gfx.h"
#include "m3_api_ws.h"
#include "m3_core.h"

/** @brief Opaque Win32 backend instance. */
typedef struct M3Win32Backend M3Win32Backend;

/**
 * @brief Configuration for the Win32 backend.
 */
typedef struct M3Win32BackendConfig {
    const M3Allocator *allocator; /**< Allocator for backend memory (NULL uses default). */
    m3_usize handle_capacity; /**< Maximum number of handles to manage. */
    m3_usize clipboard_limit; /**< Maximum clipboard byte length accepted. */
    M3Bool enable_logging; /**< Enable logging via m3_log_write. */
    M3Bool inline_tasks; /**< Execute posted tasks inline when M3_TRUE. */
} M3Win32BackendConfig;

/**
 * @brief Report whether the Win32 backend is available in this build.
 * @param out_available Receives M3_TRUE if Win32 support is available, M3_FALSE otherwise.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_win32_backend_is_available(M3Bool *out_available);

/**
 * @brief Initialize a Win32 backend configuration with defaults.
 * @param config Config to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_win32_backend_config_init(M3Win32BackendConfig *config);

/**
 * @brief Create a Win32 backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_win32_backend_create(const M3Win32BackendConfig *config, M3Win32Backend **out_backend);

/**
 * @brief Destroy a Win32 backend instance.
 * @param backend Backend instance to destroy.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_win32_backend_destroy(M3Win32Backend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_win32_backend_get_ws(M3Win32Backend *backend, M3WS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_win32_backend_get_gfx(M3Win32Backend *backend, M3Gfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_win32_backend_get_env(M3Win32Backend *backend, M3Env *out_env);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for Win32 backend config validation.
 * @param config Backend configuration.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_win32_backend_test_validate_config(const M3Win32BackendConfig *config);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_BACKEND_WIN32_H */
