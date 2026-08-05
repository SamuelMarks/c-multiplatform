/* clang-format off */
#include "../include/ui_window_backend_linux.h"
#include "../include/ui_error.h"
#include "../include/ui_event.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define ASSERT_TRUE(cond)                                                      \
  if (!(cond)) {                                                               \
    printf("Fail at %s:%d\n", __FILE__, __LINE__);                             \
    failed = 1;                                                                \
  }
#define ASSERT_EQ(a, b)                                                        \
  do {                                                                         \
    int _a = (int)(a);                                                         \
    int _b = (int)(b);                                                         \
    if (_a != _b) {                                                            \
      printf("Fail at %s:%d: %d != %d\n", __FILE__, __LINE__, _a, _b);         \
      failed = 1;                                                              \
    }                                                                          \
  } while (0)

static int test_wayland(void) {
  struct ui_window_backend *backend = NULL;
  struct ui_window *win = NULL;
  struct ui_event events[10];
  int has_event = 0;
  int failed = 0;

  ASSERT_EQ(ui_window_backend_linux_create(NULL), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_window_backend_linux_destroy(NULL), UI_ERROR_INVALID_ARGUMENT);

#if defined(__linux__) || defined(__FreeBSD__)
  if (ui_window_backend_linux_create(&backend) == UI_ERROR_NONE) {
    ASSERT_TRUE(backend != NULL);

    ASSERT_EQ(backend->create_window(NULL, "test", 800, 600, &win),
              UI_ERROR_INVALID_ARGUMENT);
    ASSERT_EQ(backend->create_window(backend, NULL, 800, 600, &win),
              UI_ERROR_INVALID_ARGUMENT);
    ASSERT_EQ(backend->create_window(backend, "test", 800, 600, NULL),
              UI_ERROR_INVALID_ARGUMENT);

    ASSERT_EQ(backend->destroy_window(NULL, win), UI_ERROR_INVALID_ARGUMENT);
    ASSERT_EQ(backend->destroy_window(backend, NULL),
              UI_ERROR_INVALID_ARGUMENT);

    ASSERT_EQ(backend->show_window(NULL, win), UI_ERROR_INVALID_ARGUMENT);
    ASSERT_EQ(backend->show_window(backend, NULL), UI_ERROR_INVALID_ARGUMENT);

    ASSERT_EQ(backend->hide_window(NULL, win), UI_ERROR_INVALID_ARGUMENT);
    ASSERT_EQ(backend->hide_window(backend, NULL), UI_ERROR_INVALID_ARGUMENT);

    ASSERT_EQ(backend->poll_events(NULL, win, events, &has_event),
              UI_ERROR_INVALID_ARGUMENT);
    ASSERT_EQ(backend->poll_events(backend, NULL, events, &has_event),
              UI_ERROR_INVALID_ARGUMENT);
    ASSERT_EQ(backend->poll_events(backend, win, NULL, &has_event),
              UI_ERROR_INVALID_ARGUMENT);
    ASSERT_EQ(backend->poll_events(backend, win, events, NULL),
              UI_ERROR_INVALID_ARGUMENT);

    ASSERT_EQ(backend->swap_buffers(NULL, win), UI_ERROR_INVALID_ARGUMENT);
    ASSERT_EQ(backend->swap_buffers(backend, NULL), UI_ERROR_INVALID_ARGUMENT);

    /* Test wayland specific mocking (which doesn't exist yet but we test
     * creation) */
    /* Due to wayland connection failures in test environment,
     * wl_display_connect will likely return NULL */
    /* and create_window will fail. We should test that behavior. */
    ASSERT_TRUE(backend->create_window(backend, "test", 800, 600, &win) !=
                UI_ERROR_NONE);

    ASSERT_EQ(ui_window_backend_linux_destroy(backend), UI_ERROR_NONE);
  }

  g_malloc_fail_countdown = 0;
  ASSERT_EQ(ui_window_backend_linux_create(&backend), UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
#else
  ASSERT_EQ(ui_window_backend_linux_create(&backend), UI_ERROR_UNKNOWN);
  /* The stub platform sets backend to NULL. Let's create a fake one to test the
   * backend check in destroy */
  backend = (struct ui_window_backend *)(void *)1;
  ASSERT_EQ(ui_window_backend_linux_destroy(backend), UI_ERROR_UNKNOWN);
#endif

  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_wayland();

  if (!failed) {
    printf("test_ui_window_backend_linux_wayland passed\n");
  } else {
    printf("test_ui_window_backend_linux_wayland failed\n");
  }
  return failed;
}
