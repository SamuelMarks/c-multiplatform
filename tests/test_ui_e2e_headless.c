/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "../include/ui_e2e_headless.h"
#include "../include/ui_engine.h"
#include "../include/ui_error.h"
#include "../include/ui_event.h"
#include "../include/ui_window_backend.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

int main(void) {
  struct ui_e2e_headless_ctx *ctx = NULL;
  struct ui_window_backend *backend = NULL;
  struct ui_window *window = NULL;
  struct ui_event ev;
  int has_event = 0;
  ui_error_t rc;

  /* Null checks */
  rc = ui_e2e_headless_create(800, 600, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  g_malloc_fail_countdown = 0;
  rc = ui_e2e_headless_create(800, 600, &ctx);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  rc = ui_e2e_headless_create(800, 600, &ctx);
  if (rc != UI_ERROR_NONE) {
    printf("ui_e2e_headless_create failed\n");
    return 1;
  }

  rc = ui_e2e_headless_get_backend(ctx, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_e2e_headless_get_backend(NULL, &backend);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_e2e_headless_get_backend(ctx, &backend) != UI_ERROR_NONE)
    backend = NULL;
  if (!backend) {
    printf("ui_e2e_headless_get_backend failed\n");
    return 1;
  }

  rc = backend->create_window(NULL, "Headless", 800, 600, &window);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = backend->create_window(backend, "Headless", 800, 600, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = backend->create_window(backend, "Headless", 800, 600, &window);
  if (rc != UI_ERROR_NONE) {
    printf("backend->create_window failed\n");
    return 1;
  }

  rc = backend->show_window(NULL, window);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = backend->show_window(backend, window);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = backend->hide_window(NULL, window);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = backend->hide_window(backend, window);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = backend->poll_events(NULL, window, &ev, &has_event);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = backend->poll_events(backend, window, NULL, &has_event);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = backend->poll_events(backend, window, &ev, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = backend->push_deep_link(NULL, "https://test");
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = backend->push_deep_link(backend, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = backend->push_deep_link(backend, "https://test");
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Read DEEP_LINK */
  rc = backend->poll_events(backend, window, &ev, &has_event);
  if (rc != UI_ERROR_NONE || !has_event || ev.type != UI_EVENT_OS_DEEP_LINK) {
    printf("poll_events failed to get DEEP_LINK\n");
    return 1;
  }

  rc = backend->swap_buffers(NULL, window);
  rc = backend->swap_buffers(backend, window);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Queue is empty now */
  rc = backend->poll_events(backend, window, &ev, &has_event);
  if (rc != UI_ERROR_NONE || has_event != 0)
    return 1;

  rc = ui_e2e_headless_click(NULL, 100, 200);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test clicking */
  rc = ui_e2e_headless_click(ctx, 100, 200);
  if (rc != UI_ERROR_NONE) {
    printf("ui_e2e_headless_click failed\n");
    return 1;
  }

  /* Read MOUSE_DOWN */
  rc = backend->poll_events(backend, window, &ev, &has_event);
  if (rc != UI_ERROR_NONE || !has_event || ev.type != UI_EVENT_MOUSE_DOWN) {
    printf("poll_events failed to get MOUSE_DOWN\n");
    return 1;
  }
  if (ev.event_data.mouse.x != 100 || ev.event_data.mouse.y != 200) {
    printf("MOUSE_DOWN coordinates incorrect\n");
    return 1;
  }

  /* Read MOUSE_UP */
  rc = backend->poll_events(backend, window, &ev, &has_event);
  if (rc != UI_ERROR_NONE || !has_event || ev.type != UI_EVENT_MOUSE_UP) {
    printf("poll_events failed to get MOUSE_UP\n");
    return 1;
  }

  rc = ui_e2e_headless_type_key(NULL, UI_KEY_ENTER, UI_MODIFIER_SHIFT);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test key typing */
  rc = ui_e2e_headless_type_key(ctx, UI_KEY_ENTER, UI_MODIFIER_SHIFT);
  if (rc != UI_ERROR_NONE) {
    printf("ui_e2e_headless_type_key failed\n");
    return 1;
  }

  /* Read KEY_DOWN */
  rc = backend->poll_events(backend, window, &ev, &has_event);
  if (rc != UI_ERROR_NONE || !has_event || ev.type != UI_EVENT_KEY_DOWN) {
    printf("poll_events failed to get KEY_DOWN\n");
    return 1;
  }
  if (ev.event_data.keyboard.key_code != UI_KEY_ENTER ||
      ev.event_data.keyboard.modifiers != UI_MODIFIER_SHIFT) {
    printf("KEY_DOWN data incorrect\n");
    return 1;
  }

  /* Read KEY_UP */
  rc = backend->poll_events(backend, window, &ev, &has_event);
  if (rc != UI_ERROR_NONE || !has_event || ev.type != UI_EVENT_KEY_UP) {
    printf("poll_events failed to get KEY_UP\n");
    return 1;
  }

  /* Test push_event directly */
  {
    struct ui_event custom_ev;
    rc = ui_e2e_headless_push_event(NULL, &custom_ev);
    if (rc != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    rc = ui_e2e_headless_push_event(ctx, NULL);
    if (rc != UI_ERROR_INVALID_ARGUMENT)
      return 1;

    memset(&custom_ev, 0, sizeof(custom_ev));
    custom_ev.type = UI_EVENT_WINDOW_RESIZE;
    custom_ev.event_data.window.width = 1024;
    custom_ev.event_data.window.height = 768;
    rc = ui_e2e_headless_push_event(ctx, &custom_ev);
    if (rc != UI_ERROR_NONE) {
      printf("ui_e2e_headless_push_event failed\n");
      return 1;
    }

    rc = backend->poll_events(backend, window, &ev, &has_event);
    if (rc != UI_ERROR_NONE || !has_event ||
        ev.type != UI_EVENT_WINDOW_RESIZE) {
      printf("poll_events failed to get RESIZE event\n");
      return 1;
    }
  }

  /* Fill queue to test OOM */
  {
    int i;
    struct ui_event ev_fill;
    memset(&ev_fill, 0, sizeof(ev_fill));
    for (i = 0; i < 255; i++) {
      ui_e2e_headless_push_event(ctx, &ev_fill);
    }
    rc = ui_e2e_headless_push_event(ctx, &ev_fill);
    if (rc != UI_ERROR_OUT_OF_MEMORY)
      return 1;
    rc = ui_e2e_headless_click(ctx, 10, 10);
    if (rc != UI_ERROR_OUT_OF_MEMORY)
      return 1;
    rc = ui_e2e_headless_type_key(ctx, UI_KEY_SPACE, 0);
    if (rc != UI_ERROR_OUT_OF_MEMORY)
      return 1;
  }

  {
    int i;
    struct ui_event ev_fill;
    memset(&ev_fill, 0, sizeof(ev_fill));

    while (backend->poll_events(backend, window, &ev_fill, &has_event) ==
               UI_ERROR_NONE &&
           has_event) {
    }

    for (i = 0; i < 254; i++) {
      ui_e2e_headless_push_event(ctx, &ev_fill);
    }
    rc = ui_e2e_headless_click(ctx, 10, 10);
    if (rc != UI_ERROR_OUT_OF_MEMORY)
      return 1;

    while (backend->poll_events(backend, window, &ev_fill, &has_event) ==
               UI_ERROR_NONE &&
           has_event) {
    }

    for (i = 0; i < 254; i++) {
      ui_e2e_headless_push_event(ctx, &ev_fill);
    }
    rc = ui_e2e_headless_type_key(ctx, UI_KEY_SPACE, 0);
    if (rc != UI_ERROR_OUT_OF_MEMORY)
      return 1;

    while (backend->poll_events(backend, window, &ev_fill, &has_event) ==
               UI_ERROR_NONE &&
           has_event) {
    }

    for (i = 0; i < 255; i++) {
      ui_e2e_headless_push_event(ctx, &ev_fill);
    }
    rc = backend->push_deep_link(backend, "test");
    if (rc != UI_ERROR_OUT_OF_MEMORY)
      return 1;

    while (backend->poll_events(backend, window, &ev_fill, &has_event) ==
               UI_ERROR_NONE &&
           has_event) {
    }
  }

  rc = backend->destroy_window(backend, window);
  if (rc != UI_ERROR_NONE) {
    printf("backend->destroy_window failed\n");
    return 1;
  }

  rc = ui_e2e_headless_destroy(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_e2e_headless_destroy(ctx);
  if (rc != UI_ERROR_NONE) {
    printf("ui_e2e_headless_destroy failed\n");
    return 1;
  }

  rc = ui_e2e_advance_time(NULL, 16.666);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Create again for Box Layout Test */
  rc = ui_e2e_headless_create(800, 600, &ctx);
  if (rc == UI_ERROR_NONE) {
    ui_e2e_advance_time(ctx, 16.666);
    ui_e2e_headless_destroy(ctx);
  }

  printf("All test_ui_e2e_headless tests passed.\n");
  return 0;
}
