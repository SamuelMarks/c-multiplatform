#ifndef M3_BACKEND_SDL3_H
#define M3_BACKEND_SDL3_H

/**
 * @file m3_backend_sdl3.h
 * @brief SDL3 debug backend for LibM3C.
 *
 * @note When SDL3_ttf support is enabled, the SDL3 backend treats the
 *       `utf8_family` parameter in M3TextVTable.create_font as a UTF-8 file
 *       path to a .ttf font file.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_env.h"
#include "m3_api_gfx.h"
#include "m3_api_ws.h"
#include "m3_core.h"

/** @brief SDL3 renderer uses hardware acceleration. */
#define M3_SDL3_RENDERER_ACCELERATED 0x01
/** @brief SDL3 renderer uses vertical sync. */
#define M3_SDL3_RENDERER_PRESENTVSYNC 0x02
/** @brief SDL3 renderer supports render targets. */
#define M3_SDL3_RENDERER_TARGETTEXTURE 0x04

/** @brief Opaque SDL3 backend instance. */
typedef struct M3SDL3Backend M3SDL3Backend;

/**
 * @brief Configuration for the SDL3 backend.
 */
typedef struct M3SDL3BackendConfig {
  const M3Allocator
      *allocator; /**< Allocator for backend memory (NULL uses default). */
  m3_usize handle_capacity; /**< Maximum number of handles to manage. */
  m3_u32 renderer_flags;    /**< Renderer flags (M3_SDL3_RENDERER_*). */
  M3Bool enable_logging;    /**< Enable logging via m3_log_write. */
  M3Bool
      enable_tasks; /**< Enable the default task runner for M3Env.get_tasks. */
} M3SDL3BackendConfig;

/**
 * @brief Report whether SDL3 support is available in this build.
 * @param out_available Receives M3_TRUE if SDL3 is available, M3_FALSE
 * otherwise.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sdl3_backend_is_available(M3Bool *out_available);

/**
 * @brief Initialize a SDL3 backend configuration with defaults.
 * @param config Config to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sdl3_backend_config_init(M3SDL3BackendConfig *config);

/**
 * @brief Create an SDL3 backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sdl3_backend_create(const M3SDL3BackendConfig *config,
                                          M3SDL3Backend **out_backend);

/**
 * @brief Destroy an SDL3 backend instance.
 * @param backend Backend instance to destroy.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sdl3_backend_destroy(M3SDL3Backend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sdl3_backend_get_ws(M3SDL3Backend *backend, M3WS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sdl3_backend_get_gfx(M3SDL3Backend *backend,
                                           M3Gfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sdl3_backend_get_env(M3SDL3Backend *backend,
                                           M3Env *out_env);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for SDL3 backend config validation.
 * @param config Backend configuration.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_sdl3_backend_test_validate_config(const M3SDL3BackendConfig *config);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_BACKEND_SDL3_H */
