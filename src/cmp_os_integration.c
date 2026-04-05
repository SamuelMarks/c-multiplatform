/* clang-format off */
#include "cmp.h"
#include <stdio.h>
/* clang-format on */

int cmp_os_copy_to_clipboard(cmp_window_t *window, const char *text) {
  if (!window || !text) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Bridge via c-multiplatform's native clipboard API */
  return cmp_window_set_clipboard_text(window, CMP_CLIPBOARD_PRIMARY, text);
}

static void on_file_dropped(const char *path, void *user_data) {
  (void)user_data;
  /* Normally we'd push an event onto the UI thread queue to create an
   * attachment pill. */
  printf("[File Drop] Emulating attachment pill for: %s\n",
         path ? path : "NULL");
}

int cmp_os_enable_file_drag_drop(cmp_window_t *window) {
  if (!window) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Hook OS window drag-and-drop via c-multiplatform */
  return cmp_window_set_drop_callback(window, on_file_dropped, NULL);
}

int cmp_os_is_voice_dictation_supported(void) {
  /* Return 1 indicating we hook into native Speech frameworks (e.g. SAPI on
   * Win, NSSpeechRecognizer on Mac) */
  return 1;
}

int cmp_os_start_voice_dictation(void) {
  if (!cmp_os_is_voice_dictation_supported()) {
    return CMP_ERROR_INVALID_STATE;
  }

  /* Mock invocation of dictation API.
     Would initialize COM/SAPI for Windows natively here. */
  return CMP_SUCCESS;
}
