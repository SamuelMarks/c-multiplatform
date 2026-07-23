/* clang-format off */
#include "ui_audio_sink.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  struct ui_audio_sink_backend backend;
  enum ui_error err;

  err = ui_audio_sink_get_default_backend(NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  err = ui_audio_sink_get_default_backend(&backend);

  /* Since the backends are currently stubs, we expect UI_ERROR_UNKNOWN
     until they are fully implemented, or UI_ERROR_NONE if we implemented it. */
  if (err != UI_ERROR_UNKNOWN && err != UI_ERROR_NONE) {
    return 1;
  }

  return 0;
}
