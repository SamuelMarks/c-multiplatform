#ifndef CMP_LOG_H
#define CMP_LOG_H

/**
 * @file cmp_log.h
 * @brief Thread-safe logging sink for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"

/** @brief Log level value type. */
typedef cmp_u32 CMPLogLevel;

/** @brief Trace-level diagnostic output. */
#define CMP_LOG_LEVEL_TRACE 0
/** @brief Debug-level diagnostic output. */
#define CMP_LOG_LEVEL_DEBUG 1
/** @brief Informational output. */
#define CMP_LOG_LEVEL_INFO 2
/** @brief Warning output. */
#define CMP_LOG_LEVEL_WARN 3
/** @brief Error output. */
#define CMP_LOG_LEVEL_ERROR 4
/** @brief Fatal error output. */
#define CMP_LOG_LEVEL_FATAL 5

/**
 * @brief Log sink callback.
 * @param ctx Sink context pointer.
 * @param level Log level.
 * @param tag Optional tag string (may be NULL).
 * @param message Message bytes.
 * @param length Message length in bytes.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPLogSinkFn)(void *ctx, CMPLogLevel level,
                                    const char *tag, const char *message,
                                    cmp_usize length);

/**
 * @brief Log sink descriptor.
 */
typedef struct CMPLogSink {
  void *ctx;          /**< Sink context pointer. */
  CMPLogSinkFn write; /**< Sink write callback. */
} CMPLogSink;

/**
 * @brief Initialize the logging system.
 * @param allocator Allocator for internal resources; NULL uses the default
 * allocator.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_log_init(const CMPAllocator *allocator);

/**
 * @brief Shut down the logging system.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_log_shutdown(void);

/**
 * @brief Set the active log sink.
 * @param sink Sink descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_log_set_sink(const CMPLogSink *sink);

/**
 * @brief Retrieve the active log sink.
 * @param out_sink Receives the sink descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_log_get_sink(CMPLogSink *out_sink);

/**
 * @brief Write a log message with a null-terminated string.
 * @param level Log level.
 * @param tag Optional tag string (may be NULL).
 * @param message Null-terminated message string.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_log_write(CMPLogLevel level, const char *tag,
                                   const char *message);

/**
 * @brief Write a log message with an explicit length.
 * @param level Log level.
 * @param tag Optional tag string (may be NULL).
 * @param message Message bytes.
 * @param length Message length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_log_write_n(CMPLogLevel level, const char *tag,
                                     const char *message, cmp_usize length);

#ifdef CMP_TESTING
/**
 * @brief Configure failure injection for the logging mutex.
 * @param init_fail Force initialization to fail when CMP_TRUE.
 * @param lock_fail Force locking to fail when CMP_TRUE.
 * @param unlock_fail Force unlocking to fail when CMP_TRUE.
 * @param shutdown_fail Force shutdown to fail when CMP_TRUE.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_log_test_set_mutex_failures(CMPBool init_fail,
                                                     CMPBool lock_fail,
                                                     CMPBool unlock_fail,
                                                     CMPBool shutdown_fail);

/**
 * @brief Configure failure injection for log output.
 * @param write_fail Force output to fail when CMP_TRUE.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_log_test_set_io_fail(CMPBool write_fail);

/**
 * @brief Override the maximum C-string length used by logging.
 * @param max_len Maximum allowed length.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_log_test_set_cstr_limit(cmp_usize max_len);

/**
 * @brief Fail a specific log write call when non-zero (tests only).
 * @param call_index 1-based write call index to fail, or 0 to disable.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_log_test_set_io_fail_on_call(cmp_usize call_index);

/**
 * @brief Test wrapper for internal C-string length helper.
 * @param cstr Null-terminated string.
 * @param out_len Receives string length.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_log_test_cstrlen(const char *cstr, cmp_usize *out_len);

/**
 * @brief Test wrapper for the default log sink.
 * @param ctx Sink context (FILE*).
 * @param level Log level.
 * @param tag Optional tag string.
 * @param message Message bytes.
 * @param length Message length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_log_test_default_sink(void *ctx, CMPLogLevel level,
                                               const char *tag,
                                               const char *message,
                                               cmp_usize length);

/**
 * @brief Test wrapper to lock the log mutex directly.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_log_test_mutex_lock(void);

/**
 * @brief Test wrapper to unlock the log mutex directly.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_log_test_mutex_unlock(void);

/**
 * @brief Test wrapper to shut down the log mutex directly.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_log_test_mutex_shutdown(void);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_LOG_H */
