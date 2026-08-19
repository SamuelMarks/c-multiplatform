/**
 * @file clipboard_posix.h
 * @brief Internal POSIX declarations for clipboard implementation.
 */

#ifndef CLIPBOARD_POSIX_H
#define CLIPBOARD_POSIX_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include <stdio.h>
/* clang-format on */

/**
 * @addtogroup ui_clipboard Clipboard
 * @{
 */

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
/**
 * @brief Standard POSIX popen declaration.
 *
 * @param command The shell command to execute.
 * @param type "r" or "w" for reading or writing.
 * @return A valid file pointer on success, or NULL on error.
 */
extern FILE *popen(const char *command, const char *type);

/**
 * @brief Standard POSIX pclose declaration.
 *
 * @param stream The file pointer to close.
 * @return The exit status of the command.
 */
extern int pclose(FILE *stream);
#endif

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CLIPBOARD_POSIX_H */
