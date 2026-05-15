/* clang-format off */
#include "cmp.h"
#include "cmp_audio_capture.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_audio_capture_lifecycle(void) {
  cmp_audio_capture_t *capture = NULL;
  unsigned char *wav = NULL;
  unsigned int size = 0;
  int res;

  res = cmp_audio_capture_create(&capture);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, capture);

  res = cmp_audio_capture_start(capture);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_audio_capture_get_wav(capture, &wav, &size);
  ASSERT_EQ(CMP_ERROR_INVALID_STATE, res); /* Must stop first */

  res = cmp_audio_capture_stop(capture);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_audio_capture_get_wav(capture, &wav, &size);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, wav);
  ASSERT_EQ(1, size > 0);

  res = cmp_audio_capture_free_wav(wav);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_audio_capture_destroy(capture);
  ASSERT_EQ(CMP_SUCCESS, res);

  PASS();
}

TEST test_audio_capture_null_args(void) {
  cmp_audio_capture_t *capture = NULL;
  unsigned char *wav = NULL;
  unsigned int size = 0;
  int res;

  res = cmp_audio_capture_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_audio_capture_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_audio_capture_start(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_audio_capture_stop(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_audio_capture_get_wav(NULL, &wav, &size);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_audio_capture_free_wav(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_audio_capture_create(&capture);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_audio_capture_get_wav(capture, NULL, &size);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_audio_capture_get_wav(capture, &wav, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_audio_capture_stop(capture);
  ASSERT_EQ(CMP_ERROR_INVALID_STATE, res); /* Not recording */

  res = cmp_audio_capture_start(capture);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_audio_capture_start(capture);
  ASSERT_EQ(CMP_ERROR_INVALID_STATE, res); /* Already recording */

  res = cmp_audio_capture_stop(capture);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_audio_capture_destroy(capture);
  ASSERT_EQ(CMP_SUCCESS, res);

  PASS();
}

SUITE(audio_capture_suite) {
  RUN_TEST(test_audio_capture_lifecycle);
  RUN_TEST(test_audio_capture_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(audio_capture_suite);
  GREATEST_MAIN_END();
}
