/**
 * @file ui_audio_webaudio.c
 * @brief WebAudio implementation stub for audio sink.
 */

/* clang-format off */
#include "ui_audio_sink.h"
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Retrieves the default WebAudio audio sink backend.
 *
 * @param out_backend Pointer to the backend structure to populate.
 * @return ui_error_t `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t
ui_audio_sink_webaudio_get_backend(struct ui_audio_sink_backend *out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_backend->create_sink = NULL;
  out_backend->destroy_sink = NULL;
  out_backend->write_frames = NULL;
  out_backend->get_delay = NULL;
  out_backend->start = NULL;
  out_backend->stop = NULL;
  out_backend->user_data = NULL;

  return UI_ERROR_UNKNOWN; /* TODO: Implement WebAudio */
}

#if defined(__EMSCRIPTEN__)
/**
 * @brief Retrieves the default audio sink backend on Web/Emscripten.
 *
 * @param out_backend Pointer to the backend structure to populate.
 * @return ui_error_t `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t
ui_audio_sink_get_default_backend(struct ui_audio_sink_backend *out_backend) {
  return ui_audio_sink_webaudio_get_backend(out_backend);
}
#endif
