/* clang-format off */
#include "../include/ui_auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

/* Mock free from standard library for test purposes since mock allocator isn't
 * exposed directly via a shared header in tests in the same way */
extern void ui_mock_free(void *ptr);

static int g_auth_success = 0;

static enum ui_error on_auth_resolved(void *result_ptr, void *user_data,
                                      void **out_result) {
  enum ui_auth_result *res = (enum ui_auth_result *)result_ptr;
  (void)user_data;
  if (res && *res == UI_AUTH_RESULT_SUCCESS) {
    g_auth_success = 1;
  }
  if (res) {
    ui_mock_free(res); /* Free the dynamically allocated result from the mock */
  }
  return UI_ERROR_NONE;
}

static enum ui_error on_auth_rejected(enum ui_error err, void *user_data,
                                      void **out_result) {
  (void)err;
  (void)user_data;
  fprintf(stderr, "Auth promise rejected\n");
  return UI_ERROR_NONE;
}

int main(void) {
  struct ui_promise *promise = NULL;
  struct ui_auth_request_config config;
  enum ui_error rc;
  int is_supported = 0;
  int i;

  /* Invalid arguments */
  assert(ui_auth_is_supported(NULL) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_auth_request_async(NULL, promise) == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_auth_is_supported(&is_supported);
  if (rc != UI_ERROR_NONE || !is_supported) {
    fprintf(stderr, "ui_auth_is_supported failed\n");
    return 1;
  }

  rc = ui_promise_create(&promise);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_promise_create failed\n");
    return 1;
  }

  assert(ui_auth_request_async(&config, NULL) == UI_ERROR_INVALID_ARGUMENT);

  config.reason = "Test Authentication";
  config.cancel_title = "Cancel";
  config.allow_device_credential = 1;

  /* OOM loops */
  for (i = 0; i < 2; i++) {
    struct ui_promise *p = NULL;
    ui_promise_create(&p);
    g_malloc_fail_countdown = i;
    rc = ui_auth_request_async(&config, p);
    if (rc == UI_ERROR_NONE) {
      /* If the second allocation fails, it rejects the promise and still
       * returns NONE */
    }
    g_malloc_fail_countdown = -1;
    ui_promise_destroy(p);
  }

  rc = ui_promise_then(promise, on_auth_resolved, on_auth_rejected, NULL, NULL);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_promise_then failed\n");
    return 1;
  }

  rc = ui_auth_request_async(&config, promise);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_auth_request_async failed\n");
    return 1;
  }

  if (!g_auth_success) {
    fprintf(stderr, "Auth did not succeed\n");
    return 1;
  }

  ui_promise_destroy(promise);

  printf("test_ui_auth passed\n");
  return 0;
}
