#ifndef CMP_BACKEND_WEB_H
#define CMP_BACKEND_WEB_H

/**
 * @file cmp_backend_web.h
 * @brief Web (Emscripten) backend for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ws.h"
#include "cmpc/cmp_core.h"

/** @brief Opaque web backend instance. */
typedef struct CMPWebBackend CMPWebBackend;

/**
 * @brief Configuration for the web backend.
 */
typedef struct CMPWebBackendConfig {
  const CMPAllocator
      *allocator; /**< Allocator for backend memory (NULL uses default). */
  cmp_usize handle_capacity; /**< Maximum number of handles to manage. */
  cmp_usize clipboard_limit; /**< Maximum clipboard byte length accepted. */
  const char
      *utf8_canvas_id; /**< Canvas element selector (UTF-8, e.g. "#canvas"). */
  CMPBool enable_logging; /**< Enable logging via cmp_log_write. */
  CMPBool inline_tasks;   /**< Execute posted tasks inline when CMP_TRUE. */
} CMPWebBackendConfig;

/**
 * @brief Report whether web (Emscripten) support is available in this build.
 * @param out_available Receives CMP_TRUE if web support is available, CMP_FALSE
 * otherwise.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_web_backend_is_available(CMPBool *out_available);

/**
 * @brief Initialize a web backend configuration with defaults.
 * @param config Config to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_web_backend_config_init(CMPWebBackendConfig *config);

/**
 * @brief Create a web backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_web_backend_create(const CMPWebBackendConfig *config,
                                            CMPWebBackend **out_backend);

/**
 * @brief Destroy a web backend instance.
 * @param backend Backend instance to destroy.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_web_backend_destroy(CMPWebBackend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_web_backend_get_ws(CMPWebBackend *backend,
                                            CMPWS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_web_backend_get_gfx(CMPWebBackend *backend,
                                             CMPGfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_web_backend_get_env(CMPWebBackend *backend,
                                             CMPEnv *out_env);

#ifdef CMP_TESTING
/**
 * @brief Force cmp_web_backend_config_init to fail for coverage.
 * @param fail Set to CMP_TRUE to force the next init to fail.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_web_backend_test_set_config_init_fail(CMPBool fail);

/**
 * @brief Test wrapper for web backend config validation.
 * @param config Backend configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_web_backend_test_validate_config(const CMPWebBackendConfig *config);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_BACKEND_WEB_H */
