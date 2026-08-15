/* clang-format off */
#include "../include/ui_video_decoder.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  struct ui_video_decoder_backend backend;

  if (ui_video_decoder_get_default_backend(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  if (ui_video_decoder_get_default_backend(&backend) != UI_ERROR_UNKNOWN) {
    return 1;
  }

  printf("test_ui_video_ffmpeg passed\n");
  return 0;
}
