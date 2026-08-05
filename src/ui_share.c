/* clang-format off */
#include "../include/ui_share.h"
#include "ui_internal_mem.h"
#include <string.h>

#if defined(_WIN32)
#ifndef UI_WINAPI
#if defined(_MSC_VER)
#define UI_WINAPI __stdcall
#elif defined(__GNUC__)
#define UI_WINAPI __attribute__((stdcall))
#else
#define UI_WINAPI
#endif
#endif
/* Forward declarations for Windows DataTransferManager if needed */
#endif
/* clang-format on */

struct ui_share_task {
  struct ui_promise *promise;
  struct ui_share_payload payload_copy;
};

ui_error_t ui_share_is_supported(int *out_is_available) {
  if (!out_is_available) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Mock: always supported for the sake of tests */
  *out_is_available = 1;
  return UI_ERROR_NONE;
}

ui_error_t ui_share_request_async(const struct ui_share_payload *payload,
                                  struct ui_promise *promise) {
  struct ui_share_task *task;

  if (!payload || !promise) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  task = (struct ui_share_task *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_share_task));
  if (!task) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  task->promise = promise;

  /* In a real implementation we would make deep copies of the strings,
     then dispatch to UIActivityViewController (iOS), NSSharingService (macOS),
     ACTION_SEND (Android), or DataTransferManager (Windows).
     For the mock, we instantly resolve.
  */

  ui_promise_resolve(promise, NULL);

  C_MULTIPLATFORM_FREE(task);
  return UI_ERROR_NONE;
}
