#ifndef M3_LOG_H
#define M3_LOG_H

/**
 * @file m3_log.h
 * @brief Thread-safe logging sink for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_core.h"

/** @brief Log level value type. */
typedef m3_u32 M3LogLevel;

/** @brief Trace-level diagnostic output. */
#define M3_LOG_LEVEL_TRACE 0
/** @brief Debug-level diagnostic output. */
#define M3_LOG_LEVEL_DEBUG 1
/** @brief Informational output. */
#define M3_LOG_LEVEL_INFO 2
/** @brief Warning output. */
#define M3_LOG_LEVEL_WARN 3
/** @brief Error output. */
#define M3_LOG_LEVEL_ERROR 4
/** @brief Fatal error output. */
#define M3_LOG_LEVEL_FATAL 5

/**
 * @brief Log sink callback.
 * @param ctx Sink context pointer.
 * @param level Log level.
 * @param tag Optional tag string (may be NULL).
 * @param message Message bytes.
 * @param length Message length in bytes.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3LogSinkFn)(void *ctx, M3LogLevel level, const char *tag,
                                  const char *message, m3_usize length);

/**
 * @brief Log sink descriptor.
 */
typedef struct M3LogSink {
  void *ctx;         /**< Sink context pointer. */
  M3LogSinkFn write; /**< Sink write callback. */
} M3LogSink;

/**
 * @brief Initialize the logging system.
 * @param allocator Allocator for internal resources; NULL uses the default
 * allocator.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_log_init(const M3Allocator *allocator);

/**
 * @brief Shut down the logging system.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_log_shutdown(void);

/**
 * @brief Set the active log sink.
 * @param sink Sink descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_log_set_sink(const M3LogSink *sink);

/**
 * @brief Retrieve the active log sink.
 * @param out_sink Receives the sink descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_log_get_sink(M3LogSink *out_sink);

/**
 * @brief Write a log message with a null-terminated string.
 * @param level Log level.
 * @param tag Optional tag string (may be NULL).
 * @param message Null-terminated message string.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_log_write(M3LogLevel level, const char *tag,
                                const char *message);

/**
 * @brief Write a log message with an explicit length.
 * @param level Log level.
 * @param tag Optional tag string (may be NULL).
 * @param message Message bytes.
 * @param length Message length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_log_write_n(M3LogLevel level, const char *tag,
                                  const char *message, m3_usize length);

#ifdef M3_TESTING
/**
 * @brief Configure failure injection for the logging mutex.
 * @param init_fail Force initialization to fail when M3_TRUE.
 * @param lock_fail Force locking to fail when M3_TRUE.
 * @param unlock_fail Force unlocking to fail when M3_TRUE.
 * @param shutdown_fail Force shutdown to fail when M3_TRUE.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_log_test_set_mutex_failures(M3Bool init_fail,
                                                  M3Bool lock_fail,
                                                  M3Bool unlock_fail,
                                                  M3Bool shutdown_fail);

/**
 * @brief Configure failure injection for log output.
 * @param write_fail Force output to fail when M3_TRUE.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_log_test_set_io_fail(M3Bool write_fail);

/**
 * @brief Override the maximum C-string length used by logging.
 * @param max_len Maximum allowed length.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_log_test_set_cstr_limit(m3_usize max_len);

/**
 * @brief Fail a specific log write call when non-zero (tests only).
 * @param call_index 1-based write call index to fail, or 0 to disable.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_log_test_set_io_fail_on_call(m3_usize call_index);

/**
 * @brief Test wrapper for internal C-string length helper.
 * @param cstr Null-terminated string.
 * @param out_len Receives string length.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_log_test_cstrlen(const char *cstr, m3_usize *out_len);

/**
 * @brief Test wrapper to lock the log mutex directly.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_log_test_mutex_lock(void);

/**
 * @brief Test wrapper to unlock the log mutex directly.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_log_test_mutex_unlock(void);

/**
 * @brief Test wrapper to shut down the log mutex directly.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_log_test_mutex_shutdown(void);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_LOG_H */
