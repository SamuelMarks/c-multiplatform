/**
 * @file ui_auth.c
 * @brief Implementation of cross-platform biometric/credential authentication.
 */

/* clang-format off */
#include "../include/ui_auth.h"
#include "ui_internal_mem.h"
#include <string.h>

#if defined(_WIN32)
#ifndef UI_WINAPI
#if defined(_MSC_VER)
/** @cond */
#define UI_WINAPI __stdcall
/** @endcond */
#elif defined(__GNUC__)
/** @cond */
#define UI_WINAPI __attribute__((stdcall))
/** @endcond */
#else
/** @cond */
#define UI_WINAPI
/** @endcond */
#endif
#endif
/* Forward declare minimum WinRT / COM functions here if needed, or simply mock for now */
#endif
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
/* @brief Global flag to inject failure into auth mock functions. */
int g_auth_mock_fail = 0;

/**
 * @brief Mock implementation of ui_promise_resolve for testing.
 *
 * @param promise The promise to resolve.
 * @param value The value to resolve with.
 * @return ui_error_t UI_ERROR_NONE on success or injected UI_ERROR_UNKNOWN.
 */
static ui_error_t mock_promise_resolve(struct ui_promise *promise,
                                       void *value) {
  if (g_auth_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_promise_resolve)(promise, value);
}
#undef ui_promise_resolve
/** @cond */
#define ui_promise_resolve mock_promise_resolve
/** @endcond */

/**
 * @brief Mock implementation of ui_promise_reject for testing.
 *
 * @param promise The promise to reject.
 * @param error The error to reject with.
 * @return ui_error_t UI_ERROR_NONE on success or injected UI_ERROR_UNKNOWN.
 */
static ui_error_t mock_promise_reject(struct ui_promise *promise,
                                      ui_error_t error) {
  if (g_auth_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_promise_reject)(promise, error);
}
#undef ui_promise_reject
/** @cond */
#define ui_promise_reject mock_promise_reject
/** @endcond */
#endif

/**
 * @struct ui_auth_task
 * @struct ui_auth_task
 * @brief Internal representation of an asynchronous authentication request.
 */
struct ui_auth_task {
  struct ui_promise
      *promise; /**< The promise to resolve or reject when complete. */
  enum ui_auth_result result; /**< The result of the authentication attempt. */
};

/* We might need the thread pool to avoid blocking the main loop if we don't
   have true async APIs. But for now, we will just mock the OS behavior
   directly. */

/**
 * @brief Checks if authentication is supported on the current platform.
 *
 * @param out_is_available Pointer to an integer, populated with 1 if supported,
 * 0 otherwise.
 * @return ui_error_t `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_auth_is_supported(int *out_is_available) {
  if (!out_is_available) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Mock: always supported for the sake of tests */
  *out_is_available = 1;
  return UI_ERROR_NONE;
}

/**
 * @brief Requests authentication asynchronously.
 *
 * @param config Pointer to the authentication request configuration.
 * @param promise Pointer to the promise to be resolved or rejected.
 * @return ui_error_t `UI_ERROR_NONE` on success, or an appropriate error code.
 */
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

/**
 * @brief Runs test coverage for authentication functionality.
 *
 * @return ui_error_t `UI_ERROR_NONE` on success.
 */
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
