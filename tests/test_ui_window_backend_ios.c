/* clang-format off */
#include "greatest.h"
#include "../include/ui_window_backend_ios.h"
#include "../include/ui_event.h"
#include "../src/ui_internal_mem.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#endif
/* clang-format on */

extern int g_mock_mem_fail;

TEST test_ios_backend_create_destroy(void) {
  struct ui_window_backend *backend = NULL;
  ui_error_t rc;

  rc = ui_window_backend_ios_create(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = ui_window_backend_ios_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_IPHONE
  rc = ui_window_backend_ios_create(&backend);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(backend != NULL);

  rc = ui_window_backend_ios_destroy(backend);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#else
  rc = ui_window_backend_ios_create(&backend);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  ASSERT(backend == NULL);

  rc = ui_window_backend_ios_destroy((struct ui_window_backend *)0x1234);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
#endif

  PASS();
}

TEST test_ios_backend_oom(void) {
#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_IPHONE
  struct ui_window_backend *backend = NULL;
  ui_error_t rc;

  g_mock_mem_fail = 1;
  rc = ui_window_backend_ios_create(&backend);
  g_mock_mem_fail = 0;

  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(backend == NULL);
#endif
  PASS();
}

TEST test_ios_backend_window_creation(void) {
#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_IPHONE
  struct ui_window_backend *backend = NULL;
  struct ui_window *window = NULL;
  ui_error_t rc;
  int has_event = 0;
  struct ui_event ev;
  int w, h;
  float scale;

  rc = ui_window_backend_ios_create(&backend);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test NULLs */
  rc = backend->create_window(NULL, "Test", 800, 600, &window);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = backend->create_window(backend, NULL, 800, 600, &window);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = backend->create_window(backend, "Test", 800, 600, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = backend->create_window(backend, "Test", 800, 600, &window);
  /* In a real device/simulator this might succeed. In CI it might fail if UIKit
   * is not loaded */
  if (rc == UI_ERROR_NONE) {
    ASSERT(window != NULL);

    rc = backend->show_window(NULL, window);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->show_window(backend, NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->show_window(backend, window);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = backend->hide_window(NULL, window);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->hide_window(backend, NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->hide_window(backend, window);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = backend->poll_events(NULL, window, &ev, &has_event);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->poll_events(backend, NULL, &ev, &has_event);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->poll_events(backend, window, NULL, &has_event);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->poll_events(backend, window, &ev, NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->poll_events(backend, window, &ev, &has_event);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = backend->swap_buffers(NULL, window);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->swap_buffers(backend, NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->swap_buffers(backend, window);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = backend->get_scale_factor(NULL, window, &scale);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->get_scale_factor(backend, NULL, &scale);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->get_scale_factor(backend, window, NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->get_scale_factor(backend, window, &scale);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = backend->get_framebuffer_size(NULL, window, &w, &h);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->get_framebuffer_size(backend, NULL, &w, &h);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->get_framebuffer_size(backend, window, NULL, &h);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->get_framebuffer_size(backend, window, &w, NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->get_framebuffer_size(backend, window, &w, &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = backend->set_on_dpi_change_callback(NULL, window, NULL, NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->set_on_dpi_change_callback(backend, NULL, NULL, NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->set_on_dpi_change_callback(backend, window, NULL, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = backend->destroy_window(NULL, window);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->destroy_window(backend, NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = backend->destroy_window(backend, window);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Test OOM during creation */
  g_mock_mem_fail = 1;
  rc = backend->create_window(backend, "Test", 800, 600, &window);
  g_mock_mem_fail = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  rc = ui_window_backend_ios_destroy(backend);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#endif
  PASS();
}

SUITE(ios_backend_suite) {
  RUN_TEST(test_ios_backend_create_destroy);
  RUN_TEST(test_ios_backend_oom);
  RUN_TEST(test_ios_backend_window_creation);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ios_backend_suite);
  GREATEST_MAIN_END();
}
