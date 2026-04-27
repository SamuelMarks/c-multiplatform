#ifndef CMP_REGEX_H
#define CMP_REGEX_H

/* clang-format off */
#include "cmp_ffi.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_regex.h
 * @brief Simple Regex Engine cross-platform wrapper.
 */

typedef struct cmp_regex cmp_regex_t;

/**
 * @brief Compiles a regular expression pattern.
 * @param out_regex Pointer to receive the compiled regex object.
 * @param pattern The regex pattern string.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_regex_compile(cmp_regex_t **out_regex, const char *pattern);

/**
 * @brief Tests if a string matches the compiled regex.
 * @param regex The compiled regex object.
 * @param string The string to test.
 * @param out_matched Pointer to receive 1 if matched, 0 otherwise.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_regex_match(cmp_regex_t *regex, const char *string,
                            int *out_matched);

/**
 * @brief Frees a compiled regex object.
 * @param regex The object to free.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_regex_free(cmp_regex_t *regex);
#ifdef __cplusplus
}
#endif

#endif /* CMP_REGEX_H */
