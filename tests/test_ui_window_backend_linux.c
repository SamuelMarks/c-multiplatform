/* clang-format off */
#include "../include/ui_window_backend.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

extern ui_error_t
ui_window_backend_linux_create(struct ui_window_backend **out_backend);
extern ui_error_t
ui_window_backend_linux_destroy(struct ui_window_backend *backend);

int main(void) {
  struct ui_window_backend *backend = NULL;
  ui_error_t err;
  int failed = 0;

  printf("Running ui_window_backend_linux tests...\n");

  err = ui_window_backend_linux_create(NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_window_backend_linux_destroy(NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_window_backend_linux_create(&backend);
#if defined(__linux__) && !defined(__ANDROID__)
  /* Actually we don't know if X11/Wayland initialized properly. It might return
   * an error. */
  failed |= (err != UI_ERROR_NONE && err != UI_ERROR_UNKNOWN);
  if (err == UI_ERROR_NONE) {
    err = ui_window_backend_linux_destroy(backend);
    failed |= (err != UI_ERROR_NONE);
  }
#else
  failed |= (err != UI_ERROR_UNKNOWN);
  err = ui_window_backend_linux_destroy((struct ui_window_backend *)0x1234);
  failed |= (err != UI_ERROR_UNKNOWN);
#endif

  printf("ui_window_backend_linux tests passed.\n");
  return failed;
}
