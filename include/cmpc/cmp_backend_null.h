#ifndef CMP_BACKEND_NULL_H
#define CMP_BACKEND_NULL_H

/**
 * @file cmp_backend_null.h
 * @brief Null backend implementation for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ws.h"
#include "cmpc/cmp_core.h"

/** @brief Opaque null backend instance. */
typedef struct CMPNullBackend CMPNullBackend;

/**
 * @brief Configuration for the null backend.
 */
typedef struct CMPNullBackendConfig {
  const CMPAllocator
      *allocator; /**< Allocator for backend memory (NULL uses default). */
  cmp_usize handle_capacity; /**< Maximum number of handles to manage. */
  cmp_usize clipboard_limit; /**< Maximum clipboard byte length accepted. */
  CMPBool enable_logging;    /**< Enable logging via cmp_log_write. */
  CMPBool inline_tasks;      /**< Execute posted tasks inline when CMP_TRUE. */
} CMPNullBackendConfig;

/**
 * @brief Initialize a null backend configuration with defaults.
 * @param config Config to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_null_backend_config_init(CMPNullBackendConfig *config);

/**
 * @brief Create a null backend instance.
 * @param config Backend configuration (NULL uses defaults).
 * @param out_backend Receives the created backend instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_null_backend_create(const CMPNullBackendConfig *config,
                                          CMPNullBackend **out_backend);

/**
 * @brief Destroy a null backend instance.
 * @param backend Backend instance to destroy.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_null_backend_destroy(CMPNullBackend *backend);

/**
 * @brief Retrieve the window system interface for the backend.
 * @param backend Backend instance.
 * @param out_ws Receives the window system interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_null_backend_get_ws(CMPNullBackend *backend, CMPWS *out_ws);

/**
 * @brief Retrieve the graphics interface for the backend.
 * @param backend Backend instance.
 * @param out_gfx Receives the graphics interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_null_backend_get_gfx(CMPNullBackend *backend,
                                           CMPGfx *out_gfx);

/**
 * @brief Retrieve the environment interface for the backend.
 * @param backend Backend instance.
 * @param out_env Receives the environment interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_null_backend_get_env(CMPNullBackend *backend,
                                           CMPEnv *out_env);

#ifdef CMP_TESTING
/**
 * @brief Override backend initialization state (tests only).
 * @param backend Backend instance.
 * @param initialized CMP_TRUE to mark initialized, CMP_FALSE otherwise.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_null_backend_test_set_initialized(CMPNullBackend *backend,
                                                        CMPBool initialized);

/**
 * @brief Test wrapper for null backend object retain.
 * @param obj Object header to retain.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_null_backend_test_object_retain(CMPObjectHeader *obj);

/**
 * @brief Test wrapper for null backend object release.
 * @param obj Object header to release.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_null_backend_test_object_release(CMPObjectHeader *obj);

/**
 * @brief Test wrapper for null backend object type query.
 * @param obj Object header to query.
 * @param out_type_id Receives the type identifier.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_null_backend_test_object_get_type_id(CMPObjectHeader *obj,
                                                           cmp_u32 *out_type_id);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_BACKEND_NULL_H */
