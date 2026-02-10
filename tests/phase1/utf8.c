#include "m3/m3_utf8.h"
#include "test_utils.h"

#include <string.h>

int main(void) {
  M3Utf8Iter iter;
  M3Bool valid;
  m3_u32 codepoint;
  m3_usize max_size;
  m3_usize advance;
  int rc;

  const char ascii[] = "hello";
  const char two_byte[] = {(char)0xC2, (char)0xA2, '\0'};
  const char three_byte[] = {(char)0xE2, (char)0x82, (char)0xAC, '\0'};
  const char four_byte[] = {(char)0xF0, (char)0x9F, (char)0x98, (char)0x80,
                            '\0'};
  const char mixed[] = {'A',        (char)0xE2, (char)0x82,
                        (char)0xAC, (char)0xF0, (char)0x9F,
                        (char)0x98, (char)0x80, '\0'};

  const char invalid_start[] = {(char)0x80, '\0'};
  const char invalid_overlong[] = {(char)0xC0, (char)0x80, '\0'};
  const char invalid_two_bad_cont[] = {(char)0xC2, (char)0x20, '\0'};
  const char invalid_trunc3[] = {(char)0xE2, (char)0x82, '\0'};
  const char invalid_bad_b2[] = {(char)0xE2, (char)0x82, (char)0x28, '\0'};
  const char invalid_trunc4[] = {(char)0xF0, (char)0x9F, (char)0x98, '\0'};
  const char invalid_surrogate[] = {(char)0xED, (char)0xA0, (char)0x80, '\0'};
  const char invalid_too_large[] = {(char)0xF4, (char)0x90, (char)0x80,
                                    (char)0x80, '\0'};
  const char invalid_four_boundary[] = {(char)0xF0, (char)0x80, (char)0x80,
                                        (char)0x80, '\0'};
  const char invalid_high_start[] = {(char)0xF5, (char)0x80, (char)0x80,
                                     (char)0x80, '\0'};
  const char invalid_bad_b2_4[] = {(char)0xF0, (char)0x90, (char)0x28,
                                   (char)0xBC, '\0'};
  const char invalid_continuation[] = {(char)0xE2, (char)0x28, (char)0xA1,
                                       '\0'};
  const m3_u8 invalid_trunc2[] = {0xC2u};
  const m3_u8 invalid_e0_boundary[] = {0xE0u, 0x9Fu, 0x80u};
  const m3_u8 invalid_surrogate_relaxed[] = {0xEDu, 0xA0u, 0x80u};
  const m3_u8 invalid_too_large_relaxed[] = {0xF4u, 0x90u, 0x80u, 0x80u};

  M3_TEST_EXPECT(m3_utf8_validate(NULL, 0, &valid), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_utf8_validate(ascii, 0, NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_utf8_validate(ascii, 0, &valid));
  M3_TEST_ASSERT(valid == M3_TRUE);

  M3_TEST_OK(m3_utf8_validate(ascii, strlen(ascii), &valid));
  M3_TEST_ASSERT(valid == M3_TRUE);

  M3_TEST_OK(m3_utf8_validate(two_byte, 2, &valid));
  M3_TEST_ASSERT(valid == M3_TRUE);

  M3_TEST_OK(m3_utf8_validate(three_byte, 3, &valid));
  M3_TEST_ASSERT(valid == M3_TRUE);

  M3_TEST_OK(m3_utf8_validate(four_byte, 4, &valid));
  M3_TEST_ASSERT(valid == M3_TRUE);

  M3_TEST_OK(m3_utf8_validate(invalid_start, 1, &valid));
  M3_TEST_ASSERT(valid == M3_FALSE);

  M3_TEST_OK(m3_utf8_validate(invalid_overlong, 2, &valid));
  M3_TEST_ASSERT(valid == M3_FALSE);

  M3_TEST_OK(m3_utf8_validate(invalid_two_bad_cont, 2, &valid));
  M3_TEST_ASSERT(valid == M3_FALSE);

  M3_TEST_OK(m3_utf8_validate(invalid_trunc3, 2, &valid));
  M3_TEST_ASSERT(valid == M3_FALSE);

  M3_TEST_OK(m3_utf8_validate(invalid_bad_b2, 3, &valid));
  M3_TEST_ASSERT(valid == M3_FALSE);

  M3_TEST_OK(m3_utf8_validate(invalid_trunc4, 3, &valid));
  M3_TEST_ASSERT(valid == M3_FALSE);

  M3_TEST_OK(m3_utf8_validate(invalid_surrogate, 3, &valid));
  M3_TEST_ASSERT(valid == M3_FALSE);

  M3_TEST_OK(m3_utf8_validate(invalid_too_large, 4, &valid));
  M3_TEST_ASSERT(valid == M3_FALSE);

  M3_TEST_OK(m3_utf8_validate(invalid_four_boundary, 4, &valid));
  M3_TEST_ASSERT(valid == M3_FALSE);

  M3_TEST_OK(m3_utf8_validate(invalid_high_start, 4, &valid));
  M3_TEST_ASSERT(valid == M3_FALSE);

  M3_TEST_OK(m3_utf8_validate(invalid_bad_b2_4, 4, &valid));
  M3_TEST_ASSERT(valid == M3_FALSE);

  M3_TEST_OK(m3_utf8_validate(invalid_continuation, 3, &valid));
  M3_TEST_ASSERT(valid == M3_FALSE);

  advance = 0;
  M3_TEST_EXPECT(m3_utf8_test_decode(NULL, 1, &codepoint, &advance),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_utf8_test_decode((const m3_u8 *)ascii, 1, NULL, &advance),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_utf8_test_decode((const m3_u8 *)ascii, 1, &codepoint, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_utf8_test_decode((const m3_u8 *)ascii, 0, &codepoint, &advance),
      M3_ERR_NOT_FOUND);
  M3_TEST_EXPECT(m3_utf8_test_decode(invalid_trunc2, 1, &codepoint, &advance),
                 M3_ERR_CORRUPT);
  M3_TEST_EXPECT(
      m3_utf8_test_decode(invalid_e0_boundary, 3, &codepoint, &advance),
      M3_ERR_CORRUPT);

#ifdef M3_TESTING
  M3_TEST_OK(m3_utf8_test_set_relaxed_checks(M3_TRUE));
  M3_TEST_EXPECT(
      m3_utf8_test_decode(invalid_surrogate_relaxed, 3, &codepoint, &advance),
      M3_ERR_CORRUPT);
  M3_TEST_EXPECT(
      m3_utf8_test_decode(invalid_too_large_relaxed, 4, &codepoint, &advance),
      M3_ERR_CORRUPT);
  M3_TEST_OK(m3_utf8_test_set_relaxed_checks(M3_FALSE));
#endif

  M3_TEST_EXPECT(m3_utf8_validate_cstr(NULL, &valid), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_utf8_validate_cstr(ascii, NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_utf8_validate_cstr(ascii, &valid));
  M3_TEST_ASSERT(valid == M3_TRUE);

  M3_TEST_OK(m3_utf8_validate_cstr(invalid_start, &valid));
  M3_TEST_ASSERT(valid == M3_FALSE);

  M3_TEST_EXPECT(m3_utf8_iter_init(NULL, ascii, strlen(ascii)),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_utf8_iter_init(&iter, NULL, 0), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_utf8_iter_init(&iter, ascii, 0));
  M3_TEST_EXPECT(m3_utf8_iter_next(&iter, &codepoint), M3_ERR_NOT_FOUND);

  M3_TEST_EXPECT(m3_utf8_iter_init_cstr(NULL, ascii), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_utf8_iter_init_cstr(&iter, NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_utf8_iter_init_cstr(&iter, mixed));
  M3_TEST_OK(m3_utf8_iter_next(&iter, &codepoint));
  M3_TEST_ASSERT(codepoint == (m3_u32)'A');
  M3_TEST_OK(m3_utf8_iter_next(&iter, &codepoint));
  M3_TEST_ASSERT(codepoint == 0x20ACu);
  M3_TEST_OK(m3_utf8_iter_next(&iter, &codepoint));
  M3_TEST_ASSERT(codepoint == 0x1F600u);
  M3_TEST_EXPECT(m3_utf8_iter_next(&iter, &codepoint), M3_ERR_NOT_FOUND);

  M3_TEST_OK(m3_utf8_iter_init(&iter, invalid_continuation, 3));
  M3_TEST_EXPECT(m3_utf8_iter_next(&iter, &codepoint), M3_ERR_CORRUPT);
  M3_TEST_EXPECT(m3_utf8_iter_next(NULL, &codepoint), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_utf8_iter_next(&iter, NULL), M3_ERR_INVALID_ARGUMENT);

  max_size = (m3_usize) ~(m3_usize)0;
  M3_TEST_EXPECT(m3_utf8_test_set_cstr_limit(0), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_utf8_test_set_cstr_limit(2));
  rc = m3_utf8_validate_cstr("abc", &valid);
  M3_TEST_ASSERT(rc == M3_ERR_OVERFLOW);
  rc = m3_utf8_iter_init_cstr(&iter, "abc");
  M3_TEST_ASSERT(rc == M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_utf8_test_set_cstr_limit(max_size));

  return 0;
}
