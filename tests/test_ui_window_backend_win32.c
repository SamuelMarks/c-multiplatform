/* clang-format off */
#include "../include/ui_window_backend_win32.h"
#include "../include/ui_event.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

int main(void) {
  struct ui_window_backend *backend = NULL;
  ui_error_t rc;
  int failed = 0;

  printf("Running ui_window_backend_win32 tests...\n");

  /* Test invalid argument */
  rc = ui_window_backend_win32_create(NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_window_backend_win32_destroy(NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_window_backend_win32_create(&backend);
#ifdef _WIN32
  failed |= (rc != UI_ERROR_NONE);

  if (backend != NULL) {
    struct ui_window *win = NULL;
    struct ui_event event;
    int has_event;

    printf("Testing null args...\n");
    failed |= (backend->create_window(NULL, "Test", 100, 100, &win) !=
               UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->destroy_window(NULL, win) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->show_window(NULL, win) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->hide_window(NULL, win) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->poll_events(NULL, win, &event, &has_event) !=
               UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->swap_buffers(NULL, win) != UI_ERROR_INVALID_ARGUMENT);

    printf("Creating window...\n");
    {
      int is_wine = 0;
#if defined(_MSC_VER) && _MSC_VER >= 1400
      char *wine_env = NULL;
      size_t wine_len = 0;
      if (_dupenv_s(&wine_env, &wine_len, "WINELOADER") == 0 &&
          wine_env != NULL) {
        is_wine = 1;
        free(wine_env);
      }
#else
      if (getenv("WINELOADER") != NULL) {
        is_wine = 1;
      }
#endif
      if (is_wine) {
        printf("Skipping actual window creation under Wine CI.\n");
        rc = ui_window_backend_win32_destroy(backend);
        failed |= (rc != UI_ERROR_NONE);
        return failed;
      }
    }
    rc = backend->create_window(backend, "Test Window", 800, 600, &win);
    failed |= (rc != UI_ERROR_NONE);

    rc = backend->show_window(backend, win);
    failed |= (rc != UI_ERROR_NONE);

    rc = backend->poll_events(backend, win, &event, &has_event);
    failed |= (rc != UI_ERROR_NONE);

    rc = backend->swap_buffers(backend, win);
    failed |= (rc != UI_ERROR_NONE);

    if (backend->get_os_handle) {
      void *handle = NULL;
      ui_error_t handle_rc = backend->get_os_handle(backend, win, &handle);
      failed |= (handle_rc != UI_ERROR_NONE);
      failed |= (handle == NULL);
    }

    rc = backend->hide_window(backend, win);
    failed |= (rc != UI_ERROR_NONE);

    rc = backend->destroy_window(backend, win);
    failed |= (rc != UI_ERROR_NONE);

    rc = ui_window_backend_win32_destroy(backend);
    failed |= (rc != UI_ERROR_NONE);
  }
#else
  failed |= (rc != UI_ERROR_UNKNOWN);

  failed |= (ui_window_backend_win32_destroy((struct ui_window_backend *)1) !=
             UI_ERROR_UNKNOWN);
#endif

  printf("All ui_window_backend_win32 tests passed.\n");
  return failed;
}
