#ifndef CMP_BACKEND_COCOA_H
#define CMP_BACKEND_COCOA_H

/**
 * @file cmp_backend_cocoa.h
 * @brief Cocoa backend for LibCMPC on macOS.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ws.h"
#include "cmpc/cmp_core.h"

/** @brief Opaque Cocoa backend instance. */
typedef struct CMPCocoaBackend CMPCocoaBackend;

/**
 * @brief Configuration for the Cocoa backend.
 */
typedef struct CMPCocoaBackendConfig {
  const CMPAllocator
      *allocator; /**< Allocator for backend memory (NULL uses default). */
  cmp_usize handle_capacity; /**< Maximum number of handles to manage. */
  cmp_usize clipboard_limit; /**< Maximum clipboard byte length accepted. */
  CMPBool enable_logging;    /**< Enable logging via cmp_log_write. */
  CMPBool inline_tasks;      /**< Execute posted tasks inline when CMP_TRUE. */
} CMPCocoaBackendConfig;

/**
 * @brief Report whether Cocoa support is available in this build.
 * @param out_available Receives CMP_TRUE if Cocoa is available, CMP_FALSE
 * otherwise.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_cocoa_backend_is_available(CMPBool *out_available);

/**
 * @brief Initialize a Cocoa backend configuration with defaults.
 * @param config Config to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_cocoa_backend_config_init(CMPCocoaBackendConfig *config);

/**
 * @brief Create a Cocoa backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_cocoa_backend_create(const CMPCocoaBackendConfig *config,
                                           CMPCocoaBackend **out_backend);

/**
 * @brief Destroy a Cocoa backend instance.
 * @param backend Backend instance to destroy.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_cocoa_backend_destroy(CMPCocoaBackend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_cocoa_backend_get_ws(CMPCocoaBackend *backend,
                                           CMPWS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_cocoa_backend_get_gfx(CMPCocoaBackend *backend,
                                            CMPGfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_cocoa_backend_get_env(CMPCocoaBackend *backend,
                                            CMPEnv *out_env);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for Cocoa backend config validation.
 * @param config Backend configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_cocoa_backend_test_validate_config(const CMPCocoaBackendConfig *config);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_BACKEND_COCOA_H */
