
#ifndef CMP_LOG_H
#define CMP_LOG_H

/* clang-format off */
#include <stdio.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef LOG_DEBUG
#ifdef DEBUG
void cmp_log_debug(const char *fmt, ...);
#define LOG_DEBUG cmp_log_debug
#else
void cmp_log_debug(const char *fmt, ...);
#define LOG_DEBUG 1 ? (void)0 : cmp_log_debug
#endif /* DEBUG */
#endif /* !LOG_DEBUG */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_LOG_H */
