#ifndef CMP_UTF8_H
#define CMP_UTF8_H

/**
 * @file cmp_utf8.h
 * @brief UTF-8 validation and iteration helpers for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"

/**
 * @brief UTF-8 iterator state.
 */
typedef struct CMPUtf8Iter {
  const cmp_u8 *data; /**< UTF-8 byte buffer. */
  cmp_usize length;   /**< Length of the buffer in bytes. */
  cmp_usize offset;   /**< Current byte offset. */
} CMPUtf8Iter;

/**
 * @brief Validate a UTF-8 buffer.
 * @param data UTF-8 byte buffer.
 * @param length Length of the buffer in bytes.
 * @param out_valid Receives CMP_TRUE if valid; CMP_FALSE otherwise.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_utf8_validate(const char *data, cmp_usize length,
                                    CMPBool *out_valid);

/**
 * @brief Validate a null-terminated UTF-8 string.
 * @param cstr Null-terminated UTF-8 string.
 * @param out_valid Receives CMP_TRUE if valid; CMP_FALSE otherwise.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_utf8_validate_cstr(const char *cstr, CMPBool *out_valid);

/**
 * @brief Initialize a UTF-8 iterator for a byte buffer.
 * @param iter Iterator to initialize.
 * @param data UTF-8 byte buffer.
 * @param length Length of the buffer in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_utf8_iter_init(CMPUtf8Iter *iter, const char *data,
                                     cmp_usize length);

/**
 * @brief Initialize a UTF-8 iterator for a null-terminated string.
 * @param iter Iterator to initialize.
 * @param cstr Null-terminated UTF-8 string.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_utf8_iter_init_cstr(CMPUtf8Iter *iter, const char *cstr);

/**
 * @brief Read the next UTF-8 codepoint from the iterator.
 * @param iter Iterator to advance.
 * @param out_codepoint Receives the decoded codepoint.
 * @return CMP_OK on success, CMP_ERR_NOT_FOUND at end, or a failure code.
 */
CMP_API int CMP_CALL cmp_utf8_iter_next(CMPUtf8Iter *iter, cmp_u32 *out_codepoint);

#ifdef CMP_TESTING
/**
 * @brief Override the maximum C-string length used for UTF-8 validation.
 * @param max_len Maximum allowed length.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_utf8_test_set_cstr_limit(cmp_usize max_len);

/**
 * @brief Test wrapper for UTF-8 decode helper.
 * @param data UTF-8 byte buffer.
 * @param length Buffer length in bytes.
 * @param out_codepoint Receives decoded codepoint.
 * @param out_advance Receives byte advance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_utf8_test_decode(const cmp_u8 *data, cmp_usize length,
                                       cmp_u32 *out_codepoint,
                                       cmp_usize *out_advance);

/**
 * @brief Toggle relaxed UTF-8 boundary checks (tests only).
 * @param enable CMP_TRUE to relax checks, CMP_FALSE to restore strict checks.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_utf8_test_set_relaxed_checks(CMPBool enable);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_UTF8_H */
