/* clang-format off */
#include "greatest.h"
#include "../include/ui_video_decoder.h"
#include "../include/ui_error.h"
/* clang-format on */

TEST test_mediacodec_unsupported(void) {
  struct ui_video_decoder_backend backend;
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_video_decoder_mediacodec_get_backend(NULL));
#if defined(__ANDROID__)
  ASSERT_EQ(UI_ERROR_NONE, ui_video_decoder_mediacodec_get_backend(&backend));
#else
  ASSERT_EQ(UI_ERROR_UNSUPPORTED,
            ui_video_decoder_mediacodec_get_backend(&backend));
#endif
  PASS();
}

SUITE(mediacodec_suite) { RUN_TEST(test_mediacodec_unsupported); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(mediacodec_suite);
  GREATEST_MAIN_END();
}
