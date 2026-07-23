/* clang-format off */
#include "../include/ui_auth.h"
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
/* Forward declare minimum WinRT / COM functions here if needed, or simply mock for now */
#endif
/* clang-format on */

struct ui_auth_task {
  struct ui_promise *promise;
  enum ui_auth_result result;
};

/* We might need the thread pool to avoid blocking the main loop if we don't
   have true async APIs. But for now, we will just mock the OS behavior
   directly. */

enum ui_error ui_auth_is_supported(int *out_is_available) {
  if (!out_is_available) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Mock: always supported for the sake of tests */
  *out_is_available = 1;
  return UI_ERROR_NONE;
}

enum ui_error ui_auth_request_async(const struct ui_auth_request_config *config,
                                    struct ui_promise *promise) {
  struct ui_auth_task *task;

  if (!config || !promise) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  task = (struct ui_auth_task *)UI_MALLOC(sizeof(struct ui_auth_task));
  if (!task) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  task->promise = promise;

  /* Mock logic: instantly resolve with SUCCESS */
  /* In a real implementation, we would dispatch a background task calling
     UserConsentVerifier on WinRT, LAContext on macOS/iOS, or BiometricPrompt on
     Android. */
  task->result = UI_AUTH_RESULT_SUCCESS;

  /* Note: We are resolving directly here for the mock, allocating memory for
     the result since promise_resolve takes a void*. We should probably
     dynamically allocate the result. */
  {
    enum ui_auth_result *heap_result =
        (enum ui_auth_result *)UI_MALLOC(sizeof(enum ui_auth_result));
    if (heap_result) {
      *heap_result = task->result;
      ui_promise_resolve(promise, heap_result);
    } else {
      ui_promise_reject(promise, UI_ERROR_OUT_OF_MEMORY);
    }
  }

  UI_FREE(task);
  return UI_ERROR_NONE;
}
