/**
 * @file ui_video_ffmpeg.c
 * @brief ui_video_ffmpeg.c implementation.
 */
/* clang-format off */
#include "ui_video_decoder.h"
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Retrieves the FFmpeg hardware video decoder backend.
 *
 * @param out_backend Pointer to the backend structure to populate.
 * @return ui_error_t `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_video_decoder_ffmpeg_get_backend(
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

#if (defined(__linux__) && !defined(__ANDROID__)) || defined(__EMSCRIPTEN__)
/**
 * @brief Retrieves the default video decoder backend on Linux / Emscripten.
 *
 * @param out_backend Pointer to the backend structure to populate.
 * @return ui_error_t `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_video_decoder_get_default_backend(
    struct ui_video_decoder_backend *out_backend) {
  return ui_video_decoder_ffmpeg_get_backend(out_backend);
}
#endif
