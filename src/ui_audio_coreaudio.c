/* clang-format off */
#include "ui_audio_sink.h"
#include "ui_error.h"
/* clang-format on */

#if defined(__APPLE__)
/* CoreAudio implementation stub */

enum ui_error
ui_audio_sink_get_default_backend(struct ui_audio_sink_backend *out_backend) {
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

  return UI_ERROR_UNKNOWN; /* TODO: Implement CoreAudio */
}

#endif
