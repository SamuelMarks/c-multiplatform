/**
 * @file strtok_posix.h
 * @brief Internal POSIX declaration for strtok_r.
 */

#ifndef STRTOK_POSIX_H
#define STRTOK_POSIX_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ui_utils Utilities
 * @{
 */

#if !defined(_MSC_VER)
/**
 * @brief Standard POSIX strtok_r declaration.
 *
 * @param str The string to tokenize.
 * @param delim The delimiter characters.
 * @param saveptr A pointer to a char * variable that is used internally to
 * maintain context.
 * @return A pointer to the next token, or NULL if there are no more tokens.
 */
extern char *strtok_r(char *str, const char *delim, char **saveptr);
#endif

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* STRTOK_POSIX_H */
