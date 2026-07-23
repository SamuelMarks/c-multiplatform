/* clang-format off */
#include "ui_video_decoder.h"
#include "ui_error.h"
/* clang-format on */

#if defined(_WIN32) || defined(__CYGWIN__)
/* MediaFoundation implementation stub */

enum ui_error ui_video_decoder_get_default_backend(
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

  return UI_ERROR_UNKNOWN; /* TODO: Implement MediaFoundation */
}

#endif
