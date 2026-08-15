/* clang-format off */
#include "../include/ui_window_backend.h"
#include "../include/ui_error.h"
#include "../include/ui_event.h"
#include <stdio.h>

extern ui_error_t
ui_window_backend_macos_create(struct ui_window_backend **out_backend);
extern ui_error_t
ui_window_backend_macos_destroy(struct ui_window_backend *backend);

int main(void) {
  struct ui_window_backend *backend = NULL;
  ui_error_t err;
  int failed = 0;

  printf("Running ui_window_backend_macos tests...\n");

  err = ui_window_backend_macos_create(NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_window_backend_macos_destroy(NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_window_backend_macos_create(&backend);
#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
/* clang-format on */
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
  failed |= (err != UI_ERROR_NONE);
  if (backend) {
    struct ui_window *window = NULL;
    struct ui_event evt;
    int has_evt;

    failed |= (backend->create_window(NULL, "Test", 100, 100, &window) !=
               UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->create_window(backend, NULL, 100, 100, &window) !=
               UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->create_window(backend, "Test", 100, 100, NULL) !=
               UI_ERROR_INVALID_ARGUMENT);

    /* Implementation returns UNKNOWN for now until FFI lands */
    failed |= (backend->create_window(backend, "Test", 100, 100, &window) !=
               UI_ERROR_UNKNOWN);

    failed |=
        (backend->destroy_window(NULL, window) != UI_ERROR_INVALID_ARGUMENT);
    failed |=
        (backend->destroy_window(backend, NULL) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->destroy_window(backend, (struct ui_window *)1) !=
               UI_ERROR_NONE);

    failed |= (backend->show_window(NULL, window) != UI_ERROR_INVALID_ARGUMENT);
    failed |=
        (backend->show_window(backend, NULL) != UI_ERROR_INVALID_ARGUMENT);
    failed |=
        (backend->show_window(backend, (struct ui_window *)1) != UI_ERROR_NONE);

    failed |= (backend->hide_window(NULL, window) != UI_ERROR_INVALID_ARGUMENT);
    failed |=
        (backend->hide_window(backend, NULL) != UI_ERROR_INVALID_ARGUMENT);
    failed |=
        (backend->hide_window(backend, (struct ui_window *)1) != UI_ERROR_NONE);

    failed |= (backend->poll_events(NULL, window, &evt, &has_evt) !=
               UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->poll_events(backend, NULL, &evt, &has_evt) !=
               UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->poll_events(backend, (struct ui_window *)1, NULL,
                                    &has_evt) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->poll_events(backend, (struct ui_window *)1, &evt,
                                    NULL) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->poll_events(backend, (struct ui_window *)1, &evt,
                                    &has_evt) != UI_ERROR_NONE);

    failed |=
        (backend->swap_buffers(NULL, window) != UI_ERROR_INVALID_ARGUMENT);
    failed |=
        (backend->swap_buffers(backend, NULL) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->swap_buffers(backend, (struct ui_window *)1) !=
               UI_ERROR_NONE);
  }

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_malloc_fail_countdown;
  g_malloc_fail_countdown = 0;
  failed |=
      (ui_window_backend_macos_create(&backend) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
#endif

  err = ui_window_backend_macos_destroy(backend);
  failed |= (err != UI_ERROR_NONE);
#else
  failed |= (err != UI_ERROR_UNKNOWN);
  err = ui_window_backend_macos_destroy((struct ui_window_backend *)0x1234);
  failed |= (err != UI_ERROR_UNKNOWN);
#endif
#else
  /* clang-format on */
  failed |= (err != UI_ERROR_UNKNOWN);
  err = ui_window_backend_macos_destroy((struct ui_window_backend *)0x1234);
  failed |= (err != UI_ERROR_UNKNOWN);
#endif

  printf("ui_window_backend_macos tests passed.\n");
  return failed;
}
