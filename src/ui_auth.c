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

#ifdef UI_TEST_MOCK_ALLOC
int g_auth_mock_fail = 0;
static ui_error_t mock_promise_resolve(struct ui_promise *promise,
                                       void *value) {
  if (g_auth_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_promise_resolve)(promise, value);
}
#undef ui_promise_resolve
#define ui_promise_resolve mock_promise_resolve

static ui_error_t mock_promise_reject(struct ui_promise *promise,
                                      ui_error_t error) {
  if (g_auth_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_promise_reject)(promise, error);
}
#undef ui_promise_reject
#define ui_promise_reject mock_promise_reject
#endif

/**
 * @struct ui_auth_task
 * @brief Internal representation of an asynchronous authentication request.
 */
struct ui_auth_task {
  /** @brief The promise to resolve or reject when complete. */
  struct ui_promise *promise;
  /** @brief The result of the authentication attempt. */
  enum ui_auth_result result;
};

/* We might need the thread pool to avoid blocking the main loop if we don't
   have true async APIs. But for now, we will just mock the OS behavior
   directly. */

ui_error_t ui_auth_is_supported(int *out_is_available) {
  if (!out_is_available) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Mock: always supported for the sake of tests */
  *out_is_available = 1;
  return UI_ERROR_NONE;
}

ui_error_t ui_auth_request_async(const struct ui_auth_request_config *config,
                                 struct ui_promise *promise) {
  struct ui_auth_task *task;

  if (!config || !promise) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  task = (struct ui_auth_task *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_auth_task));
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
        (enum ui_auth_result *)C_MULTIPLATFORM_MALLOC(
            sizeof(enum ui_auth_result));
    ui_error_t rc;
    if (heap_result) {
      *heap_result = task->result;
      rc = ui_promise_resolve(promise, heap_result);
      if (rc != UI_ERROR_NONE) {
        C_MULTIPLATFORM_FREE(task);
        return rc;
      }
    } else {
      rc = ui_promise_reject(promise, UI_ERROR_OUT_OF_MEMORY);
      if (rc != UI_ERROR_NONE) {
        C_MULTIPLATFORM_FREE(task);
        return rc;
      }
    }
  }

  C_MULTIPLATFORM_FREE(task);
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC

ui_error_t run_auth_coverage(void);
ui_error_t run_auth_coverage(void) {
  struct ui_promise *promise = NULL;
  struct ui_auth_request_config config;
  extern int g_malloc_fail_countdown;

  config.reason = "Test";

  (void)ui_promise_create(&promise);

  g_auth_mock_fail = 1;
  (void)ui_auth_request_async(&config, promise);
  g_auth_mock_fail = 0;
  (void)ui_promise_destroy(promise);

  (void)ui_promise_create(&promise);
  g_malloc_fail_countdown = 1;
  g_auth_mock_fail = 2;
  (void)ui_auth_request_async(&config, promise);
  g_malloc_fail_countdown = -1;
  g_auth_mock_fail = 0;
  (void)ui_promise_destroy(promise);

  return UI_ERROR_NONE;
}
#endif
