/* clang-format off */
#include "ui_video_decoder.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  struct ui_video_decoder_backend backend;
  ui_error_t err;
  int failed = 0;

  err = ui_video_decoder_get_default_backend(NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_video_decoder_get_default_backend(&backend);

  /* Since the backends are currently stubs, we expect UI_ERROR_UNKNOWN
     until they are fully implemented, or UI_ERROR_NONE if we implemented it. */
  failed |= (err != UI_ERROR_UNKNOWN && err != UI_ERROR_NONE);

  return failed;
}
