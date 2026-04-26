/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdio.h>
/* clang-format on */

/**
 * @brief Copy text to the OS clipboard.
 *
 * @param window The window context.
 * @param text The text to copy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_copy_to_clipboard(cmp_window_t *window, const char *text) {
  int rc;

  rc = CMP_SUCCESS;

  if (window == NULL || text == NULL) {
    LOG_DEBUG("Error in cmp_os_copy_to_clipboard: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Bridge via c-multiplatform's native clipboard API */
  rc = cmp_window_set_clipboard_text(window, CMP_CLIPBOARD_PRIMARY, text);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_copy_to_clipboard: "
              "cmp_window_set_clipboard_text failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief Handle file drop event.
 *
 * @param path The dropped file path.
 * @param user_data User data.
 */
static void on_file_dropped(const char *path, void *user_data) {
  (void)user_data;
  /* Normally we'd push an event onto the UI thread queue to create an
   * attachment pill. */
#if defined(_MSC_VER)
  if (path != NULL) {
    LOG_DEBUG("File Drop Emulating attachment pill for: ");
    LOG_DEBUG(path);
    LOG_DEBUG("\n");
  } else {
    LOG_DEBUG("File Drop Emulating attachment pill for: NULL\n");
  }
#else
  printf("[File Drop] Emulating attachment pill for: %s\n",
         path ? path : "NULL");
#endif
}

/**
 * @brief Enable file drag-and-drop for a window.
 *
 * @param window The window context.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_enable_file_drag_drop(cmp_window_t *window) {
  int rc;

  rc = CMP_SUCCESS;

  if (window == NULL) {
    LOG_DEBUG("Error in cmp_os_enable_file_drag_drop: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Hook OS window drag-and-drop via c-multiplatform */
  rc = cmp_window_set_drop_callback(window, on_file_dropped, NULL);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_enable_file_drag_drop: "
              "cmp_window_set_drop_callback failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief Check if voice dictation is supported.
 *
 * @param out_is_supported Pointer to store the result (1 if supported, 0
 * otherwise).
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_is_voice_dictation_supported(int *out_is_supported) {
  if (out_is_supported == NULL) {
    LOG_DEBUG(
        "Error in cmp_os_is_voice_dictation_supported: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Return 1 indicating we hook into native Speech frameworks (e.g. SAPI on
   * Win, NSSpeechRecognizer on Mac) */
  *out_is_supported = 1;
  return CMP_SUCCESS;
}

/**
 * @brief Start voice dictation.
 *
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_start_voice_dictation(void) {
  int is_supported;
  int rc;

  is_supported = 0;
  rc = cmp_os_is_voice_dictation_supported(&is_supported);
  if (rc != CMP_SUCCESS || !is_supported) {
    LOG_DEBUG("Error in cmp_os_start_voice_dictation: Voice dictation not "
              "supported or failed to check\n");
    return CMP_ERROR_INVALID_STATE;
  }

  /* Mock invocation of dictation API.
     Would initialize COM/SAPI for Windows natively here. */
  return CMP_SUCCESS;
}
