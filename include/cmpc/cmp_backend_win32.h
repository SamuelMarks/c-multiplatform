#ifndef CMP_BACKEND_WIN32_H
#define CMP_BACKEND_WIN32_H

/**
 * @file cmp_backend_win32.h
 * @brief Win32 (GDI) backend for LibCMPC on Windows.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ws.h"
#include "cmpc/cmp_core.h"

/** @brief Opaque Win32 backend instance. */
typedef struct CMPWin32Backend CMPWin32Backend;

/**
 * @brief Configuration for the Win32 backend.
 */
typedef struct CMPWin32BackendConfig {
  const CMPAllocator
      *allocator; /**< Allocator for backend memory (NULL uses default). */
  cmp_usize handle_capacity; /**< Maximum number of handles to manage. */
  cmp_usize clipboard_limit; /**< Maximum clipboard byte length accepted. */
  CMPBool enable_logging;    /**< Enable logging via cmp_log_write. */
  CMPBool inline_tasks;      /**< Execute posted tasks inline when CMP_TRUE. */
} CMPWin32BackendConfig;

/**
 * @brief Report whether the Win32 backend is available in this build.
 * @param out_available Receives CMP_TRUE if Win32 support is available, CMP_FALSE
 * otherwise.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_win32_backend_is_available(CMPBool *out_available);

/**
 * @brief Initialize a Win32 backend configuration with defaults.
 * @param config Config to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_win32_backend_config_init(CMPWin32BackendConfig *config);

/**
 * @brief Create a Win32 backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_win32_backend_create(const CMPWin32BackendConfig *config,
                                           CMPWin32Backend **out_backend);

/**
 * @brief Destroy a Win32 backend instance.
 * @param backend Backend instance to destroy.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_win32_backend_destroy(CMPWin32Backend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_win32_backend_get_ws(CMPWin32Backend *backend,
                                           CMPWS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_win32_backend_get_gfx(CMPWin32Backend *backend,
                                            CMPGfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_win32_backend_get_env(CMPWin32Backend *backend,
                                            CMPEnv *out_env);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for Win32 backend config validation.
 * @param config Backend configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_win32_backend_test_validate_config(const CMPWin32BackendConfig *config);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_BACKEND_WIN32_H */
