/* clang-format off */
#include "../include/ui_window_backend.h"
#include "../include/ui_error.h"
#include <stdio.h>

extern ui_error_t
ui_window_backend_ios_create(struct ui_window_backend **out_backend);
extern ui_error_t
ui_window_backend_ios_destroy(struct ui_window_backend *backend);

int main(void) {
  struct ui_window_backend *backend = NULL;
  ui_error_t err;
  int failed = 0;

  printf("Running ui_window_backend_ios tests...\n");

  err = ui_window_backend_ios_create(NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_window_backend_ios_destroy(NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_window_backend_ios_create(&backend);
#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
/* clang-format on */
#if TARGET_OS_IPHONE
  failed |= (err != UI_ERROR_NONE);
  err = ui_window_backend_ios_destroy(backend);
  failed |= (err != UI_ERROR_NONE);
#else
  failed |= (err != UI_ERROR_UNKNOWN);
  err = ui_window_backend_ios_destroy((struct ui_window_backend *)0x1234);
  failed |= (err != UI_ERROR_UNKNOWN);
#endif
#else
  /* clang-format on */
  failed |= (err != UI_ERROR_UNKNOWN);
  err = ui_window_backend_ios_destroy((struct ui_window_backend *)0x1234);
  failed |= (err != UI_ERROR_UNKNOWN);
#endif

  printf("ui_window_backend_ios tests passed.\n");
  return failed;
}
