/* clang-format off */
#include "../include/ui_window_backend_linux.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
extern ui_error_t
ui_window_backend_linux_x11_create(struct ui_window_backend **out_backend);

static int test_x11(void) {
  struct ui_window_backend *backend = NULL;
  int failed = 0;

  if (ui_window_backend_linux_x11_create(&backend) == UI_ERROR_NONE) {
    struct ui_window *win = NULL;
    backend->create_window(backend, "test", 800, 600, &win);
    ui_window_backend_linux_x11_destroy(backend);
  }

  failed |=
      (ui_window_backend_linux_x11_create(NULL) != UI_ERROR_INVALID_ARGUMENT);

#if defined(__linux__) || defined(__FreeBSD__)
  g_malloc_fail_countdown = 0;
  failed |=
      (ui_window_backend_linux_x11_create(&backend) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
#endif

  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_x11();
  if (failed) {
    fprintf(stderr, "test_ui_window_backend_linux_x11 failed\n");
  } else {
    printf("test_ui_window_backend_linux_x11 passed\n");
  }
  return failed;
}
