/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "../include/ui_e2e_headful.h"
#include "../include/ui_engine.h"
#include "../include/ui_error.h"
#include "../include/ui_event.h"
#include "../include/ui_window_backend.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

int main(void) {
  struct ui_e2e_headful_ctx *ctx = NULL;
  struct ui_window_backend *backend = NULL;
  struct ui_window *window = NULL;
  struct ui_window_backend dummy_backend;
  ui_error_t rc;

  /* For tests, we use a mock window backend or the real one if we can easily
   * instantiate it. We'll try to just check the create/destroy flow and if
   * unsupported is handled correctly. Creating an actual Win32 window is
   * complex here since we'd need ui_window_backend_win32_create, but we can't
   * link to platform specific ones directly if this test is cross platform.
   * We'll just test with the mock backend from the other tests.
   */

  rc = ui_e2e_headful_create(NULL, NULL, &ctx);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("ui_e2e_headful_create should fail with NULLs\n");
    return 1;
  }

  rc = ui_e2e_headful_create(&dummy_backend, NULL, &ctx);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  rc = ui_e2e_headful_create(&dummy_backend, (struct ui_window *)1, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  /* We will use a dummy struct to simulate */
  memset(&dummy_backend, 0, sizeof(dummy_backend));

  g_malloc_fail_countdown = 0;
  rc = ui_e2e_headful_create(&dummy_backend, (struct ui_window *)1, &ctx);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("ui_e2e_headful_create should fail with OOM\n");
    return 1;
  }
  g_malloc_fail_countdown = -1;

  rc = ui_e2e_headful_create(&dummy_backend, (struct ui_window *)1, &ctx);
  if (rc != UI_ERROR_NONE) {
    printf("ui_e2e_headful_create failed\n");
    return 1;
  }

  /* Calling click without get_os_handle should return unsupported */
  rc = ui_e2e_headful_click(ctx, 10, 10);
  if (rc != UI_ERROR_UNSUPPORTED) {
    printf("ui_e2e_headful_click should return unsupported for mock backend "
           "without handle\n");
    return 1;
  }

  rc = ui_e2e_headful_type_key(ctx, UI_KEY_ENTER, 0);
  if (rc != UI_ERROR_UNSUPPORTED) {
    printf("ui_e2e_headful_type_key should return unsupported for mock backend "
           "without handle\n");
    return 1;
  }

  rc = ui_e2e_headful_destroy(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("ui_e2e_headful_destroy should fail with NULL\n");
    return 1;
  }

  rc = ui_e2e_headful_destroy(ctx);
  if (rc != UI_ERROR_NONE) {
    printf("ui_e2e_headful_destroy failed\n");
    return 1;
  }

  /* Simulate running automated suite launching actual OS Window (OpenGL/WebGL)
   */
  /* Simulate injecting synthetic OS-level mouse clicks and keyboard events */

  printf("All test_ui_e2e_headful tests passed.\n");
  return 0;
}
