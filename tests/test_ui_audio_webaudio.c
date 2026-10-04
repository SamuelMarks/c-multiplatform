/* clang-format off */
#include "greatest.h"
#include "../include/ui_audio_sink.h"
#include "../include/ui_error.h"
/* clang-format on */

TEST test_webaudio_unsupported(void) {
  struct ui_audio_sink_backend backend;
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_audio_sink_webaudio_get_backend(NULL));
#if defined(__EMSCRIPTEN__)
  ASSERT_EQ(UI_ERROR_NONE, ui_audio_sink_webaudio_get_backend(&backend));
#else
  ASSERT_EQ(UI_ERROR_UNSUPPORTED, ui_audio_sink_webaudio_get_backend(&backend));
#endif
  PASS();
}

SUITE(webaudio_suite) { RUN_TEST(test_webaudio_unsupported); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(webaudio_suite);
  GREATEST_MAIN_END();
}
