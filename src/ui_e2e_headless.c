/**
 * \file ui_e2e_headless.c
 * \brief Headless E2E backend implementation.
 */
/* clang-format off */
#include "../include/ui_e2e_headless.h"
#include "ui_types.h"
#include <stdlib.h>
#include <string.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
EM_JS(void, ui_e2e_screenshot_js, (ui_uint32 node_id), {
  if (window._ui_e2e_screenshot_hook) {
    window._ui_e2e_screenshot_hook(node_id);
  } else {
    console.warn("E2E screenshot hook not attached in JS.");
  }
})
/* clang-format on */
#endif
#include "ui_internal_mem.h"

/** \def UI_HEADLESS_MAX_EVENTS
 *  \brief Maximum number of headless events.
 */
#define UI_HEADLESS_MAX_EVENTS 256

/**
 * \struct ui_e2e_headless_ctx
 * \brief Context for the headless E2E testing backend.
 */
struct ui_e2e_headless_ctx {
  struct ui_window_backend backend;
  struct ui_window *dummy_window;

  int width;
  int height;
  int is_visible;

  struct ui_event event_queue[UI_HEADLESS_MAX_EVENTS];
  int event_head;
  int event_tail;
};

/**
 * \brief Creates a dummy headless window.
 * \param[in,out] backend The window backend context.
 * \param[in] title The window title.
 * \param[in] width The width of the window.
 * \param[in] height The height of the window.
 * \param[out] out_window Pointer to store the created dummy window.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t headless_create_window(struct ui_window_backend *backend,
                                         const char *title, int width,
                                         int height,
                                         struct ui_window **out_window) {
  struct ui_e2e_headless_ctx *ctx;
  (void)title;
  if (!backend || !out_window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ctx = (struct ui_e2e_headless_ctx *)backend->user_data;
  ctx->width = width;
  ctx->height = height;
  ctx->dummy_window = (struct ui_window *)ctx; /* any non-null pointer */
  *out_window = ctx->dummy_window;
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a dummy headless window.
 * \param[in,out] backend The window backend context.
 * \param[in,out] window The dummy window to destroy.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t headless_destroy_window(struct ui_window_backend *backend,
                                          struct ui_window *window) {
  (void)backend;
  (void)window;
  return UI_ERROR_NONE;
}

/**
 * \brief Shows a dummy headless window.
 * \param[in,out] backend The window backend context.
 * \param[in,out] window The dummy window to show.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t headless_show_window(struct ui_window_backend *backend,
                                       struct ui_window *window) {
  struct ui_e2e_headless_ctx *ctx;
  (void)window;
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ctx = (struct ui_e2e_headless_ctx *)backend->user_data;
  ctx->is_visible = 1;
  return UI_ERROR_NONE;
}

/**
 * \brief Hides a dummy headless window.
 * \param[in,out] backend The window backend context.
 * \param[in,out] window The dummy window to hide.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t headless_hide_window(struct ui_window_backend *backend,
                                       struct ui_window *window) {
  struct ui_e2e_headless_ctx *ctx;
  (void)window;
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ctx = (struct ui_e2e_headless_ctx *)backend->user_data;
  ctx->is_visible = 0;
  return UI_ERROR_NONE;
}

/**
 * \brief Polls events from the headless queue.
 * \param[in,out] backend The window backend context.
 * \param[in,out] window The dummy window context.
 * \param[out] out_event Pointer to store the polled event.
 * \param[out] out_has_event Set to 1 if an event was polled, 0 otherwise.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t headless_poll_events(struct ui_window_backend *backend,
                                       struct ui_window *window,
                                       struct ui_event *out_event,
                                       int *out_has_event) {
  struct ui_e2e_headless_ctx *ctx;
  (void)window;
  if (!backend || !out_event || !out_has_event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ctx = (struct ui_e2e_headless_ctx *)backend->user_data;

  if (ctx->event_head == ctx->event_tail) {
    *out_has_event = 0;
    return UI_ERROR_NONE;
  }

  *out_event = ctx->event_queue[ctx->event_head];
  ctx->event_head = (ctx->event_head + 1) % UI_HEADLESS_MAX_EVENTS;
  *out_has_event = 1;

  return UI_ERROR_NONE;
}

/**
 * \brief Swaps buffers for a dummy headless window (no-op).
 * \param[in,out] backend The window backend context.
 * \param[in,out] window The dummy window context.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t headless_swap_buffers(struct ui_window_backend *backend,
                                        struct ui_window *window) {
  (void)backend;
  (void)window;
  /* Headless has no buffer to swap */
  return UI_ERROR_NONE;
}

/**
 * \brief Pushes a deep link event to the headless queue.
 * \param[in,out] backend The window backend context.
 * \param[in] uri The URI to push.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t headless_push_deep_link(struct ui_window_backend *backend,
                                          const char *uri) {
  struct ui_e2e_headless_ctx *ctx;
  struct ui_event ev;
  ui_error_t rc;

  if (!backend || !uri) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ctx = (struct ui_e2e_headless_ctx *)backend->user_data;

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_OS_DEEP_LINK;
#if defined(_MSC_VER)
  strncpy_s(ev.event_data.deep_link.uri, sizeof(ev.event_data.deep_link.uri),
            uri, sizeof(ev.event_data.deep_link.uri) - 1);
#else
  UI_STRNCPY(ev.event_data.deep_link.uri, sizeof(ev.event_data.deep_link.uri),
             uri, sizeof(ev.event_data.deep_link.uri) - 1);
#endif
  ev.event_data.deep_link.uri[sizeof(ev.event_data.deep_link.uri) - 1] = '\0';

  rc = ui_e2e_headless_push_event(ctx, &ev);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Creates a new headless E2E context.
 * \param[in] width The initial width.
 * \param[in] height The initial height.
 * \param[out] out_ctx Pointer to store the created context.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_e2e_headless_create(int width, int height,
                                  struct ui_e2e_headless_ctx **out_ctx) {
  struct ui_e2e_headless_ctx *ctx;
  if (!out_ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ctx = (struct ui_e2e_headless_ctx *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_e2e_headless_ctx));
  if (!ctx) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(ctx, 0, sizeof(struct ui_e2e_headless_ctx));
  ctx->width = width;
  ctx->height = height;

  ctx->backend.create_window = headless_create_window;
  ctx->backend.destroy_window = headless_destroy_window;
  ctx->backend.show_window = headless_show_window;
  ctx->backend.hide_window = headless_hide_window;
  ctx->backend.poll_events = headless_poll_events;
  ctx->backend.swap_buffers = headless_swap_buffers;
  ctx->backend.push_deep_link = headless_push_deep_link;
  ctx->backend.get_os_handle = NULL;
  ctx->backend.user_data = ctx;

  *out_ctx = ctx;
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a headless E2E context.
 * \param[in,out] ctx The context to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_e2e_headless_destroy(struct ui_e2e_headless_ctx *ctx) {
  if (!ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(ctx);
  return UI_ERROR_NONE;
}

/**
 * \brief Retrieves the window backend from a headless context.
 * \param[in,out] ctx The headless context.
 * \param[out] out_backend Pointer to store the window backend.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_e2e_headless_get_backend(struct ui_e2e_headless_ctx *ctx,
                                       struct ui_window_backend **out_backend) {
  if (!out_backend)
    return UI_ERROR_INVALID_ARGUMENT;
  if (!ctx) {
    *out_backend = NULL;
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_backend = &ctx->backend;
  return UI_ERROR_NONE;
}

/**
 * \brief Pushes an event into the headless event queue.
 * \param[in,out] ctx The headless context.
 * \param[in] event The event to push.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_e2e_headless_push_event(struct ui_e2e_headless_ctx *ctx,
                                      const struct ui_event *event) {
  int next_tail;
  if (!ctx || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  next_tail = (ctx->event_tail + 1) % UI_HEADLESS_MAX_EVENTS;
  if (next_tail == ctx->event_head) {
    /* Queue full */
    return UI_ERROR_OUT_OF_MEMORY;
  }

  ctx->event_queue[ctx->event_tail] = *event;
  ctx->event_tail = next_tail;

  return UI_ERROR_NONE;
}

/**
 * \brief Simulates a mouse click in the headless context.
 * \param[in,out] ctx The headless context.
 * \param[in] x The X coordinate.
 * \param[in] y The Y coordinate.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_e2e_headless_click(struct ui_e2e_headless_ctx *ctx, int x,
                                 int y) {
  struct ui_event ev;
  ui_error_t rc;

  if (!ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = x;
  ev.event_data.mouse.y = y;
  ev.event_data.mouse.button = 0;
  rc = ui_e2e_headless_push_event(ctx, &ev);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  ev.type = UI_EVENT_MOUSE_UP;
  rc = ui_e2e_headless_push_event(ctx, &ev);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Simulates key typing in the headless context.
 * \param[in,out] ctx The headless context.
 * \param[in] key_code The key code to type.
 * \param[in] modifiers The active keyboard modifiers.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_e2e_headless_type_key(struct ui_e2e_headless_ctx *ctx,
                                    int key_code, unsigned int modifiers) {
  struct ui_event ev;
  ui_error_t rc;

  if (!ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = key_code;
  ev.event_data.keyboard.modifiers = modifiers;
  rc = ui_e2e_headless_push_event(ctx, &ev);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  ev.type = UI_EVENT_KEY_UP;
  rc = ui_e2e_headless_push_event(ctx, &ev);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Advances the mock time for the headless context.
 * \param[in,out] ctx The headless context.
 * \param[in] delta_ms The time delta in milliseconds.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_e2e_advance_time(struct ui_e2e_headless_ctx *ctx,
                               double delta_ms) {
  /* This would integrate with a global mock clock or emit a synthetic tick
   * event */
  if (!ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* In a real scenario, this updates a mock time source used by animations.
     For this test, we can push a synthetic frame/tick event or simply
     simulate the engine tick. */
  (void)delta_ms; /* TODO: update mock global time */

  return UI_ERROR_NONE;
}
