#ifndef M3_BACKEND_GTK4_H
#define M3_BACKEND_GTK4_H

/**
 * @file m3_backend_gtk4.h
 * @brief GTK4 backend for LibM3C on Linux.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_env.h"
#include "m3_api_gfx.h"
#include "m3_api_ws.h"
#include "m3_core.h"

/** @brief Opaque GTK4 backend instance. */
typedef struct M3GTK4Backend M3GTK4Backend;

/**
 * @brief Configuration for the GTK4 backend.
 */
typedef struct M3GTK4BackendConfig {
    const M3Allocator *allocator; /**< Allocator for backend memory (NULL uses default). */
    m3_usize handle_capacity; /**< Maximum number of handles to manage. */
    m3_usize clipboard_limit; /**< Maximum clipboard byte length accepted. */
    M3Bool enable_logging; /**< Enable logging via m3_log_write. */
    M3Bool inline_tasks; /**< Execute posted tasks inline when M3_TRUE. */
    M3Bool enable_gdk_texture; /**< Enable GdkTexture path for GSK texture draws. */
} M3GTK4BackendConfig;

/**
 * @brief Report whether GTK4 support is available in this build.
 * @param out_available Receives M3_TRUE if GTK4 is available, M3_FALSE otherwise.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_gtk4_backend_is_available(M3Bool *out_available);

/**
 * @brief Initialize a GTK4 backend configuration with defaults.
 * @param config Config to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_gtk4_backend_config_init(M3GTK4BackendConfig *config);

/**
 * @brief Create a GTK4 backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_gtk4_backend_create(const M3GTK4BackendConfig *config, M3GTK4Backend **out_backend);

/**
 * @brief Destroy a GTK4 backend instance.
 * @param backend Backend instance to destroy.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_gtk4_backend_destroy(M3GTK4Backend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_gtk4_backend_get_ws(M3GTK4Backend *backend, M3WS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_gtk4_backend_get_gfx(M3GTK4Backend *backend, M3Gfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_gtk4_backend_get_env(M3GTK4Backend *backend, M3Env *out_env);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for GTK4 backend config validation.
 * @param config Backend configuration.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_gtk4_backend_test_validate_config(const M3GTK4BackendConfig *config);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_BACKEND_GTK4_H */
