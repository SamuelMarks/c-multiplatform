#ifndef CMP_BACKEND_GTK4_H
#define CMP_BACKEND_GTK4_H

/**
 * @file cmp_backend_gtk4.h
 * @brief GTK4 backend for LibCMPC on Linux.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ws.h"
#include "cmpc/cmp_core.h"

/** @brief Opaque GTK4 backend instance. */
typedef struct CMPGTK4Backend CMPGTK4Backend;

/**
 * @brief Configuration for the GTK4 backend.
 */
typedef struct CMPGTK4BackendConfig {
  const CMPAllocator
      *allocator; /**< Allocator for backend memory (NULL uses default). */
  cmp_usize handle_capacity; /**< Maximum number of handles to manage. */
  cmp_usize clipboard_limit; /**< Maximum clipboard byte length accepted. */
  CMPBool enable_logging;    /**< Enable logging via cmp_log_write. */
  CMPBool inline_tasks;      /**< Execute posted tasks inline when CMP_TRUE. */
  CMPBool
      enable_gdk_texture; /**< Enable GdkTexture path for GSK texture draws. */
} CMPGTK4BackendConfig;

/**
 * @brief Report whether GTK4 support is available in this build.
 * @param out_available Receives CMP_TRUE if GTK4 is available, CMP_FALSE
 * otherwise.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_gtk4_backend_is_available(CMPBool *out_available);

/**
 * @brief Initialize a GTK4 backend configuration with defaults.
 * @param config Config to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_gtk4_backend_config_init(CMPGTK4BackendConfig *config);

/**
 * @brief Create a GTK4 backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_gtk4_backend_create(const CMPGTK4BackendConfig *config,
                                             CMPGTK4Backend **out_backend);

/**
 * @brief Destroy a GTK4 backend instance.
 * @param backend Backend instance to destroy.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_gtk4_backend_destroy(CMPGTK4Backend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_gtk4_backend_get_ws(CMPGTK4Backend *backend,
                                             CMPWS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_gtk4_backend_get_gfx(CMPGTK4Backend *backend,
                                              CMPGfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_gtk4_backend_get_env(CMPGTK4Backend *backend,
                                              CMPEnv *out_env);

#ifdef CMP_TESTING
/**
 * @brief Force cmp_gtk4_backend_config_init to fail for coverage.
 * @param fail Set to CMP_TRUE to force the next init to fail.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_gtk4_backend_test_set_config_init_fail(CMPBool fail);

/**
 * @brief Test wrapper for GTK4 backend config validation.
 * @param config Backend configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_gtk4_backend_test_validate_config(const CMPGTK4BackendConfig *config);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_BACKEND_GTK4_H */
