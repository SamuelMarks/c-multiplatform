/* clang-format off */
#include "../include/ui_window_backend.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

extern enum ui_error
ui_window_backend_android_create(struct ui_window_backend **out_backend);
extern enum ui_error
ui_window_backend_android_destroy(struct ui_window_backend *backend);

int main(void) {
  struct ui_window_backend *backend = NULL;
  enum ui_error err;
  int failed = 0;

  printf("Running ui_window_backend_android tests...\n");

  err = ui_window_backend_android_create(NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_window_backend_android_destroy(NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_window_backend_android_create(&backend);
#if defined(__ANDROID__)
  failed |= (err != UI_ERROR_NONE);
  err = ui_window_backend_android_destroy(backend);
  failed |= (err != UI_ERROR_NONE);
#else
  failed |= (err != UI_ERROR_UNKNOWN);
  err = ui_window_backend_android_destroy((struct ui_window_backend *)0x1234);
  failed |= (err != UI_ERROR_UNKNOWN);
#endif

  printf("ui_window_backend_android tests passed.\n");
  return failed;
}
