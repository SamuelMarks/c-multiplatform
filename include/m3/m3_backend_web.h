#ifndef M3_BACKEND_WEB_H
#define M3_BACKEND_WEB_H

/**
 * @file m3_backend_web.h
 * @brief Web (Emscripten) backend for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_env.h"
#include "m3_api_gfx.h"
#include "m3_api_ws.h"
#include "m3_core.h"

/** @brief Opaque web backend instance. */
typedef struct M3WebBackend M3WebBackend;

/**
 * @brief Configuration for the web backend.
 */
typedef struct M3WebBackendConfig {
    const M3Allocator *allocator; /**< Allocator for backend memory (NULL uses default). */
    m3_usize handle_capacity; /**< Maximum number of handles to manage. */
    m3_usize clipboard_limit; /**< Maximum clipboard byte length accepted. */
    const char *utf8_canvas_id; /**< Canvas element selector (UTF-8, e.g. "#canvas"). */
    M3Bool enable_logging; /**< Enable logging via m3_log_write. */
    M3Bool inline_tasks; /**< Execute posted tasks inline when M3_TRUE. */
} M3WebBackendConfig;

/**
 * @brief Report whether web (Emscripten) support is available in this build.
 * @param out_available Receives M3_TRUE if web support is available, M3_FALSE otherwise.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_web_backend_is_available(M3Bool *out_available);

/**
 * @brief Initialize a web backend configuration with defaults.
 * @param config Config to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_web_backend_config_init(M3WebBackendConfig *config);

/**
 * @brief Create a web backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_web_backend_create(const M3WebBackendConfig *config, M3WebBackend **out_backend);

/**
 * @brief Destroy a web backend instance.
 * @param backend Backend instance to destroy.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_web_backend_destroy(M3WebBackend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_web_backend_get_ws(M3WebBackend *backend, M3WS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_web_backend_get_gfx(M3WebBackend *backend, M3Gfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_web_backend_get_env(M3WebBackend *backend, M3Env *out_env);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for web backend config validation.
 * @param config Backend configuration.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_web_backend_test_validate_config(const M3WebBackendConfig *config);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_BACKEND_WEB_H */
