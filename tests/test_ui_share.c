/* clang-format off */
#include "../include/ui_share.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int g_share_success = 0;

static enum ui_error on_share_resolved(void *result_ptr, void *user_data,
                                       void **out_result) {
  (void)result_ptr;
  (void)user_data;
  g_share_success = 1;
  return UI_ERROR_NONE;
}

static enum ui_error on_share_rejected(enum ui_error err, void *user_data,
                                       void **out_result) {
  (void)err;
  (void)user_data;
  fprintf(stderr, "Share promise rejected\n");
  return UI_ERROR_NONE;
}

int main(void) {
  struct ui_promise *promise = NULL;
  struct ui_share_payload payload;
  enum ui_error rc;
  int is_supported = 0;

  rc = ui_share_is_supported(&is_supported);
  if (rc != UI_ERROR_NONE || !is_supported) {
    fprintf(stderr, "ui_share_is_supported failed\n");
    return 1;
  }

  rc = ui_share_is_supported(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr,
            "ui_share_is_supported(NULL) failed to return INVALID_ARGUMENT\n");
    return 1;
  }

  rc = ui_promise_create(&promise);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_promise_create failed\n");
    return 1;
  }

  payload.title = "Test Title";
  payload.text = "Test description body.";
  payload.url = "https://example.com";

  rc = ui_promise_then(promise, on_share_resolved, on_share_rejected, NULL,
                       NULL);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_promise_then failed\n");
    return 1;
  }

  /* Test NULL payload */
  rc = ui_share_request_async(NULL, promise);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_share_request_async(NULL, promise) failed to return "
                    "INVALID_ARGUMENT\n");
    return 1;
  }

  /* Test NULL promise */
  rc = ui_share_request_async(&payload, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_share_request_async(payload, NULL) failed to return "
                    "INVALID_ARGUMENT\n");
    return 1;
  }

  /* Test malloc failure */
  g_malloc_fail_countdown = 0;
  rc = ui_share_request_async(&payload, promise);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    fprintf(stderr, "ui_share_request_async out of memory test failed\n");
    return 1;
  }

  /* Test successful execution */
  rc = ui_share_request_async(&payload, promise);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_share_request_async failed\n");
    return 1;
  }

  if (!g_share_success) {
    fprintf(stderr, "Share did not succeed\n");
    return 1;
  }

  ui_promise_destroy(promise);

  printf("test_ui_share passed\n");
  return 0;
}
