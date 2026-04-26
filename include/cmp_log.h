#ifndef CMP_LOG_H
#define CMP_LOG_H

/* clang-format off */
#include <stdio.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Log a debug message to standard error.
 *
 * This function handles format strings and variable arguments safely.
 * Under MSVC, it automatically uses safe CRT functions.
 *
 * @param fmt The format string.
 * @param ... Variable arguments matching the format string.
 */
void cmp_log_debug(const char *fmt, ...);

#ifndef LOG_DEBUG
#ifdef DEBUG
/** @brief Debug log macro mapped to cmp_log_debug. */
#define LOG_DEBUG cmp_log_debug
#else
/** @brief Debug log macro compiled out in non-debug builds. */
#define LOG_DEBUG 1 ? (void)0 : cmp_log_debug
#endif /* DEBUG */
#endif /* !LOG_DEBUG */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_LOG_H */