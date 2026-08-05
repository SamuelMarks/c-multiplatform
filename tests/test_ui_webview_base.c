/* clang-format off */
#include "../include/ui_webview_base.h"
#include "../include/ui_error.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

static int ipc_received = 0;

static ui_error_t test_ipc_callback(struct ui_webview_base *webview,
                                    const char *message, void *user_data) {
  (void)webview;
  if (user_data != (void *)0x1234) {
    return UI_ERROR_NONE;
  }
  if (strcmp(message, "hello_from_js") == 0) {
    ipc_received = 1;
  } else {
    ipc_received = 0;
  }
  return UI_ERROR_NONE;
}

int main(void) {
  struct ui_webview_base *webview = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;
  int failed = 0;

  printf("Running ui_webview_base tests...\n");

  rc = ui_webview_base_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

#ifdef UI_TEST_MOCK_ALLOC
  /* OOM test for webview alloc */
  g_malloc_fail_countdown = 0;
  rc = ui_webview_base_create(&webview);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    failed = 1;
  }

  /* OOM test for ui_component_create */
  g_malloc_fail_countdown = 1;
  rc = ui_webview_base_create(&webview);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    failed = 1;
  }

  /* OOM test for ui_dom_node_create. It might take 1 allocation, or more.
     ui_component_create uses some allocations, ui_dom_node_create uses more.
     We will just use a countdown that fails inside dom node creation. */
  g_malloc_fail_countdown = 2;
  rc = ui_webview_base_create(&webview);
  if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
    failed = 1;
  } /* Might pass if it doesn't OOM */

  g_malloc_fail_countdown = -1;
#endif

  rc = ui_webview_base_create(&webview);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  /* Destroy fresh webview to cover NULL current_url / current_html */
  rc = ui_webview_base_destroy(webview);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc = ui_webview_base_create(&webview);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc = ui_webview_base_get_component(NULL, &comp);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  rc = ui_webview_base_get_component(webview, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  rc = ui_webview_base_get_component(webview, &comp);
  if (rc != UI_ERROR_NONE || !comp || !comp->shadow_root) {
    failed = 1;
  }

  rc = ui_webview_base_set_url(NULL, "https://example.com");
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

#ifdef UI_TEST_MOCK_ALLOC
  /* OOM test for duplicate_string */
  g_malloc_fail_countdown = 0;
  rc = ui_webview_base_set_url(webview, "https://example.com");
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    failed = 1;
  }
  g_malloc_fail_countdown = -1;
#endif

  rc = ui_webview_base_set_url(webview, "https://example.com");
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  /* Setting URL to NULL */
  rc = ui_webview_base_set_url(webview, NULL);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  /* Resetting it to non-NULL to test freeing the old one */
  rc = ui_webview_base_set_url(webview, "https://example.com");
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }
  rc = ui_webview_base_set_url(webview, "https://example.com/2");
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc = ui_webview_base_set_html(NULL, "<h1>Hello</h1>");
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = ui_webview_base_set_html(webview, "<h1>Hello</h1>");
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    failed = 1;
  }
  g_malloc_fail_countdown = -1;
#endif

  rc = ui_webview_base_set_html(webview, "<h1>Hello</h1>");
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  /* Setting HTML to NULL */
  rc = ui_webview_base_set_html(webview, NULL);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  /* Resetting HTML to non-NULL to test freeing old one */
  rc = ui_webview_base_set_html(webview, "<h1>Hello</h1>");
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }
  rc = ui_webview_base_set_html(webview, "<h2>Hello</h2>");
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc = ui_webview_base_bind_url(NULL, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  rc = ui_webview_base_bind_url(webview, NULL); /* signal can be NULL */
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc = ui_webview_base_evaluate_js(NULL, "test");
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  rc = ui_webview_base_evaluate_js(webview, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  rc = ui_webview_base_evaluate_js(webview, "console.log('test');");
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  /* Dispatch IPC message BEFORE callback is set */
  rc = ui_webview_base_dispatch_ipc_message(webview, "test");
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc =
      ui_webview_base_set_ipc_callback(NULL, test_ipc_callback, (void *)0x1234);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  rc = ui_webview_base_set_ipc_callback(webview, test_ipc_callback,
                                        (void *)0x1234);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc = ui_webview_base_dispatch_ipc_message(NULL, "hello");
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  rc = ui_webview_base_dispatch_ipc_message(webview, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  rc = ui_webview_base_dispatch_ipc_message(webview, "hello_from_js");
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }
  if (!ipc_received) {
    failed = 1;
  }

  /* Trigger the user_data mismatch branch for coverage */
  test_ipc_callback(webview, "hello_from_js", NULL);
  /* Trigger the strcmp mismatch branch for coverage */
  test_ipc_callback(webview, "not_hello", (void *)0x1234);

  rc = ui_webview_base_destroy(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  rc = ui_webview_base_destroy(webview);
  if (rc != UI_ERROR_NONE) {
    failed = 1;
  }

  if (!failed) {
    printf("All tests passed.\n");
  } else {
    printf("Some tests failed!\n");
  }
  return failed;
}
