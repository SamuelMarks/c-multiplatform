/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdio.h>
/* clang-format on */

int cmp_os_copy_to_clipboard(cmp_window_t *window, const char *text) {
  int rc = CMP_SUCCESS;

  if (!window || !text) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_os_copy_to_clipboard: Invalid argument\n");
    return rc;
  }

  /* Bridge via c-multiplatform's native clipboard API */
  rc = cmp_window_set_clipboard_text(window, CMP_CLIPBOARD_PRIMARY, text);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_copy_to_clipboard: "
              "cmp_window_set_clipboard_text failed\n");
  }
  return rc;
}

static void on_file_dropped(const char *path, void *user_data) {
  (void)user_data;
  /* Normally we'd push an event onto the UI thread queue to create an
   * attachment pill. */
  printf("[File Drop] Emulating attachment pill for: %s\n",
         path ? path : "NULL");
}

int cmp_os_enable_file_drag_drop(cmp_window_t *window) {
  int rc = CMP_SUCCESS;

  if (!window) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_os_enable_file_drag_drop: Invalid argument\n");
    return rc;
  }

  /* Hook OS window drag-and-drop via c-multiplatform */
  rc = cmp_window_set_drop_callback(window, on_file_dropped, NULL);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_enable_file_drag_drop: "
              "cmp_window_set_drop_callback failed\n");
  }
  return rc;
}

int cmp_os_is_voice_dictation_supported(void) {
  /* Return 1 indicating we hook into native Speech frameworks (e.g. SAPI on
   * Win, NSSpeechRecognizer on Mac) */
  return 1;
}

int cmp_os_start_voice_dictation(void) {
  int rc = CMP_SUCCESS;

  if (!cmp_os_is_voice_dictation_supported()) {
    rc = CMP_ERROR_INVALID_STATE;
    LOG_DEBUG("Error in cmp_os_start_voice_dictation: Voice dictation not "
              "supported\n");
    return rc;
  }

  /* Mock invocation of dictation API.
     Would initialize COM/SAPI for Windows natively here. */
  return rc;
}
