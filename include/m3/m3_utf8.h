#ifndef M3_UTF8_H
#define M3_UTF8_H

/**
 * @file m3_utf8.h
 * @brief UTF-8 validation and iteration helpers for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_core.h"

/**
 * @brief UTF-8 iterator state.
 */
typedef struct M3Utf8Iter {
    const m3_u8 *data; /**< UTF-8 byte buffer. */
    m3_usize length; /**< Length of the buffer in bytes. */
    m3_usize offset; /**< Current byte offset. */
} M3Utf8Iter;

/**
 * @brief Validate a UTF-8 buffer.
 * @param data UTF-8 byte buffer.
 * @param length Length of the buffer in bytes.
 * @param out_valid Receives M3_TRUE if valid; M3_FALSE otherwise.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_utf8_validate(const char *data, m3_usize length, M3Bool *out_valid);

/**
 * @brief Validate a null-terminated UTF-8 string.
 * @param cstr Null-terminated UTF-8 string.
 * @param out_valid Receives M3_TRUE if valid; M3_FALSE otherwise.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_utf8_validate_cstr(const char *cstr, M3Bool *out_valid);

/**
 * @brief Initialize a UTF-8 iterator for a byte buffer.
 * @param iter Iterator to initialize.
 * @param data UTF-8 byte buffer.
 * @param length Length of the buffer in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_utf8_iter_init(M3Utf8Iter *iter, const char *data, m3_usize length);

/**
 * @brief Initialize a UTF-8 iterator for a null-terminated string.
 * @param iter Iterator to initialize.
 * @param cstr Null-terminated UTF-8 string.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_utf8_iter_init_cstr(M3Utf8Iter *iter, const char *cstr);

/**
 * @brief Read the next UTF-8 codepoint from the iterator.
 * @param iter Iterator to advance.
 * @param out_codepoint Receives the decoded codepoint.
 * @return M3_OK on success, M3_ERR_NOT_FOUND at end, or a failure code.
 */
M3_API int M3_CALL m3_utf8_iter_next(M3Utf8Iter *iter, m3_u32 *out_codepoint);

#ifdef M3_TESTING
/**
 * @brief Override the maximum C-string length used for UTF-8 validation.
 * @param max_len Maximum allowed length.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_utf8_test_set_cstr_limit(m3_usize max_len);

/**
 * @brief Test wrapper for UTF-8 decode helper.
 * @param data UTF-8 byte buffer.
 * @param length Buffer length in bytes.
 * @param out_codepoint Receives decoded codepoint.
 * @param out_advance Receives byte advance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_utf8_test_decode(const m3_u8 *data, m3_usize length, m3_u32 *out_codepoint, m3_usize *out_advance);

/**
 * @brief Toggle relaxed UTF-8 boundary checks (tests only).
 * @param enable M3_TRUE to relax checks, M3_FALSE to restore strict checks.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_utf8_test_set_relaxed_checks(M3Bool enable);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_UTF8_H */
