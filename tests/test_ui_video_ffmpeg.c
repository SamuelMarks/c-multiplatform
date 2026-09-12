/* clang-format off */
#include "../include/ui_video_decoder.h"
#include "../include/ui_error.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

int main(void) {
  struct ui_video_decoder_backend backend;

  if (ui_video_decoder_ffmpeg_get_backend(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected UI_ERROR_INVALID_ARGUMENT for NULL backend\n");
    return 1;
  }

  if (ui_video_decoder_ffmpeg_get_backend(&backend) != UI_ERROR_UNKNOWN) {
    fprintf(stderr, "Expected UI_ERROR_UNKNOWN for FFmpeg stub\n");
    return 1;
  }

#if (defined(__linux__) && !defined(__ANDROID__)) || defined(__EMSCRIPTEN__)
  if (ui_video_decoder_get_default_backend(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr,
            "Expected UI_ERROR_INVALID_ARGUMENT for NULL default backend\n");
    return 1;
  }

  if (ui_video_decoder_get_default_backend(&backend) != UI_ERROR_UNKNOWN) {
    fprintf(stderr, "Expected UI_ERROR_UNKNOWN for default FFmpeg stub\n");
    return 1;
  }
#endif

  printf("test_ui_video_ffmpeg passed\n");
  return 0;
}
