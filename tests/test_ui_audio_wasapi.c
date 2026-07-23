/* clang-format off */
#include "../include/ui_audio_sink.h"
#include "../include/ui_error.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

#if defined(_WIN32) || defined(__CYGWIN__)
extern enum ui_error
ui_audio_sink_get_default_backend(struct ui_audio_sink_backend *out_backend);

int main(void) {
  struct ui_audio_sink_backend backend;

  if (ui_audio_sink_get_default_backend(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected UI_ERROR_INVALID_ARGUMENT for NULL backend\n");
    return 1;
  }

  if (ui_audio_sink_get_default_backend(&backend) != UI_ERROR_UNKNOWN) {
    fprintf(stderr, "Expected UI_ERROR_UNKNOWN for WASAPI stub\n");
    return 1;
  }

  printf("test_ui_audio_wasapi passed\n");
  return 0;
}
#else
int main(void) {
  printf("test_ui_audio_wasapi skipped (not WIN32)\n");
  return 0;
}
#endif
