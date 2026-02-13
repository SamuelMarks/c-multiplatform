#include "cmpc/cmp_utf8.h"
#include "test_utils.h"

#include <string.h>

int main(void) {
  CMPUtf8Iter iter;
  CMPBool valid;
  cmp_u32 codepoint;
  cmp_usize max_size;
  cmp_usize advance;
  int rc;

  const char ascii[] = "hello";
  const unsigned char two_byte_bytes[] = {0xC2u, 0xA2u, '\0'};
  const unsigned char three_byte_bytes[] = {0xE2u, 0x82u, 0xACu, '\0'};
  const unsigned char four_byte_bytes[] = {0xF0u, 0x9Fu, 0x98u, 0x80u, '\0'};
  const unsigned char mixed_bytes[] = {'A',   0xE2u, 0x82u, 0xACu, 0xF0u,
                                       0x9Fu, 0x98u, 0x80u, '\0'};

  const unsigned char invalid_start_bytes[] = {0x80u, '\0'};
  const unsigned char invalid_overlong_bytes[] = {0xC0u, 0x80u, '\0'};
  const unsigned char invalid_two_bad_cont_bytes[] = {0xC2u, 0x20u, '\0'};
  const unsigned char invalid_trunc3_bytes[] = {0xE2u, 0x82u, '\0'};
  const unsigned char invalid_bad_b2_bytes[] = {0xE2u, 0x82u, 0x28u, '\0'};
  const unsigned char invalid_trunc4_bytes[] = {0xF0u, 0x9Fu, 0x98u, '\0'};
  const unsigned char invalid_surrogate_bytes[] = {0xEDu, 0xA0u, 0x80u, '\0'};
  const unsigned char invalid_too_large_bytes[] = {0xF4u, 0x90u, 0x80u, 0x80u,
                                                   '\0'};
  const unsigned char invalid_four_boundary_bytes[] = {0xF0u, 0x80u, 0x80u,
                                                       0x80u, '\0'};
  const unsigned char invalid_high_start_bytes[] = {0xF5u, 0x80u, 0x80u, 0x80u,
                                                    '\0'};
  const unsigned char invalid_bad_b2_4_bytes[] = {0xF0u, 0x90u, 0x28u, 0xBCu,
                                                  '\0'};
  const unsigned char invalid_continuation_bytes[] = {0xE2u, 0x28u, 0xA1u,
                                                      '\0'};

  const char *two_byte = (const char *)two_byte_bytes;
  const char *three_byte = (const char *)three_byte_bytes;
  const char *four_byte = (const char *)four_byte_bytes;
  const char *mixed = (const char *)mixed_bytes;
  const char *invalid_start = (const char *)invalid_start_bytes;
  const char *invalid_overlong = (const char *)invalid_overlong_bytes;
  const char *invalid_two_bad_cont = (const char *)invalid_two_bad_cont_bytes;
  const char *invalid_trunc3 = (const char *)invalid_trunc3_bytes;
  const char *invalid_bad_b2 = (const char *)invalid_bad_b2_bytes;
  const char *invalid_trunc4 = (const char *)invalid_trunc4_bytes;
  const char *invalid_surrogate = (const char *)invalid_surrogate_bytes;
  const char *invalid_too_large = (const char *)invalid_too_large_bytes;
  const char *invalid_four_boundary = (const char *)invalid_four_boundary_bytes;
  const char *invalid_high_start = (const char *)invalid_high_start_bytes;
  const char *invalid_bad_b2_4 = (const char *)invalid_bad_b2_4_bytes;
  const char *invalid_continuation = (const char *)invalid_continuation_bytes;
  const cmp_u8 invalid_trunc2[] = {0xC2u};
  const cmp_u8 invalid_e0_boundary[] = {0xE0u, 0x9Fu, 0x80u};
  const cmp_u8 invalid_surrogate_relaxed[] = {0xEDu, 0xA0u, 0x80u};
  const cmp_u8 invalid_too_large_relaxed[] = {0xF4u, 0x90u, 0x80u, 0x80u};

  CMP_TEST_EXPECT(cmp_utf8_validate(NULL, 0, &valid), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_utf8_validate(ascii, 0, NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_utf8_validate(ascii, 0, &valid));
  CMP_TEST_ASSERT(valid == CMP_TRUE);

  CMP_TEST_OK(cmp_utf8_validate(ascii, strlen(ascii), &valid));
  CMP_TEST_ASSERT(valid == CMP_TRUE);

  CMP_TEST_OK(cmp_utf8_validate(two_byte, 2, &valid));
  CMP_TEST_ASSERT(valid == CMP_TRUE);

  CMP_TEST_OK(cmp_utf8_validate(three_byte, 3, &valid));
  CMP_TEST_ASSERT(valid == CMP_TRUE);

  CMP_TEST_OK(cmp_utf8_validate(four_byte, 4, &valid));
  CMP_TEST_ASSERT(valid == CMP_TRUE);

  CMP_TEST_OK(cmp_utf8_validate(invalid_start, 1, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_OK(cmp_utf8_validate(invalid_overlong, 2, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_OK(cmp_utf8_validate(invalid_two_bad_cont, 2, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_OK(cmp_utf8_validate(invalid_trunc3, 2, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_OK(cmp_utf8_validate(invalid_bad_b2, 3, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_OK(cmp_utf8_validate(invalid_trunc4, 3, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_OK(cmp_utf8_validate(invalid_surrogate, 3, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_OK(cmp_utf8_validate(invalid_too_large, 4, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_OK(cmp_utf8_validate(invalid_four_boundary, 4, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_OK(cmp_utf8_validate(invalid_high_start, 4, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_OK(cmp_utf8_validate(invalid_bad_b2_4, 4, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_OK(cmp_utf8_validate(invalid_continuation, 3, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  advance = 0;
  CMP_TEST_EXPECT(cmp_utf8_test_decode(NULL, 1, &codepoint, &advance),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_utf8_test_decode((const cmp_u8 *)ascii, 1, NULL, &advance),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_utf8_test_decode((const cmp_u8 *)ascii, 1, &codepoint, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_utf8_test_decode((const cmp_u8 *)ascii, 0, &codepoint, &advance),
      CMP_ERR_NOT_FOUND);
  CMP_TEST_EXPECT(cmp_utf8_test_decode(invalid_trunc2, 1, &codepoint, &advance),
                  CMP_ERR_CORRUPT);
  CMP_TEST_EXPECT(
      cmp_utf8_test_decode(invalid_e0_boundary, 3, &codepoint, &advance),
      CMP_ERR_CORRUPT);

#ifdef CMP_TESTING
  CMP_TEST_OK(cmp_utf8_test_set_relaxed_checks(CMP_TRUE));
  CMP_TEST_EXPECT(
      cmp_utf8_test_decode(invalid_surrogate_relaxed, 3, &codepoint, &advance),
      CMP_ERR_CORRUPT);
  CMP_TEST_EXPECT(
      cmp_utf8_test_decode(invalid_too_large_relaxed, 4, &codepoint, &advance),
      CMP_ERR_CORRUPT);
  CMP_TEST_OK(cmp_utf8_test_set_relaxed_checks(CMP_FALSE));
#endif

  CMP_TEST_EXPECT(cmp_utf8_validate_cstr(NULL, &valid),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_utf8_validate_cstr(ascii, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_utf8_validate_cstr(ascii, &valid));
  CMP_TEST_ASSERT(valid == CMP_TRUE);

  CMP_TEST_OK(cmp_utf8_validate_cstr(invalid_start, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_EXPECT(cmp_utf8_iter_init(NULL, ascii, strlen(ascii)),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_utf8_iter_init(&iter, NULL, 0), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_utf8_iter_init(&iter, ascii, 0));
  CMP_TEST_EXPECT(cmp_utf8_iter_next(&iter, &codepoint), CMP_ERR_NOT_FOUND);

  CMP_TEST_EXPECT(cmp_utf8_iter_init_cstr(NULL, ascii),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_utf8_iter_init_cstr(&iter, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_utf8_iter_init_cstr(&iter, mixed));
  CMP_TEST_OK(cmp_utf8_iter_next(&iter, &codepoint));
  CMP_TEST_ASSERT(codepoint == (cmp_u32)'A');
  CMP_TEST_OK(cmp_utf8_iter_next(&iter, &codepoint));
  CMP_TEST_ASSERT(codepoint == 0x20ACu);
  CMP_TEST_OK(cmp_utf8_iter_next(&iter, &codepoint));
  CMP_TEST_ASSERT(codepoint == 0x1F600u);
  CMP_TEST_EXPECT(cmp_utf8_iter_next(&iter, &codepoint), CMP_ERR_NOT_FOUND);

  CMP_TEST_OK(cmp_utf8_iter_init(&iter, invalid_continuation, 3));
  CMP_TEST_EXPECT(cmp_utf8_iter_next(&iter, &codepoint), CMP_ERR_CORRUPT);
  CMP_TEST_EXPECT(cmp_utf8_iter_next(NULL, &codepoint),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_utf8_iter_next(&iter, NULL), CMP_ERR_INVALID_ARGUMENT);

  max_size = (cmp_usize) ~(cmp_usize)0;
  CMP_TEST_EXPECT(cmp_utf8_test_set_cstr_limit(0), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_utf8_test_set_cstr_limit(2));
  rc = cmp_utf8_validate_cstr("abc", &valid);
  CMP_TEST_ASSERT(rc == CMP_ERR_OVERFLOW);
  rc = cmp_utf8_iter_init_cstr(&iter, "abc");
  CMP_TEST_ASSERT(rc == CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_utf8_test_set_cstr_limit(max_size));

  return 0;
}
