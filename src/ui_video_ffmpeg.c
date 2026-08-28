/**
 * @file ui_video_ffmpeg.c
 * @brief ui_video_ffmpeg.c implementation.
 */
/* clang-format off */
#include "ui_video_decoder.h"
#include "ui_error.h"
/* clang-format on */

#if defined(__linux__) && !defined(__ANDROID__) || defined(__EMSCRIPTEN__)
/* FFmpeg fallback implementation stub */

ui_error_t ui_video_decoder_get_default_backend(
    struct ui_video_decoder_backend *out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_backend->create_decoder = NULL;
  out_backend->destroy_decoder = NULL;
  out_backend->decode_packet = NULL;
  out_backend->get_frame = NULL;
  out_backend->release_frame = NULL;
  out_backend->user_data = NULL;

  return UI_ERROR_UNKNOWN; /* TODO: Implement FFmpeg fallback */
}

#endif
