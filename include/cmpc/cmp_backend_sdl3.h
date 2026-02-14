#ifndef CMP_BACKEND_SDL3_H
#define CMP_BACKEND_SDL3_H

/**
 * @file cmp_backend_sdl3.h
 * @brief SDL3 debug backend for LibCMPC.
 *
 * @note When SDL3_ttf support is enabled, the SDL3 backend treats the
 *       `utf8_family` parameter in CMPTextVTable.create_font as a UTF-8 file
 *       path to a .ttf font file.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ws.h"
#include "cmpc/cmp_core.h"

/** @brief SDL3 renderer uses hardware acceleration. */
#define CMP_SDL3_RENDERER_ACCELERATED 0x01
/** @brief SDL3 renderer uses vertical sync. */
#define CMP_SDL3_RENDERER_PRESENTVSYNC 0x02
/** @brief SDL3 renderer supports render targets. */
#define CMP_SDL3_RENDERER_TARGETTEXTURE 0x04

/** @brief Opaque SDL3 backend instance. */
typedef struct CMPSDL3Backend CMPSDL3Backend;

/**
 * @brief Configuration for the SDL3 backend.
 */
typedef struct CMPSDL3BackendConfig {
  const CMPAllocator
      *allocator; /**< Allocator for backend memory (NULL uses default). */
  cmp_usize handle_capacity; /**< Maximum number of handles to manage. */
  cmp_u32 renderer_flags;    /**< Renderer flags (CMP_SDL3_RENDERER_*). */
  CMPBool enable_logging;    /**< Enable logging via cmp_log_write. */
  CMPBool
      enable_tasks; /**< Enable the default task runner for CMPEnv.get_tasks. */
} CMPSDL3BackendConfig;

/**
 * @brief Report whether SDL3 support is available in this build.
 * @param out_available Receives CMP_TRUE if SDL3 is available, CMP_FALSE
 * otherwise.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_sdl3_backend_is_available(CMPBool *out_available);

/**
 * @brief Initialize a SDL3 backend configuration with defaults.
 * @param config Config to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_sdl3_backend_config_init(CMPSDL3BackendConfig *config);

/**
 * @brief Create an SDL3 backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_sdl3_backend_create(const CMPSDL3BackendConfig *config,
                                             CMPSDL3Backend **out_backend);

/**
 * @brief Destroy an SDL3 backend instance.
 * @param backend Backend instance to destroy.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_sdl3_backend_destroy(CMPSDL3Backend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_sdl3_backend_get_ws(CMPSDL3Backend *backend,
                                             CMPWS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_sdl3_backend_get_gfx(CMPSDL3Backend *backend,
                                              CMPGfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_sdl3_backend_get_env(CMPSDL3Backend *backend,
                                              CMPEnv *out_env);

#ifdef CMP_TESTING
/**
 * @brief Force cmp_sdl3_backend_config_init to fail for coverage.
 * @param fail Set to CMP_TRUE to force the next init to fail.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_sdl3_backend_test_set_config_init_fail(CMPBool fail);

/**
 * @brief Test wrapper for SDL3 backend config validation.
 * @param config Backend configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_sdl3_backend_test_validate_config(const CMPSDL3BackendConfig *config);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_BACKEND_SDL3_H */
