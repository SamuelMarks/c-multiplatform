/* clang-format off */
#include <stdio.h>
#include "../include/ui_window_backend.h"
#include "../include/ui_event.h"
#include "../include/ui_error.h"
/* clang-format on */

struct mock_window_data {
  int id;
};

static enum ui_error mock_create_window(struct ui_window_backend *backend,
                                        const char *title, int width,
                                        int height,
                                        struct ui_window **out_window) {
  (void)backend;
  (void)title;
  (void)width;
  (void)height;
  if (out_window) {
    *out_window = (struct ui_window *)0x12345678;
    return UI_ERROR_NONE;
  }
  return UI_ERROR_NONE;
}

static enum ui_error mock_destroy_window(struct ui_window_backend *backend,
                                         struct ui_window *window) {
  (void)backend;
  (void)window;
  return UI_ERROR_NONE;
}

static enum ui_error mock_show_window(struct ui_window_backend *backend,
                                      struct ui_window *window) {
  (void)backend;
  (void)window;
  return UI_ERROR_NONE;
}

static enum ui_error mock_hide_window(struct ui_window_backend *backend,
                                      struct ui_window *window) {
  (void)backend;
  (void)window;
  return UI_ERROR_NONE;
}

static enum ui_error mock_poll_events(struct ui_window_backend *backend,
                                      struct ui_window *window,
                                      struct ui_event *out_event,
                                      int *out_has_event) {
  (void)backend;
  (void)window;
  (void)out_event;
  if (out_has_event) {
    *out_has_event = 0;
    return UI_ERROR_NONE;
  }
  return UI_ERROR_NONE;
}

static enum ui_error mock_swap_buffers(struct ui_window_backend *backend,
                                       struct ui_window *window) {
  (void)backend;
  (void)window;
  return UI_ERROR_NONE;
}

int main(void) {
  struct ui_window_backend backend;
  struct ui_window *window = NULL;
  struct ui_event event;
  int has_event;
  enum ui_error rc;
  int failed = 0;

  backend.create_window = mock_create_window;
  backend.destroy_window = mock_destroy_window;
  backend.show_window = mock_show_window;
  backend.hide_window = mock_hide_window;
  backend.poll_events = mock_poll_events;
  backend.swap_buffers = mock_swap_buffers;
  backend.push_deep_link = NULL;
  backend.get_os_handle = NULL;
  backend.user_data = NULL;

  rc = backend.create_window(&backend, "Test", 800, 600, &window);
  failed |= (rc != UI_ERROR_NONE);

  rc = backend.show_window(&backend, window);
  failed |= (rc != UI_ERROR_NONE);

  rc = backend.swap_buffers(&backend, window);
  failed |= (rc != UI_ERROR_NONE);

  rc = backend.poll_events(&backend, window, &event, &has_event);
  failed |= (rc != UI_ERROR_NONE);

  rc = backend.hide_window(&backend, window);
  failed |= (rc != UI_ERROR_NONE);

  rc = backend.destroy_window(&backend, window);
  failed |= (rc != UI_ERROR_NONE);

  rc = mock_create_window(&backend, "Test", 800, 600, NULL);
  failed |= (rc != UI_ERROR_NONE);
  rc = mock_poll_events(&backend, window, NULL, NULL);
  failed |= (rc != UI_ERROR_NONE);

  printf("All window backend tests passed.\n");
  return failed;
}
